#pragma once

#include "service.hh"

namespace mylife {

  class internal_temp : public service {
  public:
    virtual ~internal_temp() = default;

    virtual void setup() override;
    virtual void loop() override;

    uint16_t get_raw() const;
    float get_temp() const;

  private:
    absolute_time_t m_next_time;
    bool m_next_value{false};
    uint16_t m_raw;
  };
}
