/**
 * utils.hpp
 *
 * Project-wide utilities.
 */

#pragma once

#include "core/config.hpp"  // IWYU pragma: export
#include "core/pch.hpp"

#define WSC_STRINGIFY_IMPL_(token) #token
#define WSC_STRINGIFY(token) WSC_STRINGIFY_IMPL_(token)
#define WSC_EXCEPT_FORMAT(message)                                                                \
  wsc::ConcatenateLiterals<                                                                       \
      "An exception has occured at ", __FILE__, " in ", __func__, "():", WSC_STRINGIFY(__LINE__), \
      "\n", message>()()                                                                          \
      .data()

#if !defined(WSC_NOASSERT) && !defined(_NDEBUG)

#define WSC_ASSERT(condition, message)                                                            \
  do                                                                                              \
  {                                                                                               \
    if (!(condition))                                                                             \
    {                                                                                             \
      std::cerr << "\033[2J\033[H"                                                                \
                << wsc::ConcatenateLiterals<                                                      \
                       "Assertion failed: ", #condition, " at ", __FILE__, " in ", __func__,      \
                       "():", WSC_STRINGIFY(__LINE__), "\n", message, "\nStop debugger at line ", \
                       WSC_STRINGIFY(__LINE__) "? (Y/N): ">()()                                   \
                       .data()                                                                    \
                << std::flush;                                                                    \
      char dbgBreak = '\0';                                                                       \
      std::cin >> dbgBreak;                                                                       \
      if ((dbgBreak & ~32) == 'Y')                                                                \
      {                                                                                           \
        __debugbreak();                                                                           \
      }                                                                                           \
      else                                                                                        \
      {                                                                                           \
        std::exit(EXIT_FAILURE);                                                                  \
      }                                                                                           \
    }                                                                                             \
  } while (false)

#else

#define WSC_ASSERT(condition, message)

#endif

#ifndef WSC_NOPROFILE

#define WSC_PROFILESCOPE() ZoneScopedN(__func__)
#define WSC_PROFILESCOPEN(name) ZoneScopedN(name)

#else

#define WSC_PROFILESCOPE()
#define WSC_PROFILESCOPEN(name)

#endif
namespace wsc
{

template <size_t S>
struct Literal
{
  std::array<char, S> data = {};

  constexpr Literal(const char (&string)[S])
  {
    for (std::size_t i = 0ULL; i < S; ++i)
    {
      data[i] = string[i];
    }
  }
  constexpr std::size_t size() const { return S; }
};

template <Literal... Args>
struct ConcatenateLiterals
{
  constexpr ConcatenateLiterals() = default;
  constexpr auto operator()() -> std::array<char, ((Args.size() - 1ULL) + ...) + 1ULL> const
  {
    std::size_t offset = 0ULL;
    std::array<char, ((Args.size() - 1ULL) + ...) + 1ULL> data = {};
    (
        [&](const auto &literal)
        {
          for (std::size_t i = 0ULL; i < literal.size() - 1ULL; ++i)
          {
            data[i + offset] = literal.data[i];
          }
          offset += literal.size() - 1ULL;
        }(Args),
        ...
    );
    return data;
  }
};

/**
 * Calls GetLastError() if condition isn't met, and throws an
 * std::system_error(std::system_category()) along with a message.
 */
constexpr void windowsRequire(bool condition, const char *message);

/**
 * Throws an std::runtime_error along with the message if a condition
 * isn't met.
 */
constexpr void runtimeRequire(bool condition, const char *message);

}  // namespace wsc
