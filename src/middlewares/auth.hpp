//
// Created by Seungwoo on 2021-10-02.
//

#pragma once
#include <crow/http_request.h>
#include <crow/http_response.h>

namespace middleware {
class auth {
  using request  = crow::request;
  using response = crow::response;

 public:
  auth();

 public:
  struct context {
  };

  void before_handle(request& req, response& res, context&);

  void after_handle(request& /*req*/, response& res, context& /*ctx*/) {}
};
}  // namespace middleware
