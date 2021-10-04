//
// Created by Seungwoo on 2021-10-03.
//

#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>

#include <perfkit/common/array_view.hxx>

namespace apiserver {

class session {
 public:
  struct init_arg {
    int64_t id;
    std::function<void(std::string_view)> write;
    std::string ip;
  };

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

  void handle_recv(perfkit::array_view<char> buf);

 private:
  void _handle_header();
  void _handle_register();

  template <typename Ty_>
  std::optional<Ty_> _retrieve();

 private:
  bool _is_valid = true;

  std::string _name = "_none_";
  std::string _host = "_none_";
  int64_t _pid      = 0;
  int64_t _epoch    = 0;
  std::string _desc = "";

  init_arg _data;

  // -- command handling
  size_t _size_pending_recv = {};
  std::string _receiving_payload;

  std::function<void()> _state_proc;

  // -- shell management
};

}  // namespace apiserver
