#include <iostream>
#include "application.hh"
#include "logger.hh"
#include "shell.hh"

namespace mylife {

  logger::logger() {
    s_instance = this;
  }

  void logger::setup() {
    auto *sh = static_cast<shell *>(application::instance()->get_service("shell"));

    sh->register_command("logger-level-get", [&](const std::vector<std::string> & args) {
      std::cout << "Current logger level is '";

      switch (m_level) {
      case level::debug:
        std::cout << "debug";
        break;

      case level::info:
        std::cout << "info";
        break;

      case level::error:
        std::cout << "error";
        break;

      case level::none:
        std::cout << "none";
        break;
      }

      std::cout << "'" << std::endl;
    });

    sh->register_command("logger-level-set", [&](const std::vector<std::string> & args) {
      if (args.size() < 1) {
        std::cout << "No level provided." << std::endl;
      }

      const auto &slevel = args.front();

      if (slevel == "debug") {
        m_level = level::debug;
        std::cout << "Logger level set to 'debug'" << std::endl;
      } else if (slevel == "info") {
        m_level = level::info;
        std::cout << "Logger level set to 'info'" << std::endl;
      } else if (slevel == "error") {
        m_level = level::error;
        std::cout << "Logger level set to 'error'" << std::endl;
      } else if (slevel == "none") {
        m_level = level::none;
        std::cout << "Logger level set to 'none'" << std::endl;
      } else {
        std::cout << "Invalid level provided '" << slevel << "'" << std::endl;
      }
    });
  }

  logger::log_stream::log_stream(std::streambuf *sb, const std::string_view &name, level lvl)
  : std::ostream(sb) {
    auto &os = (*this);

    switch (lvl) {
    case level::debug:
      os << "[debug]";
      break;

    case level::info:
      os << "[info]";
      break;

    case level::error:
      os << "[error]";
      break;
    }

    os << " " << name << " - ";
  }

  logger::log_stream::~log_stream() {
    (*this) << std::endl;
  }

  class logger::null_streambuf : public std::streambuf {
  public:
    int overflow(int c) {
      return c;
    }

    static null_streambuf s_instance;
  };

  logger::log_stream logger::append(const std::string_view &name, level lvl) {
    auto sb = lvl >= m_level ? std::cout.rdbuf() : &null_streambuf::s_instance;
    return log_stream(sb, name, lvl);
  }

  logger* logger::s_instance = nullptr;
  logger::null_streambuf logger::null_streambuf::s_instance;
}

