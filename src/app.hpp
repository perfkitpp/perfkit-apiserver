//
// Created by Seungwoo on 2021-10-03.
//

#pragma once
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "session/session.hpp"

namespace apiserver {

#if __linux__ || __unix__
using socket_ty = int;
#elif _WIN32
using socket_ty = void*;
#endif

class app {
 public:
  struct init_arg {
    std::string bind_ip;
    uint16_t bind_port;
  };

  app(init_arg const& init);
  ~app();

 public:
  std::string list_sessions() const;
  std::string fetch_shell_output(int64_t session, int64_t sequence);
  std::string post_shell_input(int64_t session, std::string const& body);
  std::string fetch_config_update(int64_t session, int64_t fence);

 private:
  void _worker_fn();

  template <typename Fn_>
  auto _session_critical_op(int64_t sess_id, Fn_&& fn) {
    using namespace std::literals;

    std::unique_lock lock{_session_lock};
    auto it = std::find_if(
            _sessions.begin(), _sessions.end(), [&](auto&& s) { return s->id() == sess_id; });

    if (it == _sessions.end()) { return false; }

    fn(it->get());
    return true;
  }

 private:
  mutable std::mutex _session_lock;
  std::vector<std::unique_ptr<session>> _sessions;

  int64_t _id_gen = 0;

  socket_ty _sock_srv;

  std::atomic_flag _active{true};
  std::thread _worker;
};
}  // namespace apiserver