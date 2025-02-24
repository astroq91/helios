#pragma once

#ifndef LOG_H
#define LOG_H

#include <memory>
#include <spdlog/spdlog.h>

namespace Helios {
class Log {
public:
  static void init();

  static std::shared_ptr<spdlog::logger> &get_core_logger() {
    return s_core_logger;
  }

private:
  static std::shared_ptr<spdlog::logger> s_core_logger;
};
} // namespace Helios

// Core log macros
#define HL_INFO(...) ::Helios::Log::get_core_logger()->info(__VA_ARGS__)
#define HL_WARN(...) ::Helios::Log::get_core_logger()->warn(__VA_ARGS__)
#define HL_DEBUG(...) ::Helios::Log::get_core_logger()->debug(__VA_ARGS__)
#define HL_TRACE(...) ::Helios::Log::get_core_logger()->trace(__VA_ARGS__)
#define HL_ERROR(...) ::Helios::Log::get_core_logger()->error(__VA_ARGS__)
#define HL_CRITICAL(...) ::Helios::Log::get_core_logger()->critical(__VA_ARGS__)

#endif
