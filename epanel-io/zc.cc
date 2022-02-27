#include "zc.hh"

namespace mylife {
  zero_crossing_detector *zero_crossing_detector::m_instance = nullptr;

  zero_crossing_detector::zero_crossing_detector(uint gpio)
   : m_gpio(gpio) {
    m_instance = this;
  }

  void zero_crossing_detector::setup() {
    // To see: maybe setup on core1?
    alarm_pool_init_default();

    gpio_init(m_gpio);
    gpio_set_dir(m_gpio, GPIO_IN);

    // rising_per_sec rising per second
    gpio_set_irq_enabled_with_callback(m_gpio, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &s_irq_handler);
  }

  void zero_crossing_detector::irq_handler(uint32_t events) {
    auto now = get_absolute_time();
    auto elapsed = absolute_time_diff_us(m_last_event, now);
    m_last_event = now;

    switch(events) {
    case GPIO_IRQ_EDGE_RISE:
      m_period = m_zero_duration + elapsed;

      // middle of zero
      add_alarm_in_us(m_zero_duration / 2, s_alarm_trigger, this, true);
      break;

    case GPIO_IRQ_EDGE_FALL:
      m_zero_duration = elapsed;
      break;
    }
  }

  void zero_crossing_detector::alarm_trigger() {
    m_callback.call();
  }

  void zero_crossing_detector::s_irq_handler(uint gpio, uint32_t events) {
    m_instance->irq_handler(events);
  }

  int64_t zero_crossing_detector::s_alarm_trigger(alarm_id_t id, void *user_data) {
    auto instance = reinterpret_cast<zero_crossing_detector *>(user_data);
    instance->alarm_trigger();
    return 0;
  }
}