#pragma once

#include <unordered_map>
#include <string_view>
#include <cassert>

namespace mylife {
  class service;
    
  class application {
    application() = default;

  public:
    void register_service(const std::string_view &name, service *svc);

    service *get_service(const std::string_view &name) {
      auto it = m_services.find(name);
      assert(it != m_services.end());
      return it->second;
    }

    void setup();

    void loop();

    static application* instance() {
      return s_instance;
    }

    static void init();

  private:
    static application *s_instance;
    std::unordered_map<std::string_view, service *> m_services;
  };
}
