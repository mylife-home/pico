#pragma once

#include "service.hh"

namespace mylife {
  class state;
  class zero_crossing_detector;

  class dimmer : public service {
  public:
    dimmer(uint gpio, uint8_t state_index);
    virtual ~dimmer() = default;

    virtual void setup() override;

  protected:
    virtual void setup_with_zc(zero_crossing_detector *zc) {
    }

    uint8_t get_state() const;
    void set_gpio(bool value);

  private:
    uint m_gpio;
    state *m_state;
    uint8_t m_state_index;
  };

  class trailing_edge_dimmer : public dimmer {
  public:
    trailing_edge_dimmer(uint gpio, uint8_t state_index);
    virtual ~trailing_edge_dimmer() = default;

  protected:
    virtual void setup_with_zc(zero_crossing_detector *zc) override;

  private:
    static int64_t s_end(alarm_id_t id, void *user_data);
  };

  class leading_pulse_dimmer : public dimmer {
  public:
    leading_pulse_dimmer(uint gpio, uint8_t state_index);
    virtual ~leading_pulse_dimmer() = default;

  protected:
    virtual void setup_with_zc(zero_crossing_detector *zc) override;

  private:
    static int64_t s_trigger(alarm_id_t id, void *user_data);
    static int64_t s_end(alarm_id_t id, void *user_data);
  };
}