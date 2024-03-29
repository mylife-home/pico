#include <iostream>
#include "application.hh"
#include "shell.hh"
#include "logger.hh"
#include "state.hh"

namespace mylife {
  static auto logger_name = "state";

  static void input_set_help() {
    std::cout << "state-input-set <index> <value>" << std::endl;
    std::cout << "index: 0-15" << std::endl;
    std::cout << "value: 0-1" << std::endl;
  }

  static void output_set_help() {
    std::cout << "state-output-set <index> <value>" << std::endl;
    std::cout << "index: 0-15" << std::endl;
    std::cout << "value: 0-255" << std::endl;
  }

  void state::setup() {
    auto *sh = static_cast<shell *>(application::instance()->get_service("shell"));
    m_scheduler = static_cast<scheduler *>(application::instance()->get_service("scheduler"));

    sh->register_command("state-inputs-get", [&](const std::vector<std::string> & args) {
      std::cout << "Current inputs are" << std::endl;
      for (int index=0; index<16; ++index) {
        std::cout << index << " = " << get_input(index) << std::endl;
      }
    });

    sh->register_command("state-input-set", [&](const std::vector<std::string> & args) {
      if (args.size() < 2) {
        input_set_help();
        return;
      }

      auto index = std::stoi(args[0]);
      auto value = std::stoi(args[1]);

      if (index < 0 || index > 15 || value < 0 || value > 1) {
        input_set_help();
        return;
      }

      set_input(index, value == 1);
      std::cout << index << " = " << get_input(index) << std::endl;
    });

    sh->register_command("state-outputs-get", [&](const std::vector<std::string> & args) {
      std::cout << "Current inputs are" << std::endl;
      for (int index=0; index<16; ++index) {
        std::cout << index << " = " << static_cast<uint32_t>(get_output(index)) << std::endl;
      }
    });

    sh->register_command("state-output-set", [&](const std::vector<std::string> & args) {
      if (args.size() < 2) {
        output_set_help();
        return;
      }

      auto index = std::stoi(args[0]);
      auto value = std::stoi(args[1]);

      if (index < 0 || index > 15 || value < 0 || value > 255) {
        output_set_help();
        return;
      }

      set_output(index, value);
      std::cout << index << " = " << static_cast<uint32_t>(get_output(index)) << std::endl;
    });
  }

  void state::set_input(uint8_t index, bool value) {
    assert(index >= 0 && index < 16);

    {
      auto lock = m_cs.lock();

      bool old_value = m_inputs.test(index);
      if (old_value == value) {
        return;
      }

      m_inputs.set(index, value);
    }

    m_scheduler->defer([this, index, value] () {
      for (const auto &callback : m_callbacks) {
        callback();
      }

      DEBUG << "set input " << static_cast<uint32_t>(index) << " = " << value;
    });
  }

  bool state::get_input(uint8_t index) const {
    assert(index >= 0 && index < 16);
    auto lock = m_cs.lock();
    return m_inputs.test(index);
  }

  uint16_t state::get_inputs() const {
    auto lock = m_cs.lock();
    return static_cast<uint16_t>(m_inputs.to_ulong());
  }

  void state::set_output(uint8_t index, uint8_t value) {
    assert(index >= 0 && index < 16);

    {
      auto lock = m_cs.lock();

      if (m_outputs[index] == value) {
        return;
      }

      m_outputs[index] = value;
    }

    m_scheduler->defer([this, index, value] () {
      DEBUG << "set output " << static_cast<uint32_t>(index) << " = " << static_cast<uint32_t>(value);
    });
  }

  uint8_t state::get_output(uint8_t index) const {
    assert(index >= 0 && index < 16);
    auto lock = m_cs.lock();
    return m_outputs[index];
  }

  void state::reset() {
    {
      auto lock = m_cs.lock();
      
      for (int index=0; index<16; ++index) {
        m_outputs[index] = 0;
      }
    }

    m_scheduler->defer([this] () {
      DEBUG << "reset";
    });
  }

  void state::register_inputs_change_callback(std::function<void()> callback) {
    m_callbacks.emplace_back(std::move(callback));
  }
}

