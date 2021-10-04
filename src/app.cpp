//
// Created by Seungwoo on 2021-10-03.
//

#include "app.hpp"

#include <nlohmann/json.hpp>
#include <perfkit/common/format.hxx>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/iota.hpp>

#if __linux__ || __unix__
#include <arpa/inet.h>
#include <spdlog/spdlog.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/unistd.h>

using std::lock_guard;
using pollfd_ty = pollfd;
#elif _WIN32
#define _CRT_NONSTDC_NO_WARNINGS
#include <Winsock2.h>
#include <process.h>

#endif

using namespace std::literals;
using namespace perfkit::literals;

apiserver::app::app(const apiserver::app::init_arg& init) {
  _sock_srv = ::socket(AF_INET, SOCK_STREAM, 0);

  SPDLOG_INFO("(SOCK {}) binding server on {}:{}", _sock_srv, init.bind_ip, init.bind_port);
  sockaddr_in addr     = {};
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons(init.bind_port);
  addr.sin_addr.s_addr = inet_addr(init.bind_ip.c_str());
  if (0 != ::bind(_sock_srv, (sockaddr*)&addr, sizeof addr)) {
    ::close(_sock_srv);
    throw std::runtime_error{"binding failed. ({}) {}"_fmt % errno % strerror(errno)};
  }

  if (0 != ::listen(_sock_srv, 5)) {
    ::close(_sock_srv);
    throw std::runtime_error("listening failed ({}) {}"_fmt % errno % strerror(errno));
  }

  _worker = std::thread{[&] { _worker_fn(); }};
}

apiserver::app::~app() {
  _active.clear();
  if (_worker.joinable()) {
    SPDLOG_INFO("joining async worker thread ...");
    _worker.join();
  }

  if (_sock_srv) {
    SPDLOG_INFO("closing application listen socket {} ...", _sock_srv);
    ::close(_sock_srv);
  }
}

void apiserver::app::_worker_fn() {
  enum { RECV_BUFLEN = 4096 };
  auto _buf = std::make_unique<char[]>(RECV_BUFLEN);

  std::vector<::pollfd> _pollees;
  {
    auto acceptor    = &_pollees.emplace_back();
    acceptor->events = POLLIN;
    acceptor->fd     = _sock_srv;
  }

  auto _close_session = [this, &_pollees](size_t sessidx) {
    std::lock_guard _{_session_lock};

    auto fd = _pollees[sessidx + 1].fd;
    _pollees.erase(_pollees.begin() + sessidx + 1);
    _sessions.erase(_sessions.begin() + sessidx);
    ::close(fd);
  };

  while (_active.test_and_set(std::memory_order_relaxed)) {
    auto n_poll = ::poll(_pollees.data(), _pollees.size(), 1000);

    if (n_poll < 0) {
      SPDLOG_CRITICAL("POLL() failed. ({}) {}", errno, strerror(errno));
      throw std::runtime_error("poll() failed.");
    }

    if (auto acpt = &_pollees[0]; acpt->revents & POLLIN) {
      // handle accept.
      // 1. create new session
      // 2. create new stream
      // 3. map pollfd
      sockaddr_in ipaddr   = {};
      socklen_t ipaddr_len = sizeof ipaddr;

      auto sock = ::accept(acpt->fd, (sockaddr*)&ipaddr, &ipaddr_len);
      if (sock == -1) {
        SPDLOG_ERROR(
                "failed to accept new connection: [{}] ({}) {}",
                acpt->fd, errno, strerror(errno));
      } else {
        SPDLOG_INFO(
                "new connection established: {}:{}",
                inet_ntoa(ipaddr.sin_addr), htons(ipaddr.sin_port));

        auto pollee    = &_pollees.emplace_back();
        pollee->fd     = sock;
        pollee->events = POLLIN;

        session::init_arg init;
        init.id = ++_id_gen;
        init.ip = inet_ntoa(ipaddr.sin_addr);
        init.write =
                [=](std::string_view s) {
                  ::send(sock, s.data(), s.size(), 0);
                };

        std::lock_guard _{_session_lock};
        _sessions.emplace_back(std::make_unique<session>(std::move(init)));
      }
    }

    for (size_t idx = 0; idx < _sessions.size() && n_poll > 0; ++idx) {
      auto pfd  = &_pollees[idx + 1];
      auto sess = _sessions.at(idx).get();
      if (pfd->revents != 0) { n_poll--; }
      if (not(pfd->revents & POLLIN)) {
        if (errno == 0) { continue; }
        SPDLOG_WARN("> session '{}' poll() error: ({}) {} ", sess->name(), errno, strerror(errno));
        SPDLOG_WARN("| Disconnecting ...");
        _close_session(idx--);
        continue;
      }
      if (not sess->is_valid_state()) {
        SPDLOG_WARN("> session '{}' is not in valid state. disconnecting ...", sess->name());
        _close_session(idx--);
        continue;
      }

      SPDLOG_INFO("RPOLL: {:x}", pfd->revents);
      auto n_read = ::recv(pfd->fd, _buf.get(), RECV_BUFLEN, 0);
      if (n_read <= 0) {
        SPDLOG_WARN("EOF received: [fd {}] ({}) {}", pfd->fd, errno, strerror(errno));
        _close_session(idx--);
        continue;
      }

      sess->handle_recv({_buf.get(), (size_t)n_read});
      pfd->revents = {};
    }
  }

  SPDLOG_INFO("worker thread shutting down ...");
}

std::string apiserver::app::list_sessions() const {
  nlohmann::json js;
  auto list = &js["sessions"];

  {
    lock_guard _{_session_lock};
    for (const auto& ptr : _sessions) {
      auto elem = &(*list)[std::to_string(ptr->id())];
      elem->emplace("ip", ptr->ip());
      elem->emplace("pid", ptr->pid());
      elem->emplace("machine-name", ptr->host());
      elem->emplace("name", ptr->name());
      elem->emplace("epoch", ptr->epoch());
      elem->emplace("description", ptr->description());
    }
  }

  return js.dump();
}
