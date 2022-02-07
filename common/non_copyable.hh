#pragma once

namespace mylife {
  class non_copyable {
  protected:
    constexpr non_copyable() = default;
    constexpr non_copyable(non_copyable&&) = default;
    ~non_copyable() = default;
    non_copyable& operator=(non_copyable&&) = default;

    non_copyable(const non_copyable&) = delete;
    non_copyable& operator=(const non_copyable&) = delete;
  };
}
