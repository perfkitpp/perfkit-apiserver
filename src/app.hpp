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
  std::string fetch_shell_output(std::string_view session, int64_t sequence);

 private:
  void _worker_fn();

 private:
  mutable std::mutex _session_lock;
  std::vector<std::unique_ptr<session>> _sessions;

  int64_t _id_gen = 0;

  socket_ty _sock_srv;

  std::atomic_flag _active{true};
  std::thread _worker;
};
}  // namespace apiserver