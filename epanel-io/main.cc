#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "application.hh"
#include "status_blink.hh"
#include "shell.hh"
#include "logger.hh"
#include "state.hh"
#include "com.hh"
#include "zc.hh"
#include "button.hh"
#include "dimmer.hh"

#define GPIO_ZC 13 // green
#define GPIO_BUTTON 14 // yellow
#define GPIO_MOSFET 15 // orange

int main() {
  bi_decl(bi_program_description("epanel-io"));

  auto logger_name = "main";

  stdio_init_all();

  mylife::application::init();

  auto app = mylife::application::instance();

  app->register_service("status_blink", new mylife::status_blink());
  app->register_service("shell", new mylife::shell());
  app->register_service("logger", new mylife::logger());
  app->register_service("state", new mylife::state());
  app->register_service("com", new mylife::com(0x01));
  app->register_service("zc", new mylife::zero_crossing_detector(GPIO_ZC));
  app->register_service("button0", new mylife::button(GPIO_BUTTON, 0));
  app->register_service("dimmer0", new mylife::trailing_edge_dimmer(GPIO_MOSFET, 0));
  
  app->setup();

  while (true) {
    app->loop();
  }

}