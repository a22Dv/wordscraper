/**
 * types.hpp
 *
 * Project-wide generic types.
 */

#include "core/pch.hpp"

#pragma once

namespace wsr {

struct Rgba {
  std::uint8_t r = 0U;
  std::uint8_t g = 0U;
  std::uint8_t b = 0U;
  std::uint8_t a = 0U;
};

}  // namespace wsr