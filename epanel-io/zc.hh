#pragma once

#include "pico/stdlib.h"
#include "callback.hh"
#include "service.hh"

namespace mylife {

  class zero_crossing_detector : public service {
  public:
    zero_crossing_detector(uint gpio);
    virtual ~zero_crossing_detector() = default;
    virtual void setup() override;

    uint32_t period_us() const {
      return m_period;
    }

    uint32_t zero_duration_us() const {
      return m_zero_duration;
    }

    void register_callback(std::function<void()> &&callback) {
      m_callback.add(std::move(callback));
    }

  private:
    void irq_handler(uint32_t events);
    void alarm_trigger();
    static void s_irq_handler(uint gpio, uint32_t events);
    static int64_t s_alarm_trigger(alarm_id_t id, void *user_data);

    // this is ugly but we cannot pass args to irq handler
    static zero_crossing_detector *m_instance;

    uint m_gpio;

    absolute_time_t ABSOLUTE_TIME_INITIALIZED_VAR(m_last_event, 0);
    uint32_t m_period = 0;
    uint32_t m_zero_duration = 0;

    callback_manager<void()> m_callback;
  };
}