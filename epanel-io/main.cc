#include "pico/stdlib.h"
#include "common/application.hh"
#include "common/status_blink.hh"
#include "common/shell.hh"
#include "common/logger.hh"
#include "state.hh"
#include "com.hh"

int main() {
  auto logger_name = "main";

  stdio_init_all();

  mylife::application::init();

  auto app = mylife::application::instance();

  app->register_service("status_blink", new mylife::status_blink());
  app->register_service("shell", new mylife::shell());
  app->register_service("logger", new mylife::logger());
  app->register_service("state", new mylife::state());
  app->register_service("com", new mylife::com(0x01));

  app->setup();

  while(1) {
    app->loop();
  }

}