/**
 * utilities.hpp
 *
 * Project-wide utilities.
 */

#pragma once

#include <concepts>
#include "core/config.hpp"
#include "core/pch.hpp"

#define WSR_EXCEPTMSG(varname) static constexpr const char varname[]
#define WSR_LOGMSG(varname) WSR_EXCEPTMSG(varname)  // Alias for readability.
#define WSR_TO_LITERAL_IMPL_(tokens) #tokens
#define WSR_TO_LITERAL(tokens) \
  WSR_TO_LITERAL_IMPL_(tokens)  // Expand possible macro before #-operator.

#if !defined(WSR_NOPROFILE)
  #define WSR_PROFILE_SCOPE() ZoneScopedN(__func__)
  #define WSR_PROFILE_SCOPEN(name) ZoneScopedN(name)
#else
  #define WSR_PROFILE_SCOPE()
  #define WSR_PROFILE_SCOPEN(name)
#endif

#define WSR_IMGSHOW(image)                                                                   \
  do {                                                                                       \
    cv::imshow(                                                                              \
        wsr::utils::ConcatenateLiterals<__func__, "():", WSR_TO_LITERAL(__LINE__)>::concat() \
            .data(),                                                                         \
        image);                                                                              \
    cv::waitKey(0);                                                                          \
  } while (false)

#if !defined(NDEBUG) && !defined(WSR_NOASSERT)
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
  consteval Literal(const char (&str)[S]) noexcept {
    for (std::size_t i = {}; i < S - 1U; ++i) {
      string[i] = str[i];
    }
  }
  consteval std::size_t size() const noexcept {
    return string.size();
  }
  consteval char operator[](std::size_t i) const {
    return string[i];
  }
};

template <Literal... L>
struct ConcatenateLiterals {
  /**
   * Concatenates compile-time strings together.
   */
  static consteval std::array<char, (L.size() + ...) + 1U> concat() noexcept {
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

/**
 * Uses GetLastError() to get an error code.
 * Throws std::system_error if a condition is not met along with a message.
 */
inline void windowsRequire(bool condition, std::string_view message) {
  if (!condition) [[unlikely]] {
    logMessage(LogSeverity::LOG_CRITICAL, message);
    throw std::system_error(GetLastError(), std::system_category(), message.data());
  }
}

/**
 * Throws a runtime_error exception if said condition
 * isn't met, along with the message.
 */
inline void runtimeRequire(bool condition, std::string_view message) {
  if (!condition) [[unlikely]] {
    logMessage(LogSeverity::LOG_CRITICAL, message);
    throw std::runtime_error(message.data());
  }
}

/**
 * Uses a busy-wait spin loop with _mm_pause().
 * Can be used when wait-time is smaller than the system's tick rate.
 */
template <typename Rep, typename Period>
inline void preciseSleepFor(std::chrono::duration<Rep, Period> duration) {
  if (duration <= duration.zero()) {
    return;
  }
  const auto start = std::chrono::steady_clock::now();
  while ((std::chrono::steady_clock::now() - start) < duration) {
    _mm_pause();
  }
}

/**
 * Checks if a given value is within a specified range.
 * Defaults by considering a value in range if its
 * min <= val <= max
 */
template <const Literal range = "[]", std::totally_ordered T>
bool inRange(T val, T min, T max) {
  bool begin = false;
  bool end = false;
  static_assert(range.size() == 2, "Invalid range specifier size.");
  if constexpr (range[0] == '[') {
    begin = val >= min;
  } else if constexpr (range[0] == '(') {
    begin = val > min;
  } else {
    static_assert(false, "Invalid begin inclusion specifier.");
  }

  if constexpr (range[1] == ']') {
    end = val <= max;
  } else if constexpr (range[1] == ')') {
    end = val < max;
  } else {
    static_assert(false, "Invalid end inclusion specifier.");
  }
  return begin && end;
}

/**
 * Gets the root directory of the current executable.
 */
inline std::filesystem::path getRoot() {
  WSR_EXCEPTMSG(rDirErrMsg) = "Could not retrieve root directory.";
  std::wstring buffer(MAX_PATH, '\0');
  while (true) {
    DWORD ch = GetModuleFileNameW(nullptr, buffer.data(), buffer.size());
    wsr::utils::windowsRequire(ch, WSR_EXCEPTION(rDirErrMsg));
    if (ch == buffer.size()) {
      buffer.resize(buffer.size() * 2U);
    }
    break;
  }
  return std::filesystem::path(buffer).parent_path();
}


}  // namespace wsr::utils