#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "application.hh"
#include "dimmer.hh"
#include "zc.hh"
#include "state.hh"

namespace mylife {

  dimmer::dimmer(uint gpio, uint8_t state_index)
   : m_gpio(gpio)
   , m_state_index(state_index) {
  }

  void dimmer::setup() {
    m_state = static_cast<state *>(application::instance()->get_service("state"));

    gpio_init(m_gpio);
    gpio_set_dir(m_gpio, GPIO_OUT);

    auto zc = static_cast<zero_crossing_detector *>(application::instance()->get_service("zc"));
    setup_with_zc(zc);
  }

  uint8_t dimmer::get_state() const {
    return m_state->get_output(m_state_index);
  }

  void dimmer::set_gpio(bool value) {
    gpio_put(m_gpio, value ? 1 : 0);
  }
  
  trailing_edge_dimmer::trailing_edge_dimmer(uint gpio, uint8_t state_index)
    : dimmer(gpio, state_index) {
  }

  void trailing_edge_dimmer::setup_with_zc(zero_crossing_detector *zc) {
    zc->register_callback([this, zc]() {
      auto value = get_state();
      if (value == 0) {
        set_gpio(false);
        return;
      }
      
      if (value == 255) {
        set_gpio(true);
        return;
      }

      set_gpio(true);

      auto delay_us = zc->period_us() * get_state() / 255;
      add_alarm_in_us(delay_us, &s_end, this, true);
    });
  }

  int64_t trailing_edge_dimmer::s_end(alarm_id_t id, void *user_data) {
    auto dimmer = reinterpret_cast<trailing_edge_dimmer *>(user_data);
    dimmer->set_gpio(false);
    return 0;
  }

  // from esphome
  /// Time in microseconds the gate should be held high
  /// 10µs should be long enough for most triacs
  /// For reference: BT136 datasheet says 2µs nominal (page 7)
  /// However other factors like gate driver propagation time
  /// are also considered and a really low value is not important
  /// See also: https://github.com/esphome/issues/issues/1632
  static constexpr uint32_t GATE_ENABLE_TIME = 50;

  leading_pulse_dimmer::leading_pulse_dimmer(uint gpio, uint8_t state_index)
    : dimmer(gpio, state_index) {
  }

  void leading_pulse_dimmer::setup_with_zc(zero_crossing_detector *zc) {
    zc->register_callback([&]() {
      // set_gpio(false);

      auto delay_us = zc->period_us() * (255 - get_state()) / 255;
      add_alarm_in_us(delay_us, &s_trigger, this, true);
    });
  }

  int64_t leading_pulse_dimmer::s_trigger(alarm_id_t id, void *user_data) {
    auto dimmer = reinterpret_cast<leading_pulse_dimmer *>(user_data);
    
    dimmer->set_gpio(true);
    add_alarm_in_us(GATE_ENABLE_TIME, &s_end, dimmer, true);

    return 0;
  }

  int64_t leading_pulse_dimmer::s_end(alarm_id_t id, void *user_data) {
    auto dimmer = reinterpret_cast<leading_pulse_dimmer *>(user_data);
    dimmer->set_gpio(false);
    return 0;
  }
}