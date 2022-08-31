#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "application.hh"
#include "button.hh"
#include "zc.hh"
#include "state.hh"

namespace mylife {

  static constexpr uint8_t change_retries = 3;

  std::optional<callback_manager<void()>> button::s_callback;

  button::button(uint gpio, uint8_t state_index)
   : m_gpio(gpio)
   , m_state_index(state_index)
   , m_change_retry(0) {
  }

  void button::setup() {
    m_state = static_cast<state *>(application::instance()->get_service("state"));

    gpio_init(m_gpio);
    gpio_set_dir(m_gpio, GPIO_IN);

    // register to ZC with delay only once
    if (!s_callback) {
      s_callback = callback_manager<void()>();

      auto zc = static_cast<zero_crossing_detector *>(application::instance()->get_service("zc"));

      zc->register_callback([this, zc]() {
        auto period = zc->period_us();
        if (period > 0) {
          // check at half-period (should be higher level possible)
          add_alarm_in_us(period / 2, &s_callback_trigger, nullptr, true);
        }
      });
    }

    // add ourself to callback
    s_callback->add([&]() {
      auto actual_value = m_state->get_input(m_state_index);
      auto new_value = !gpio_get(m_gpio); // level is reversed (like ZC detector to be high at ZC)

      // we only change the value after "change_retries" times the same value (avoid false changes)
      if (actual_value == new_value) {
        m_change_retry = 0;
        return;
      }

      ++m_change_retry;

      if (m_change_retry == change_retries) {
        m_change_retry = 0;
        m_state->set_input(m_state_index, new_value);
      }
    });
  }

  int64_t button::s_callback_trigger(alarm_id_t id, void *user_data) {
    s_callback->call();

    return 0;
  }
}