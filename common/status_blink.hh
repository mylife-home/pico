#pragma once

#include "service.hh"

namespace mylife {

  class status_blink : public service {
  public:
    virtual ~status_blink() = default;

    virtual void setup() override;
    virtual void loop() override;

  private:
    absolute_time_t m_next_time;
    bool m_next_value{false};
  };
}
