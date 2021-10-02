//
// Created by Seungwoo on 2021-10-02.
//

#pragma once
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <spdlog/spdlog.h>

namespace middleware {
class auth {
  using request  = crow::request;
  using response = crow::response;

 public:
  struct context {
  };

  void before_handle(request& req, response& res, context&) {
    spdlog::info(
            "middleware called: \n"
            "\turl: {}\n"
            "\turl_raw: {}\n"
            "\tbody: {}\n",
            req.url, req.raw_url, req.body);

    // TODO: Implement authentication
    //  If URL is not "/login", try authenticate with "auth" header,
    //   then reject if target is not properly authenticated.
    //  There will be some other operations which require administrative privileges, and those
    //   operations should be handled inside this middleware either.
  }

  void after_handle(request& /*req*/, response& res, context& /*ctx*/) {}
};
}  // namespace middleware
