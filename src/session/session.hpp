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
  void request_update();

  std::string fetch_shell(size_t req_seqn);
  std::future<std::string> post_shell(std::string const& content, bool is_suggest);

  void handle_recv(perfkit::array_view<char> buf);

 private:
  void _handle_header();
  void _handle_register();
  void _handle_session_fetch();
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

  // -- command handling
  size_t _size_pending_recv = {};
  std::string _receiving_payload;

  std::function<void()> _state_proc;

  perfkit::_net::message_builder _build_msg;
  std::mutex _write_lock;

  // -- shell management
  std::mutex _shell_output_lock;
  size_t _shell_output_seqn = 0;
  perfkit::circular_queue<char> _shell_output{4 * 1024 * 1024 - 1};

  // --
  std::mutex _shell_suggest_lock;
  std::map<int64_t, std::pair<clock_type::time_point, std::promise<std::string>>>
          _shell_suggest_replies;

  // -- config manips
  struct config_state {
    //
    std::mutex lock;

    //
    int64_t fence;

    // All instnatiated registries.
    std::multimap<int64_t /*fence*/, perfkit::_net::config_registry_descriptor> registries;

    // all latest configurations.
    // this will be sent to the client that who lost too many updates
    std::unordered_map<int64_t /*hash*/, nlohmann::json> all_values;

    // for optimization purpose, recent updates will be cached.
    std::multimap<int64_t /*fence*/, int64_t /*hash*/> updates;
  } _state_cfg;

  //
};

}  // namespace apiserver
