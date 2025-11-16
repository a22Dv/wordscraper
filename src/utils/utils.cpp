/**
 * utils.cpp
 * Implementation for utils.hpp
 */

#include "utils/utils.hpp"  // Master header file.

#include <chrono>

#include "core/config.hpp"  // IWYU pragma: export
#include "core/pch.hpp"


namespace ch = std::chrono;

namespace wsc::utils
{

void windowsRequire(bool condition, const char *message)
{
  if (!condition)
  {
    logError(message);
    throw std::system_error(GetLastError(), std::system_category(), message);
  }
}

void runtimeRequire(bool condition, const char *message)
{
  if (!condition)
  {
    logError(message);
    throw std::runtime_error(message);
  }
}

void log(std::string_view severity, std::string_view message)
{
  const auto time = ch::time_point_cast<ch::milliseconds>(ch::system_clock::now());
  const auto zone = ch::current_zone();
  const auto localTime = ch::zoned_time(zone, time);
  const std::string log = std::format("[{} - {:%I:%M %p (%Ss)}] {}", severity, localTime, message);
  std::cerr << log << '\n';
}

void logError([[maybe_unused]] std::string_view message)
{
#ifdef WSC_LOGGING_ERROR
  log("WSC:ERROR", message);
#endif
}

void logInfo([[maybe_unused]] std::string_view message)
{
#ifdef WSC_LOGGING_INFO
  log("WSC:INFO", message);
#endif
}

}  // namespace wsc::utils