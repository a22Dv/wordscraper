/**
 * utils.cpp
 * Implementation for utils.hpp
 */

#ifdef _WIN64

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#else

#error Requires a Windows(x64) compilation target.

#endif

#include <stdexcept>
#include <system_error>

#include "utils/utils.hpp"  // Master header file.


namespace wsc
{

constexpr void windowsRequire(bool condition, const char *message)
{
  if (!condition)
  {
    throw std::system_error(GetLastError(), std::system_category(), message);
  }
}

constexpr void runtimeRequire(bool condition, const char *message)
{
  if (!condition)
  {
    throw std::runtime_error(message);
  }
}

}  // namespace wsc