#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "i2c_fifo.h"
#include "common/service.hh"
#include "common/application.hh"
#include "common/logger.hh"
#include "state.hh"
#include "com.hh"

namespace mylife {
  static auto logger_name = "com";

  static constexpr uint intr_pin = 3;
  static constexpr uint sda_pin = 4; // PICO_DEFAULT_I2C_SDA_PIN;
  static constexpr uint scl_pin = 5; // PICO_DEFAULT_I2C_SCL_PIN;
  static constexpr uint baudrate = 50000; // 50 kHz

  static constexpr uint8_t reg_check = 1;
  static constexpr uint8_t reg_reset = 2;
  static constexpr uint8_t reg_inputs = 3;
  static constexpr uint8_t reg_outputs = 4;

  static constexpr uint16_t magic = 0x4242;

  com *com::s_instance = nullptr;

  // all transactions are on u16
  class transaction {
  public:
    transaction(uint8_t type)
     : m_type(type) {
       // DEBUG << "begin transaction " << static_cast<int>(type);
    }

    ~transaction() {
      // DEBUG << "end transaction";
    }

    void set_value(uint16_t value) {
      m_value.value = value;
    }

    uint16_t get_value() const {
      return m_value.value;
    }

    uint8_t get_byte() {
      assert(m_offset < 2);
      return m_value.parts[m_offset++];
    }

    void set_byte(uint8_t value) {
      assert(m_offset < 2);
      m_value.parts[m_offset++] = value;
    }

    uint8_t type() const {
      return m_type;
    }

    bool ended() const {
      return m_offset >= 2;
    }

  private:
    uint8_t m_type;

    union {
      std::uint16_t value;
      std::uint8_t  parts[2];
    } m_value;

    int m_offset = 0;
  };

  com::com(uint8_t address)
   : m_address(address) {
    s_instance = this;
  }

  static void gpio_opendrain_init(uint gpio) {
    // emulate open drain: input for value=false, output-low for value=true
    gpio_init(gpio);
    gpio_set_dir(gpio, false);
    gpio_put(gpio, false);
  }

  static void gpio_opendrain_put(uint gpio, bool value) {
    gpio_set_dir(gpio, value);
  }

  void com::setup() {
    m_state = static_cast<state *>(application::instance()->get_service("state"));
    
    gpio_opendrain_init(intr_pin);

    gpio_init(sda_pin);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    // gpio_pull_up(sda_pin);

    gpio_init(scl_pin);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    // gpio_pull_up(scl_pin);

    i2c_init(i2c0, baudrate);
    i2c_slave_init(i2c0, m_address, &i2c_handler_s);

    m_state->register_inputs_change_callback([&]() { inputs_changed(); });
  }

  void com::i2c_handler_s(i2c_inst_t *i2c, i2c_slave_event_t event) {
    s_instance->i2c_handler(i2c, event);
  }

  void com::i2c_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    switch (event) {
      case I2C_SLAVE_RECEIVE: {
        auto data = i2c_read_byte(i2c);

        if (!m_transaction) {
          init_transaction(data);
          return;
        }

        m_transaction->set_byte(data);

        if (m_transaction->ended()) {
          finish_transaction();
        }

        break;
      }

      case I2C_SLAVE_REQUEST: {
        assert(m_transaction);
        auto data = m_transaction->get_byte();
        i2c_write_byte(i2c, data);

        if (m_transaction->ended()) {
          finish_transaction();
        }

        break;
      }

      case I2C_SLAVE_FINISH:
        // esphome does not handle repeated stop, so we have to handle it though multiple queries
        break;
    }
  }

  void com::init_transaction(uint8_t reg) {
    m_transaction = new transaction(reg);

    switch(reg) {
      case reg_check:
        m_transaction->set_value(magic);
        break;

      case reg_inputs:
        m_transaction->set_value(m_state->get_inputs());
        // reset interrupt line
        gpio_opendrain_put(intr_pin, false);
        break;

      case reg_reset:
      case reg_outputs:
        break;

      default:
        ERROR << "got unknown request" << static_cast<int>(reg);
        break;
    }
  }

  void com::finish_transaction() {
    switch(m_transaction->type()) {
      case reg_check:
      case reg_inputs:
        break;

      case reg_reset:
        m_state->reset();
        break;

      case reg_outputs: {
        auto word = m_transaction->get_value();
        auto index = (uint8_t)(word >> 8);
        auto value = (uint8_t)(word & 0x00ff);
        m_state->set_output(index, value);
        break;
      }
    }

    delete m_transaction;
    m_transaction = nullptr;
  }

  void com::inputs_changed() {
    // set interrupt line on change
    gpio_opendrain_put(intr_pin, true);
  }
}
