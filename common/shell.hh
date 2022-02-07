#pragma once

#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include "service.hh"

namespace mylife {

  class shell : public service {
  public:
    typedef std::function<void(const std::vector<std::string> &)> handler_t;

    virtual ~shell() = default;

    virtual void setup() override;
    virtual void loop() override;
    void register_command(std::string_view cmd, handler_t handler);

  private:
    void execute(const std::string &line) const;

    std::unordered_map<std::string_view, handler_t> m_handlers;
    std::string m_current_line;
  };
}
