#pragma once

#include <optional>
#include "service.hh"
#include "callback.hh"

namespace mylife {
  class state;
  class zero_crossing_detector;

  class button : public service {
  public:
    button(uint gpio, uint8_t state_index);
    virtual ~button() = default;

    virtual void setup() override;

  private:
    static int64_t s_callback_trigger(alarm_id_t id, void *user_data);

    uint m_gpio;
    state *m_state;
    uint8_t m_state_index;
    uint8_t m_change_retry;

    static std::optional<callback_manager<void()>> s_callback;
  };
}