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

// cf https://raw.githubusercontent.com/mylife-home/mylife-home-devices/main/epanel-io/v1/schematic-top.png
// Note: we name io from top to bottom, left to right, disrearding the numbers on schematic which are not aligned
static constexpr uint gpio_zc = 2;

static constexpr uint gpio_input_0 = 6;
static constexpr uint gpio_input_1 = 7;
static constexpr uint gpio_input_2 = 8;
static constexpr uint gpio_input_3 = 9;
static constexpr uint gpio_output_0 = 10;
static constexpr uint gpio_output_1 = 11;
static constexpr uint gpio_output_2 = 12;
static constexpr uint gpio_output_3 = 13;

static constexpr uint gpio_input_4 = 14;
static constexpr uint gpio_input_5 = 15;
static constexpr uint gpio_input_6 = 16;
static constexpr uint gpio_input_7 = 17;
static constexpr uint gpio_output_4 = 18;
static constexpr uint gpio_output_5 = 19;
static constexpr uint gpio_output_6 = 20;
static constexpr uint gpio_output_7 = 21;

static constexpr uint gpio_input_8 = 22;
static constexpr uint gpio_input_9 = 26;
static constexpr uint gpio_output_8 = 27;
static constexpr uint gpio_output_9 = 28;

#define xstr(s) str(s)
#define str(s) #s

// Define in cmake script
#ifdef I2C_ADDRESS
  #pragma message "using I2C_ADDRESS " xstr(I2C_ADDRESS)
#else
  #error "I2C_ADDRESS undefined!"
#endif

int main() {
  bi_decl(bi_program_description("epanel-io (i2c address=" xstr(I2C_ADDRESS) ")"));

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
  app->register_service("com", new mylife::com(I2C_ADDRESS));
  
  app->register_service("zc", new mylife::zero_crossing_detector(gpio_zc));
  app->register_service("button0", new mylife::button(gpio_input_0, 0));
  app->register_service("dimmer0", new mylife::trailing_edge_dimmer(gpio_output_0, 0));
  app->register_service("button1", new mylife::button(gpio_input_1, 1));
  app->register_service("dimmer1", new mylife::trailing_edge_dimmer(gpio_output_1, 1));
  app->register_service("button2", new mylife::button(gpio_input_2, 2));
  app->register_service("dimmer2", new mylife::trailing_edge_dimmer(gpio_output_2, 2));
  app->register_service("button3", new mylife::button(gpio_input_3, 3));
  app->register_service("dimmer3", new mylife::trailing_edge_dimmer(gpio_output_3, 3));
  app->register_service("button4", new mylife::button(gpio_input_4, 4));
  app->register_service("dimmer4", new mylife::trailing_edge_dimmer(gpio_output_4, 4));
  app->register_service("button5", new mylife::button(gpio_input_5, 5));
  app->register_service("dimmer5", new mylife::trailing_edge_dimmer(gpio_output_5, 5));
  app->register_service("button6", new mylife::button(gpio_input_6, 6));
  app->register_service("dimmer6", new mylife::trailing_edge_dimmer(gpio_output_6, 6));
  app->register_service("button7", new mylife::button(gpio_input_7, 7));
  app->register_service("dimmer7", new mylife::trailing_edge_dimmer(gpio_output_7, 7));
  app->register_service("button8", new mylife::button(gpio_input_8, 8));
  app->register_service("dimmer8", new mylife::trailing_edge_dimmer(gpio_output_8, 8));
  app->register_service("button9", new mylife::button(gpio_input_9, 9));
  app->register_service("dimmer9", new mylife::trailing_edge_dimmer(gpio_output_9, 9));
  
  app->setup();

  while (true) {
    app->loop();
  }

}