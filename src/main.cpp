#include <atomic>

#include <crow/app.h>
#include <perfkit/configs.h>
#include <perfkit/detail/commands.hpp>
#include <perfkit/extension/cli.hpp>
#include <perfkit/extension/net-provider.hpp>
#include <spdlog/spdlog.h>

#include "app.hpp"
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
  std::unique_ptr<apiserver::app> srv_app;
  perfkit::terminal_ptr term;
  perfkit::terminal_ptr term_cli;
  std::thread thrd_term_cli;
  std::atomic_bool running{true};

  {  // initialize net terminal, which will connect to itself.
    perfkit::terminal::net_provider::init_info term_init{"__MONITORING_SERVER__"};
    term_init.wait_connection = false;
    term_init.host_port       = *provider::bind_port;
    term_init.host_name       = "127.0.0.1";

    term = perfkit::terminal::net_provider::create(term_init);
    perfkit::terminal::register_logging_manip_command(term.get());
  }
  {  // register terminal manipulation
    term_cli = perfkit::terminal::create_cli();
    perfkit::terminal::initialize_with_basic_commands(term_cli.get());
    term_cli->commands()->root()->add_subcommand(
            "quit", [&](auto&&) { return app.stop(), (running = false), true; });
    thrd_term_cli = std::thread{
            [&] {
              while (running.load(std::memory_order_relaxed)) {
                auto cmd = term_cli->fetch_command(10ms);
                if (cmd && not cmd->empty())
                  term_cli->commands()->invoke_command(*cmd);

                cmd = term->fetch_command(10ms);
                if (cmd && not cmd->empty()) {
                  spdlog::info("command from network: {}", *cmd);
                  term->commands()->invoke_command(*cmd);
                }
              }
            }};
  }
  {  // initialize server app
    apiserver::app::init_arg init;
    init.bind_ip   = *provider::bind_ip;
    init.bind_port = *provider::bind_port;

    srv_app = std::make_unique<apiserver::app>(init);
  }

  spdlog::set_level(spdlog::level::info);

  CROW_ROUTE(app, "/")
  ([&] {
    return "hell, world!";
  });

  CROW_ROUTE(app, "/ping")
  ([] {
    return std::to_string(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count());
  });

  CROW_ROUTE(app, "/login")
  ([&] {
    // TODO
    std::this_thread::sleep_for(5s);
    return "trying login";
  });

  CROW_ROUTE(app, "/sessions")
  ([&] {
    return srv_app->list_sessions();
  });

  CROW_ROUTE(app, "/shell/<int>/<int>")
  ([&](int64_t sess_id, int64_t seqn) {
    return srv_app->fetch_shell_output(sess_id, seqn);
  });

  CROW_ROUTE(app, "/shell/<int>")
          .methods(crow::HTTPMethod::POST)(
                  [&](crow::request const& req, int64_t sess_id) {
                    return srv_app->post_shell_input(sess_id, req.body);
                  });

  app.loglevel(crow::LogLevel::WARNING);

  app.port(apiserver::bind_port.value())
          .bindaddr(apiserver::bind_ip.value())
          .run();

  running.store(false);
  if (thrd_term_cli.joinable()) { thrd_term_cli.join(); }
  srv_app.reset();

  return 0;
}
