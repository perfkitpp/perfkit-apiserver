#include <crow/app.h>
#include <perfkit/configs.h>

#include "middlewares/auth.hpp"

PERFKIT_CATEGORY(apiserver) {
  PERFKIT_CONFIGURE(bind_ip, "0.0.0.0").make_flag(true).confirm();
  PERFKIT_CONFIGURE(bind_port, 15572).make_flag(true).confirm();
}

int main(int argc, char** argv) {
  perfkit::configs::parse_args(&argc, &argv, true);

  crow::App<middleware::auth> app;
  CROW_ROUTE(app, "/")
  ([] {
    return "hell, world!";
  });

  app.port(apiserver::bind_port.value())
          .bindaddr(apiserver::bind_ip.value())
          .run();
}
