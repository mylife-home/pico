#include <iostream>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "internal_temp.hh"
#include "application.hh"
#include "shell.hh"

namespace mylife {

  void internal_temp::setup() {
    adc_init();
    adc_select_input(4);
    adc_set_temp_sensor_enabled(true);

    m_next_time = get_absolute_time();

    auto *sh = static_cast<shell *>(application::instance()->get_service("shell"));

    sh->register_command("internal-temp-read", [&](const std::vector<std::string> & args) {
      std::cout << "Internal temp: " << get_temp() << " C (raw=" << get_raw() << ")" << std::endl;
    });
  }

  void internal_temp::loop() {
    if (!time_reached(m_next_time)) {
      return;
    }

    m_raw = adc_read();

    m_next_value = !m_next_value;
    m_next_time = delayed_by_ms(m_next_time, m_next_value ? 1000 : 250);
  }

  uint16_t internal_temp::get_raw() const {
    return m_raw;
  }

  // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
  static constexpr const float conversion_factor = 3.3f / (1 << 12);

  float internal_temp::get_temp() const {
    float adc_voltage = get_raw() * conversion_factor;
    // T = 27 - (ADC_Voltage - 0.706)/0.001721
    return 27 - (adc_voltage - 0.706) / 0.001721;
  }
}
