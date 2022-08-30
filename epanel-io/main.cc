#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "application.hh"
#include "status_blink.hh"
#include "internal_temp.hh"
#include "shell.hh"
#include "scheduler.hh"
#include "logger.hh"
#include "state.hh"
#include "com.hh"
#include "zc.hh"
#include "button.hh"
#include "dimmer.hh"

#define GPIO_ZC 13 // green
#define GPIO_BUTTON_0 14 // yellow
#define GPIO_MOSFET_0 15 // orange
#define GPIO_BUTTON_1 16 // black
#define GPIO_MOSFET_1 17 // braun

int main() {
  bi_decl(bi_program_description("epanel-io"));

  auto logger_name = "main";

  stdio_init_all();

  mylife::application::init();

  auto app = mylife::application::instance();

  app->register_service("status_blink", new mylife::status_blink());
  app->register_service("internal_temp", new mylife::internal_temp());
  app->register_service("shell", new mylife::shell());
  app->register_service("scheduler", new mylife::scheduler());
  app->register_service("logger", new mylife::logger());
  app->register_service("state", new mylife::state());
  app->register_service("com", new mylife::com(0x01));
  app->register_service("zc", new mylife::zero_crossing_detector(GPIO_ZC));
  app->register_service("button0", new mylife::button(GPIO_BUTTON_0, 0));
  app->register_service("dimmer0", new mylife::trailing_edge_dimmer(GPIO_MOSFET_0, 0));
  app->register_service("button1", new mylife::button(GPIO_BUTTON_1, 1));
  app->register_service("dimmer1", new mylife::trailing_edge_dimmer(GPIO_MOSFET_1, 1));
  
  app->setup();

  while (true) {
    app->loop();
  }

}