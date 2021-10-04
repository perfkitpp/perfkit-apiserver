//
// Created by Seungwoo on 2021-10-02.
//

#include "auth.hpp"

#include <spdlog/spdlog.h>

void middleware::auth::before_handle(middleware::auth::request &req, middleware::auth::response &res, middleware::auth::context &) {
  spdlog::info(
          "middleware called: \r\n"
          "\turl: {}\r\n"
          "\turl_raw: {}\r\n"
          "\tbody: {}\r\n",
          req.url, req.raw_url, req.body);

  // TODO: Implement authentication
  //  If URL is not "/login", try authenticate with "auth" header,
  //   then reject if target is not properly authenticated.
  //  There will be some other operations which require administrative privileges, and those
  //   operations should be handled inside this middleware either.
}

middleware::auth::auth() {
  spdlog::info("middleware created");
}
