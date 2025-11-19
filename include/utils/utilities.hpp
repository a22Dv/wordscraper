/**
 * utilities.hpp
 *
 * Project-wide utilities.
 */

#pragma once

#include "core/config.hpp"
#include "core/pch.hpp"

#define WSR_EXCEPTMSG(varname) static constexpr const char varname[]

#define WSR_TO_LITERAL_IMPL_(tokens) #tokens
#define WSR_TO_LITERAL(tokens) \
  WSR_TO_LITERAL_IMPL_(tokens)  // Expand possible macro before #-operator.

#if defined(NDEBUG) && !defined(WSR_NOPROFILE)
  #define WSR_PROFILE_SCOPE() \
    ZoneScoped(wsr::utils::ConcatenateLiterals<__func__, "()">::concat().data())
  #define WSR_PROFILE_SCOPEN(name) \
    ZoneScoped(wsr::utils::ConcatenateLiterals<__func__, "(): ", name>::concat().data())
#else
  #define WSR_PROFILE_SCOPE()
  #define WSR_PROFILE_SCOPEN(name)
#endif

#if defined(NDEBUG) && !defined(WSR_NOASSERT)
  #define WSR_ASSERT(condition)                                                                 \
    do {                                                                                        \
      if (!(condition)) {                                                                       \
        std::cerr << wsr::utils::ConcatenateLiterals<"Assertion Failed: ", #condition, "\nIn ", \
                                                     __FILE__, " at ", __func__,                \
                                                     "():", WSR_TO_LITERAL(__LINE__)>::concat() \
                         .data()                                                                \
                  << std::endl;                                                                 \
        std::cerr << "Continue? [Y/N]: ";                                                       \
        char ch = {};                                                                           \
        std::cin >> ch;                                                                         \
        if ((ch & ~0x20) == 'Y') {                                                              \
          DebugBreak();                                                                         \
        } else {                                                                                \
          std::abort();                                                                         \
        }                                                                                       \
      }                                                                                         \
    } while (false)
#else
  #define WSR_ASSERT(condition)
#endif

#define WSR_EXCEPTION(message)                                                                   \
  wsr::utils::ConcatenateLiterals<"An exception has occurred in:\n", __FILE__, " at ", __func__, \
                                  "():", WSR_TO_LITERAL(__LINE__), "\n", message>::concat()      \
      .data()
namespace wsr::utils {

template <std::size_t S>
struct Literal {
  std::array<char, S - 1U> string = {};
  constexpr Literal(const char (&str)[S]) noexcept {
    for (std::size_t i = {}; i < S - 1U; ++i) {
      string[i] = str[i];
    }
  }
  constexpr std::size_t size() const noexcept {
    return string.size();
  }
};

template <Literal... L>
struct ConcatenateLiterals {
  static std::array<char, (L.size() + ...) + 1U> concat() noexcept {
    std::array<char, (L.size() + ...) + 1U> ccLiterals = {};
    std::size_t offset = {};
    const auto lambda = [&](const auto &literal) {
      for (auto c : literal.string) {
        ccLiterals[offset] = c;
        ++offset;
      }
    };
    (lambda(L), ...);
    return ccLiterals;
  }
};

enum class LogSeverity : std::uint8_t {
  LOG_DEBUG,
  LOG_INFO,
  LOG_ERROR,
  LOG_CRITICAL,
  LOG_NOLOG  // No logging.
};

/**
 * @brief
 * Log a message to stderr. Filters messages based on
 * what project-wide logging macros are defined.
 */
inline void logMessage(LogSeverity severity, std::string_view message) {
  static const auto zone = std::chrono::current_zone();
  const auto now = std::chrono::system_clock::now();
  const auto zonedNow = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
  const auto zonedTime = std::chrono::zoned_time(zone, zonedNow);

  const std::string_view prefix = [severity] {
    switch (severity) {
      case LogSeverity::LOG_ERROR:
        return "ERROR";
      case LogSeverity::LOG_INFO:
        return "INFO";
      case LogSeverity::LOG_DEBUG:
        return "DEBUG";
      case LogSeverity::LOG_CRITICAL:
        return "CRITICAL";
      case LogSeverity::LOG_NOLOG:
        return "";
    }
  }();

#if defined(WSR_LOGDEBUG)
  constexpr auto thresh = LogSeverity::LOG_DEBUG;
#elif defined(WSR_LOGERROR)
  constexpr auto thresh = LogSeverity::LOG_ERROR;
#elif defined(WSR_LOGINFO)
  constexpr auto thresh = LogSeverity::LOG_INFO;
#elif defined(WSR_LOGCRITICAL)
  constexpr auto thresh = LogSeverity::LOG_CRITICAL;
#else
  constexpr auto thresh = LogSeverity::LOG_NOLOG;
#endif

  if (severity >= thresh) {
    std::cerr << std::format("[{} - {:%H:%M (%Ss)}] {}", prefix, zonedTime, message) << '\n';
  }
}

inline void windowsRequire(bool condition, std::string_view message) {
  if (!condition) [[unlikely]] {
    throw std::system_error(GetLastError(), std::system_category(), message.data());
  }
}

inline void runtimeRequire(bool condition, std::string_view message) {
  if (!condition) [[unlikely]] {
    throw std::runtime_error(message.data());
  }
}

}  // namespace wsr::utils