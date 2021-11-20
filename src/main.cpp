#include <atomic>

#include <crow/app.h>
#include <perfkit/configs.h>
#include <perfkit/detail/commands.hpp>
#include <perfkit/extension/cli.hpp>
#include <perfkit/extension/net-provider.hpp>
#include <spdlog/spdlog.h>

#include "middlewares/auth.hpp"

using namespace std::literals;

PERFKIT_CATEGORY(apiserver) {
  PERFKIT_CONFIGURE(bind_ip, "0.0.0.0")
          .make_flag(true, "apiserver-bind-ip")
          .confirm();

  PERFKIT_CONFIGURE(bind_port, 15572)
          .make_flag(true, "apiserver-bind-port")
          .confirm();
}

PERFKIT_CATEGORY(provider) {
  PERFKIT_CONFIGURE(bind_ip, "0.0.0.0")
          .make_flag(true, "provider-bind-ip")
          .confirm();

  PERFKIT_CONFIGURE(bind_port, 25572)
          .make_flag(true, "provider-bind-port")
          .confirm();
}

int main(int argc, char** argv) {
  perfkit::configs::parse_args(&argc, &argv, true);
  crow::App<middleware::auth> app;

  spdlog::set_level(spdlog::level::info);

  CROW_ROUTE(app, "/")
  ([&] {
    return "hell, world!";
  });

  app.loglevel(crow::LogLevel::WARNING);
  app.port(apiserver::bind_port.value())
          .bindaddr(apiserver::bind_ip.value())
          .run();

  return 0;
}
