#include "pico/stdlib.h"
#include "status_blink.hh"

namespace mylife {

  static constexpr uint led_pin = PICO_DEFAULT_LED_PIN;

  void status_blink::setup() {
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);
    m_next_time = get_absolute_time();
  }

  void status_blink::loop() {
    if (!time_reached(m_next_time)) {
      return;
    }

    gpio_put(led_pin, m_next_value ? 1 : 0);

    m_next_value = !m_next_value;
    m_next_time = delayed_by_ms(m_next_time, m_next_value ? 1000 : 250);
  }

}
