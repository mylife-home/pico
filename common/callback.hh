#pragma once

#include <functional>
#include <vector>

namespace mylife {
  template<typename... X>
  class callback_manager;

  template<typename... Args>
  class callback_manager<void(Args...)> final {
  public:
    void add(std::function<void(Args...)> &&callback) {
      m_callbacks.emplace_back(std::move(callback));
    }

    void call(Args... args) {
      for (const auto &callback : m_callbacks) {
        callback(args...);
      }
    }

  private:
    std::vector<std::function<void(Args...)>> m_callbacks;
  };
}