#pragma once

#include <cassert>
#include <streambuf>
#include <ostream>
#include <string_view>
#include "service.hh"
#include "non_copyable.hh"

namespace mylife {

  class logger : public service {
  public:
    enum class level { debug, info, error, none };

    class log_stream final : private non_copyable, public std::ostream {
    public:
      log_stream(std::streambuf *sb, const std::string_view &name, level lvl);
      virtual ~log_stream();
    };

    logger();
    virtual ~logger() = default;

    virtual void setup() override;
    log_stream append(const std::string_view &name, level lvl);

    // faster than through application
    static logger *instance() {
      assert(s_instance);
      return s_instance;
    }

  private:
    class null_streambuf;

    static logger *s_instance;
    level m_level = level::none;
  };

}

#define LOG(lvl) mylife::logger::instance()->append(logger_name, lvl)
#define DEBUG LOG(mylife::logger::level::debug)
#define INFO LOG(mylife::logger::level::info)
#define ERROR LOG(mylife::logger::level::error)
