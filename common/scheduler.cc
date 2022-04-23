#include "scheduler.hh"

namespace mylife {
  void scheduler::loop() {
    std::optional<scheduler::task_t> task;

    while (task = get_one()) {
      (*task)();
    }
  }

  std::optional<scheduler::task_t> scheduler::get_one() {
    auto lock = m_cs.lock();

    if (m_tasks.empty()) {
      return std::nullopt;
    }

    auto task = std::move(m_tasks.front());
    m_tasks.pop();
    return task;
  }

  void scheduler::defer(task_t task) {
    auto lock = m_cs.lock();
    m_tasks.emplace(std::move(task));
  }
}
