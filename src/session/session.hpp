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
    std::function<void(std::string_view)> write;
  };

  explicit session(init_arg init) : _data(std::move(init)) {}

 public:
  std::string_view name() const { return {}; }
  std::string_view host() const { return {}; }

  void handle_recv(perfkit::array_view<char> buf);

 private:
  init_arg _data;
};

}  // namespace apiserver
