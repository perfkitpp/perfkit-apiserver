//
// Created by Seungwoo on 2021-10-03.
//

#include "session.hpp"

#include <spdlog/spdlog.h>

void apiserver::session::handle_recv(perfkit::array_view<char> buf) {
  SPDLOG_INFO("Recevied {} bytes: {}", buf.size(), std::string_view{buf.data(), buf.size()});
}
