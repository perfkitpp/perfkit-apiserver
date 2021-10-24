//
// Created by Seungwoo on 2021-10-03.
//

#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <list>
#include <mutex>

#include <perfkit/_net/net-proto.hpp>
#include <perfkit/common/array_view.hxx>
#include <perfkit/common/circular_queue.hxx>

namespace apiserver {

class session {
 public:
  struct init_arg {
    int64_t id;
    std::function<void(std::string_view)> write;
    std::string ip;
  };

  using clock_type = std::chrono::steady_clock;

  explicit session(init_arg init) : _data(std::move(init)) {}

 public:
  bool is_valid_state() const noexcept { return _is_valid; }

  int64_t id() const { return _data.id; }
  std::string_view name() const { return _name; }
  std::string_view host() const { return _host; }
  std::string_view ip() const { return _data.ip; }
  std::string_view description() const { return _desc; }
  int64_t epoch() const { return _epoch; }
  int64_t pid() const { return _pid; }

  void heartbeat();
  std::future<std::string> fetch_shell(size_t req_seqn);
  std::future<std::string> post_shell(std::string const& content, bool is_suggest);

  void handle_recv(perfkit::array_view<char> buf);

 private:
  void _handle_header();
  void _handle_register();
  void _handle_shell_fetch();
  void _handle_shell_suggestion();

  int64_t _req_id() { return ++_request_id_generator; }

  template <typename Ty_>
  std::optional<Ty_> _retrieve();

 private:
 private:
  bool _is_valid = true;

  std::string _name = "_none_";
  std::string _host = "_none_";
  int64_t _pid      = 0;
  int64_t _epoch    = 0;
  std::string _desc = "";

  volatile int64_t _request_id_generator = 0;

  init_arg _data;
  std::mutex _oplock;  // generic lock for overall critical operations

  // -- command handling
  size_t _size_pending_recv = {};
  std::string _receiving_payload;

  std::function<void()> _state_proc;

  perfkit::_net::message_builder _build_msg;
  std::mutex _write_lock;

  // -- shell management
  size_t _shell_output_seqn = 0;
  perfkit::circular_queue<char> _shell_output{4 * 1024 * 1024 - 1};

  using shell_handler_queue = std::list<
          std::function<void(size_t, perfkit::circular_queue<char> const&)>>;
  shell_handler_queue _shell_fetch_callbacks;

  // --
  std::map<int64_t, std::pair<clock_type::time_point, std::promise<std::string>>>
          _shell_suggest_replies;
};

}  // namespace apiserver
