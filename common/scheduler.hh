#pragma once

#include <queue>
#include <functional>
#include <optional>
#include "service.hh"
#include "critical_section.hh"

namespace mylife {

  class scheduler : public service {
  public:
    typedef std::function<void()> task_t;

    virtual ~scheduler() = default;

    virtual void loop() override;
    void defer(task_t task);

  private:
    std::optional<task_t> get_one();

    std::queue<task_t> m_tasks;
    critical_section m_cs;
  };
}
