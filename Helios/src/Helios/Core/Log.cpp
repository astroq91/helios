#include "spdlog/sinks/stdout_color_sinks.h"

namespace Helios {
std::shared_ptr<spdlog::logger> Log::s_core_logger;

void Log::init() {
  spdlog::set_pattern("%^[%T] %n: %v%$");
    s_core_logger = spdlog::stdout_color_mt("HELIOS");
  s_core_logger->set_level(spdlog::level::trace);
}
} // namespace Helios
