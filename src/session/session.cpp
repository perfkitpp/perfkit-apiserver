//
// Created by Seungwoo on 2021-10-03.
//

#include "session.hpp"

#include <perfkit/_net/net-proto.hpp>
#include <spdlog/spdlog.h>

using namespace perfkit;
using std::lock_guard;

template <typename Ty_>
std::optional<Ty_> apiserver::session::_retrieve() {
  try {
    return _net::json::from_bson(_receiving_payload).get<Ty_>();
  } catch (_net::json::parse_error& e) {
    SPDLOG_WARN("{}@{}: parse error: ({}:{}) {}", name(), host(), e.id, e.byte, e.what());
  } catch (std::exception& e) {
    SPDLOG_WARN("{}@{}: conversion error: {}", name(), host(), e.what());
  }

  return {};
}

namespace apiserver {
struct invalid_session_state : std::exception {};
}  // namespace apiserver

void apiserver::session::handle_recv(perfkit::array_view<char> buf) {
  while (not buf.empty()) {
    if (_size_pending_recv == 0) {
      _size_pending_recv = sizeof(_net::message_header);
      _state_proc        = [&] { _handle_header(); };
    }

    auto n_recv = std::min(_size_pending_recv, buf.size());

    auto rdbuf = buf.subspan(0, n_recv);
    buf        = buf.subspan(n_recv);

    _size_pending_recv -= n_recv;
    _receiving_payload.append(rdbuf.begin(), rdbuf.end());

    if (_size_pending_recv == 0) {
      try {
        _state_proc();
        _receiving_payload.clear();
      } catch (invalid_session_state&) {
        SPDLOG_WARN("{}@{}: session transited into invalid state.", name(), host());
        _is_valid = false;
        return;
      }
    }
  }
}

void apiserver::session::_handle_header() {
  assert(_receiving_payload.size() == sizeof(_net::message_header));
  assert(_size_pending_recv == 0);

  auto head          = *reinterpret_cast<_net::message_header*>(_receiving_payload.data());
  _size_pending_recv = head.payload_size;

  if (not std::equal(_net::HEADER.begin(), _net::HEADER.end(), head.header)) {
    SPDLOG_WARN("{}@{}: delivered invalid header ...", name(), host());
    throw invalid_session_state{};
  }

  _state_proc = [&]() -> decltype(_state_proc) {
    switch (head.type.session) {
      case _net::provider_message::register_session: return [&] { _handle_register(); };
      case _net::provider_message::heartbeat: return [] { throw std::bad_function_call{}; };

      default:
      case _net::provider_message::config_all:
      case _net::provider_message::config_update:
      case _net::provider_message::trace_groups:
      case _net::provider_message::trace_image:
      case _net::provider_message::shell_flush:
      case _net::provider_message::shell_suggest:
      case _net::provider_message::invalid:
        SPDLOG_WARN("{}@{}: handler not implemented", name(), host());
        throw invalid_session_state{};
    }
  }();
}

static std::string epoch_to_string(int64_t ll_ms) {
  auto ll = ll_ms / 1000;  // by seconds

  const time_t old = (time_t)ll;
  struct tm* oldt  = localtime(&old);
  return asctime(oldt);
}

void apiserver::session::_handle_register() {
  SPDLOG_INFO("handling register request ...", name(), host());
  auto info = _retrieve<_net::session_register_info>();
  if (not info) { return; }

  SPDLOG_INFO("> new session");
  SPDLOG_INFO("|{:>30}: {}", "session name", info->session_name);
  SPDLOG_INFO("|{:>30}: {}", "machine name", info->machine_name);
  SPDLOG_INFO("|{:>30}: {}", "PID", info->pid);
  SPDLOG_INFO("|{:>30}: {}", "epoch", epoch_to_string(info->epoch));
  SPDLOG_INFO("|{:>30}: {}", "description", info->description);

  _name  = std::move(info->session_name);
  _host  = std::move(info->machine_name);
  _pid   = info->pid;
  _epoch = info->epoch;
  _desc  = std::move(info->description);
}

void apiserver::session::heartbeat() {
  SPDLOG_TRACE("{}@{}: sent heartbeat", name(), host());
  lock_guard _{_write_lock};
  _data.write(_build_msg(_net::server_message::heartbeat, {}));
}
