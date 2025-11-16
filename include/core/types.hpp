/**
 * types.hpp
 * Project-wide types.
 */

#pragma once

#include "pch.hpp"

namespace wsc
{
  
struct RGBA
{
  std::uint8_t red = 0U;
  std::uint8_t green = 0U;
  std::uint8_t blue = 0U;
  std::uint8_t alpha = 0U;
};

}  // namespace wsc