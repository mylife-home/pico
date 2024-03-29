#pragma once

#include <cstdint>
#include <array>
#include <functional>
#include <bitset>
#include "service.hh"
#include "scheduler.hh"
#include "critical_section.hh"

namespace mylife {

  class state : public service {
  public:
    virtual ~state() = default;
    virtual void setup() override;

    void set_input(uint8_t index, bool value);
    bool get_input(uint8_t index) const;
    uint16_t get_inputs() const;
    void set_output(uint8_t index, uint8_t value);
    uint8_t get_output(uint8_t index) const;

    void reset();

    void register_inputs_change_callback(std::function<void()> callback);

  private:
    mutable critical_section m_cs;
    std::bitset<16> m_inputs = {0};
    std::array<uint8_t, 16> m_outputs = {0};
    std::vector<std::function<void()>> m_callbacks;
    scheduler *m_scheduler = nullptr;
  };

}
