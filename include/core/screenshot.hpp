/**
 * screenshot.hpp
 *
 * Declaration for the Screenshot class.
 */

#pragma once

#include "core/pch.hpp"
#include "core/types.hpp"

namespace wsr::detail {

struct GdiData {
  HBITMAP bitmap = {};
  HBITMAP exBitmap = {};
  BITMAPINFOHEADER bitmapInfo = {};
  HDC screenDc = {};
  HDC memoryDc = {};
  int screenX = {};
  int screenY = {};
  int screenOffX = {};
  int screenOffY = {};

  GdiData(int w = 0, int h = 0);
  GdiData(const GdiData &) = delete;
  GdiData(GdiData &&other) noexcept;
  GdiData &operator=(const GdiData &) = delete;
  GdiData &operator=(GdiData &&other) noexcept;
  ~GdiData();
};

};  // namespace wsr::detail

namespace wsr {

class Screenshot {
  mutable detail::GdiData gdi_ = {};
  cv::Rect target_ = {};
  int screenX_ = {};
  int screenY_ = {};
 public:
  Screenshot();

  int screenX() const noexcept;
  int screenY() const noexcept;
  cv::Rect target() const noexcept;

  // Takes screenshots based on a set target.
  // Note that this method will not resize the buffer 
  // if it exceeds the current target size.
  void take(std::vector<Rgb>& buffer) const;

  // Takes a screenshot and returns a buffer.
  std::vector<Rgb> take() const;
  void setTarget(cv::Rect target);
};

}  // namespace wsr