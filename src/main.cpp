#include <crow/app.h>
#include <perfkit/configs.h>
#include <perfkit/extension/net-provider.hpp>
#include <spdlog/spdlog.h>

#include "app.hpp"
#include "middlewares/auth.hpp"

using namespace std::literals;

PERFKIT_CATEGORY(apiserver) {
  PERFKIT_CONFIGURE(bind_ip, "0.0.0.0").make_flag(true).confirm();
  PERFKIT_CONFIGURE(bind_port, 15572).make_flag(true).confirm();
}

PERFKIT_CATEGORY(provider) {
  PERFKIT_CONFIGURE(bind_ip, "0.0.0.0").make_flag(true).confirm();
  PERFKIT_CONFIGURE(bind_port, 25572).make_flag(true).confirm();
}

int main(int argc, char** argv) {
  perfkit::configs::parse_args(&argc, &argv, true);

  perfkit::terminal_ptr term;
  {
    perfkit::terminal::net_provider::init_info term_init{"__SERVER__"};
    term_init.wait_connection = false;
    term_init.host_port       = *provider::bind_port;
    term_init.host_name       = "127.0.0.1";

    term = perfkit::terminal::net_provider::create(term_init);
    spdlog::default_logger()->sinks().push_back(term->sink());
    perfkit::terminal::register_logging_manip_command(term.get());
  }

  std::unique_ptr<apiserver::app> instance;
  {
    apiserver::app::init_arg init;
    init.bind_ip   = *provider::bind_ip;
    init.bind_port = *provider::bind_port;

    instance = std::make_unique<apiserver::app>(init);
  }

  crow::App<middleware::auth> app;
  CROW_ROUTE(app, "/")
  ([] {
    return "hell, world!";
  });

  CROW_ROUTE(app, "/login")
  ([] {
    // TODO
    std::this_thread::sleep_for(5s);
    return "trying login";
  });

  CROW_ROUTE(app, "/sessions")
  ([] {
    // TODO
    return "enumerating sessions";
  });

  app.port(apiserver::bind_port.value())
          .bindaddr(apiserver::bind_ip.value())
          .multithreaded()
          .run();
}
