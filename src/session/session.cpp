//
// Created by Seungwoo on 2021-10-03.
//

#include "session.hpp"

#include <unordered_set>

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
      case _net::provider_message::session_flush_reply: return [&] { _handle_session_fetch(); };
      case _net::provider_message::heartbeat: return [] { throw std::bad_function_call{}; };
      case _net::provider_message::shell_suggest: return [&] { _handle_shell_suggestion(); };

      default:
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

std::string apiserver::session::fetch_shell(size_t req_seqn) {
  // 1. register shell fetch handler callbacks
  SPDLOG_DEBUG("{}@{}: handling fetched shell message", name(), host());
  auto locker{std::unique_lock{_shell_output_lock}};

  size_t seqn                           = _shell_output_seqn;
  perfkit::circular_queue<char>& buffer = _shell_output;

  auto seqn_actual = std::min(seqn, req_seqn);
  auto n_send      = seqn - seqn_actual;
  n_send           = std::min(n_send, buffer.size());

  int64_t offset   = seqn - n_send;
  int64_t sequence = seqn;
  std::string content{buffer.end() - n_send, buffer.end()};

  locker.unlock();

  nlohmann::json builder;
  builder.emplace("offset", offset);
  builder.emplace("sequence", sequence);
  builder.emplace("content", std::move(content));

  return builder.dump();
}

void apiserver::session::_handle_session_fetch() {
  // 1. retrieve message
  // 2. update sequence & shell output buffer
  // 3. iterate and call all fetch callbacks
  auto message = _retrieve<_net::session_flush_chunk>();
  if (not message)
    return;

  {
    auto _{std::lock_guard{_shell_output_lock}};
    _shell_output_seqn  = message->shell_fence;
    auto& shell_content = message->shell_content;
    std::copy(shell_content.begin(), shell_content.end(), std::back_inserter(_shell_output));
  }
  {
    auto* s = &_state_cfg;
    auto _{std::unique_lock{s->lock}};

    s->fence = message->config_fence;

    for (auto& registry : message->config_registry_new) {
      SPDLOG_INFO("{}@{}: new config registry: {}", name(), host(), registry.name);
      s->registries.emplace(s->fence, std::move(registry));
    }

    for (auto& entity : message->config_updates) {
      s->all_values[entity.hash] = entity.data;
      s->updates.emplace(s->fence, entity.hash);
    }

    if (s->updates.size() > 30) {  // purge old updates
      auto it_old_update = s->updates.lower_bound(s->fence - 10);
      s->updates.erase(s->updates.begin(), it_old_update);
    }
  }
}

std::future<std::string> apiserver::session::post_shell(const std::string& content, bool is_suggest) {
  perfkit::_net::shell_input_line payload;
  payload.content    = content;
  payload.is_suggest = is_suggest;
  payload.request_id = _req_id();

  std::future<std::string> future;

  if (is_suggest) {
    std::lock_guard _{_shell_suggest_lock};
    auto [it, is_new] = _shell_suggest_replies.try_emplace(payload.request_id);

    if (not is_new)
      throw std::logic_error{"generator never return same number"};

    it->second.first = clock_type::now();
    future           = it->second.second.get_future();
  }

  SPDLOG_DEBUG("{}@{}: sent shell input request (suggest: {})", name(), host(), is_suggest);
  lock_guard{_write_lock},
          _data.write(_build_msg(payload));

  return future;
}

void apiserver::session::_handle_shell_suggestion() {
  auto message = _retrieve<_net::shell_suggest_reply>();
  if (not message)
    return;

  {
    lock_guard _{_shell_suggest_lock};
    auto it = _shell_suggest_replies.find(message->request_id);
    if (it == _shell_suggest_replies.end()) {
      SPDLOG_ERROR("LOGIC ERROR: unsent request {} received.", message->request_id);
      return;
    }

    nlohmann::json builder;
    builder["suggestion"] = std::move(message->content);
    builder["candidates"] = message->suggest_words;

    it->second.second.set_value(builder.dump());
    _shell_suggest_replies.erase(it);
  }
}

void apiserver::session::request_update() {
  SPDLOG_DEBUG("{}@{}: sent session fetch request", name(), host());
  lock_guard _{_write_lock};
  _data.write(_build_msg(_net::server_message::session_flush_request, {}));
}

std::string apiserver::session::fetch_config_update(int64_t fence) {
  auto* s = &_state_cfg;
  auto _{std::shared_lock{s->lock}};

  nlohmann::json builder;
  builder["fence"] = s->fence;
  if (s->all_values.empty()) { return builder.dump(); }

  auto* js_registries = &builder["registry_new"];

  // find all registries which are registered after 'fence'
  for (auto it = s->registries.lower_bound(fence);
       it != s->registries.end();
       ++it) {
    auto const& [_unused0, registry] = *it;
    auto* js_registry                = &js_registries->emplace_back();
    (*js_registry)["name"]           = it->second.name;

    auto* js_entities = &(*js_registry)["entities"];
    if (registry.entities.empty())
      continue;  // if there's no entities...

    (*js_entities)[registry.entities.size() - 1];  // reserve array size

    size_t idx = 0;
    for (auto& entity : registry.entities) {
      auto* js_entity                   = &(*js_entities)[idx++];
      (*js_entity)["hash"]              = entity.hash;
      (*js_entity)["order_key"]         = entity.order_key;
      (*js_entity)["hierarchical_name"] = entity.display_key;
      (*js_entity)["metadata"]          = entity.metadata;
    }
  }

  // find all configuration updates
  // if the first update is later than fence, all values should be transferred.
  bool is_too_old  = not s->updates.empty() && fence < s->updates.begin()->first;
  auto* js_updates = &builder["updates"];

  if (is_too_old) {
    (*js_updates)[s->all_values.size() - 1];
    size_t idx = 0;
    for (const auto& pair : s->all_values)
      (*js_updates)[idx++] = pair;
  } else {
    // there can be duplicated updates. to prevent redundant expensive copy,
    //  merge all deltas firstly.
    std::unordered_set<int64_t> updated;

    updated.reserve(s->updates.size());
    std::transform(
            s->updates.lower_bound(fence), s->updates.end(),
            std::inserter(updated, updated.begin()),
            [&](auto&& k) { return k.second; });

    if (not updated.empty())
      (*js_updates)[updated.size() - 1];

    std::transform(
            updated.begin(), updated.end(),
            js_updates->begin(),
            [&](int64_t hash) {
              return std::make_pair(hash, s->all_values[hash]);
            });
  }

  return builder.dump();
}
