#include "application.hh"
#include "service.hh"

namespace mylife {
    
  void application::register_service(const std::string_view &name, service *svc) {
    m_services.emplace(name, svc);
  }

  void application::setup() {
    for (const auto &[name, svc] : m_services) {
      svc->setup();
    }
  }

  void application::loop() {
    for (const auto &[name, svc] : m_services) {
      svc->loop();
    }
  }

  void application::init() {
    s_instance = new application();
  }

  application *application::s_instance = nullptr;
}
