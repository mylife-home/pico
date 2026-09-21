#pragma once

#include <memory>
#include <optional>
#include "i2c_slave.h" // need this for i2c_slave_event_t (cannot forward declare it)
#include "service.hh"

namespace mylife {
  class state;
  class internal_temp;
  class transaction;

  class com : public service {
  public:
    com(uint8_t address);
    virtual ~com();

    virtual void setup() override;

  private:
    static void i2c_handler_s(i2c_inst_t *i2c, i2c_slave_event_t event);
    void i2c_handler(i2c_inst_t *i2c, i2c_slave_event_t event);
    void init_transaction(uint8_t reg);
    void finish_transaction();

    void inputs_changed();

    static com *s_instance;
    state *m_state = nullptr;
    internal_temp *m_temp = nullptr;
    uint8_t m_address;

    // allocated once at construction: the optional then holds the transaction
    // in place, so a transfer never touches the heap
    std::unique_ptr<std::optional<transaction>> m_transaction;
    bool m_expect_reg = true; // next received byte is a register, not a payload
  };

}
