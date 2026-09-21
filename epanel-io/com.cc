#include <cassert>
#include <cstdint>

#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "i2c_fifo.h"
#include "service.hh"
#include "application.hh"
#include "logger.hh"
#include "state.hh"
#include "com.hh"
#include "internal_temp.hh"

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
  static constexpr uint8_t reg_internal_temp = 5;

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

    // only a transaction that has exchanged no byte yet may survive a STOP
    bool started() const {
      return m_offset > 0;
    }

    bool ended() const {
      return m_offset >= 2;
    }

  private:
    uint8_t m_type;

    union {
      std::uint16_t value;
      std::uint8_t  parts[2];
    } m_value = {0};

    int m_offset = 0;
  };

  // esphome does not use a repeated start: reading a register is split into
  // "write [reg], STOP" then "read [lo][hi], STOP". So the selected register
  // has to survive a STOP. The byte position inside the u16 must not: keeping a
  // half received word would shift every following byte by one, forever, and
  // nothing would ever resynchronize us.
  //
  // The invariant that makes this recoverable is that the first byte received
  // after a STOP is always a register. Whatever the master did or failed to do
  // before, the next transfer starts clean.
  //
  // The transaction itself carries the byte position, so "survives a STOP"
  // means "exists and has not started yet".

  com::com(uint8_t address)
   : m_address(address)
   , m_transaction(std::make_unique<std::optional<transaction>>()) {
    s_instance = this;
  }

  com::~com() = default;

  static void gpio_opendrain_init(uint gpio) {
    // emulate open drain: input for value=false, output-low for value=true
    gpio_init(gpio);
    gpio_disable_pulls(gpio);
    gpio_put(gpio, false);
    gpio_set_dir(gpio, false);
  }

  static void gpio_opendrain_put(uint gpio, bool value) {
    gpio_set_dir(gpio, value);
  }

  void com::setup() {
    m_state = static_cast<state *>(application::instance()->get_service("state"));
    m_temp = static_cast<internal_temp *>(application::instance()->get_service("internal_temp"));
    
    gpio_opendrain_init(intr_pin);

    gpio_init(sda_pin);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_disable_pulls(sda_pin);
    // gpio_pull_up(sda_pin);

    gpio_init(scl_pin);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_disable_pulls(scl_pin);
    // gpio_pull_up(scl_pin);

    i2c_init(i2c0, baudrate);
    i2c_slave_init(i2c0, m_address, &i2c_handler_s);

    m_state->register_inputs_change_callback([&]() { inputs_changed(); });
  }

  void com::i2c_handler_s(i2c_inst_t *i2c, i2c_slave_event_t event) {
    s_instance->i2c_handler(i2c, event);
  }

  void com::i2c_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    auto &tx = *m_transaction;

    switch (event) {
      case I2C_SLAVE_RECEIVE: {
        auto data = i2c_read_byte(i2c);

        // a byte is a register if a STOP said so, or if nothing is in progress
        if (m_expect_reg || !tx) {
          m_expect_reg = false;
          init_transaction(data);
          break;
        }

        if (tx->ended()) {
          // more payload than we expect: swallow it
          break;
        }

        tx->set_byte(data);

        if (tx->ended()) {
          finish_transaction();
        }

        break;
      }

      case I2C_SLAVE_REQUEST: {
        // a read never starts a transfer, it continues the one whose register
        // was selected by the preceding write
        uint8_t data = 0xff;

        if (tx && !tx->ended()) {
          data = tx->get_byte();
        }

        i2c_write_byte(i2c, data);

        if (tx && tx->ended()) {
          finish_transaction();
        }

        break;
      }

      case I2C_SLAVE_FINISH:
        // A STOP always ends a transfer, so the next byte we receive is a
        // register again, whatever happened before. Only the register selection
        // survives, because esphome puts a STOP between selecting a register
        // and reading its value.
        m_expect_reg = true;

        if (tx && tx->started()) {
          // aborted in the middle of a word: the payload is unusable, and so is
          // the register it belonged to
          tx.reset();
        }

        break;
    }
  }

  void com::init_transaction(uint8_t reg) {
    auto &tx = *m_transaction;

    tx.emplace(reg);

    switch(reg) {
      case reg_check:
        tx->set_value(magic);
        break;

      case reg_inputs:
        tx->set_value(m_state->get_inputs());
        break;

      case reg_internal_temp:
        tx->set_value(m_temp->get_raw());
        break;

      case reg_reset:
      case reg_outputs:
        break;

      default:
        // kept, so the payload is drained instead of being taken for registers
        ERROR << "got unknown request " << static_cast<int>(reg);
        break;
    }
  }

  void com::finish_transaction() {
    auto &tx = *m_transaction;

    switch(tx->type()) {
      case reg_check:
      case reg_internal_temp:
        break;

      case reg_inputs:
        // only acknowledge if what the master got is still current
        if (m_state->get_inputs() == tx->get_value()) {
          gpio_opendrain_put(intr_pin, false);
        }
        break;

      case reg_reset:
        m_state->reset();
        // reset interrupt line
        gpio_opendrain_put(intr_pin, false);
        break;

      case reg_outputs: {
        auto word = tx->get_value();
        auto index = (uint8_t)(word >> 8);
        auto value = (uint8_t)(word & 0x00ff);

        if (index >= 16) {
          ERROR << "got out of range output index " << static_cast<int>(index);
          break;
        }

        m_state->set_output(index, value);
        break;
      }

      default:
        break;
    }

    tx.reset();
  }

  void com::inputs_changed() {
    // set interrupt line on change
    gpio_opendrain_put(intr_pin, true);
  }
}
