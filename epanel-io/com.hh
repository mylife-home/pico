#pragma once

#include "i2c_slave.h" // need this for i2c_slave_event_t (cannot forward declare it)
#include "common/service.hh"

namespace mylife {
  class state;
  class transaction;

  class com : public service {
  public:
    com(uint8_t address);
    virtual ~com() = default;

    virtual void setup() override;

  private:
    static void i2c_handler_s(i2c_inst_t *i2c, i2c_slave_event_t event);
    void i2c_handler(i2c_inst_t *i2c, i2c_slave_event_t event);
    void init_transaction(uint8_t reg);
    void finish_transaction();

    void inputs_changed();

    static com *s_instance;
    state *m_state = nullptr;
    uint8_t m_address;
    transaction *m_transaction = nullptr;
  };

}
