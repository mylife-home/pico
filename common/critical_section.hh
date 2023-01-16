#pragma once

#include "non_copyable.hh"
#include "pico/critical_section.h"

namespace mylife {

  class lock_guard : private non_copyable {
  public:
    explicit lock_guard(critical_section_t &cs)
     : m_cs(cs) {
      critical_section_enter_blocking(&m_cs);
      __dmb();
    }

    ~lock_guard() {
      __dmb();
      critical_section_exit(&m_cs);
    }

  private:
    critical_section_t &m_cs;
  };
  
  class critical_section final : private non_copyable {
  public:
    explicit critical_section() {
      critical_section_init(&m_cs);
    }

    ~critical_section() {
      critical_section_deinit(&m_cs);
    }

    lock_guard lock() {
      return lock_guard(m_cs);
    }
    
  private:
    critical_section_t m_cs;
  };

}
