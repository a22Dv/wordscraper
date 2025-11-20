/**
 * screenshot.cpp
 *
 * Implementation for screenshot.hpp
 */

#include "core/screenshot.hpp"

#include "core/pch.hpp"
#include "utils/utilities.hpp"

namespace wsr::detail {

GdiData::GdiData(int w, int h) {
  WSR_EXCEPTMSG(gsmErrMsg) = "Screen dimensions cannot be retrieved.";
  WSR_EXCEPTMSG(gdcErrMsg) = "Device context cannot be retrieved.";
  WSR_EXCEPTMSG(ccdcErrMsg) = "Device context cannot be created.";
  WSR_EXCEPTMSG(ccbErrMsg) = "Bitmap cannot be created.";
  WSR_EXCEPTMSG(bsoErrMsg) = "Bitmap cannot be retrieved.";
  WSR_ASSERT(w >= 0 && h >= 0);
  WSR_PROFILE_SCOPE();

  try {
    screenX = w ? w : GetSystemMetrics(SM_CXVIRTUALSCREEN);
    wsr::utils::windowsRequire(screenX, WSR_EXCEPTION(gsmErrMsg));
    screenY = h ? h : GetSystemMetrics(SM_CYVIRTUALSCREEN);
    wsr::utils::windowsRequire(screenY, WSR_EXCEPTION(gsmErrMsg));
    screenOffX = GetSystemMetrics(SM_XVIRTUALSCREEN);
    screenOffY = GetSystemMetrics(SM_YVIRTUALSCREEN);
    screenDc = GetDC(nullptr);
    wsr::utils::windowsRequire(screenDc, WSR_EXCEPTION(gdcErrMsg));
    memoryDc = CreateCompatibleDC(screenDc);
    wsr::utils::windowsRequire(memoryDc, WSR_EXCEPTION(ccdcErrMsg));
    bitmap = CreateCompatibleBitmap(screenDc, screenX, screenY);
    wsr::utils::windowsRequire(bitmap, WSR_EXCEPTION(ccbErrMsg));
    exBitmap = static_cast<HBITMAP>(SelectObject(memoryDc, bitmap));
    wsr::utils::windowsRequire(exBitmap, WSR_EXCEPTION(bsoErrMsg));
    bitmapInfo.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.biBitCount = sizeof(wsr::Rgba) * CHAR_BIT;
    bitmapInfo.biCompression = BI_RGB;
    bitmapInfo.biPlanes = 1;
    bitmapInfo.biWidth = screenX;
    bitmapInfo.biHeight = -screenY;
  } catch (const std::system_error &sysE) {
    if (exBitmap) {
      SelectObject(memoryDc, exBitmap);
    }
    if (memoryDc) {
      DeleteDC(memoryDc);
    }
    if (bitmap) {
      DeleteObject(bitmap);
    }
    if (screenDc) {
      ReleaseDC(nullptr, screenDc);
    }
    throw;
  }
}

GdiData::GdiData(GdiData &&other) noexcept {
  std::swap(bitmap, other.bitmap);
  std::swap(exBitmap, other.exBitmap);
  std::swap(bitmapInfo, other.bitmapInfo);
  std::swap(screenDc, other.screenDc);
  std::swap(memoryDc, other.memoryDc);
  screenX = other.screenX;
  screenY = other.screenY;
  screenOffX = other.screenOffX;
  screenOffY = other.screenOffY;
}

GdiData &GdiData::operator=(GdiData &&other) noexcept {
  if (&other == this) {
    return *this;
  }
  std::swap(bitmap, other.bitmap);
  std::swap(exBitmap, other.exBitmap);
  std::swap(bitmapInfo, other.bitmapInfo);
  std::swap(screenDc, other.screenDc);
  std::swap(memoryDc, other.memoryDc);
  screenX = other.screenX;
  screenY = other.screenY;
  screenOffX = other.screenOffX;
  screenOffY = other.screenOffY;
  return *this;
}

GdiData::~GdiData() {
  SelectObject(memoryDc, exBitmap);
  DeleteObject(bitmap);
  DeleteDC(memoryDc);
  ReleaseDC(nullptr, screenDc);
}

}  // namespace wsr::detail

namespace wsr {

Screenshot::Screenshot() {
  target_.width = gdi_.screenX;   // Screen-size auto-detect.
  target_.height = gdi_.screenY;  // Screen-size auto-detect.
  target_.x = 0;
  target_.y = 0;

  screenX_ = target_.width;
  screenY_ = target_.height;
}

int Screenshot::screenX() const noexcept {
  return screenX_;
}

int Screenshot::screenY() const noexcept {
  return screenY_;
}

cv::Rect Screenshot::target() const noexcept {
  return target_;
}

void Screenshot::take(std::vector<Rgba> &buffer) const {
  WSR_EXCEPTMSG(bbErrMsg) = "Bit-transfer encountered a failure.";
  WSR_EXCEPTMSG(gdbErrMsg) = "Bits cannot be copied onto buffer.";
  WSR_ASSERT(target_.x >= 0 && target_.y >= 0);
  WSR_ASSERT(target_.width >= 0 && target_.height >= 0);
  WSR_ASSERT(target_.x + target_.width <= screenX_ && target_.y + target_.height <= screenY_);
  WSR_PROFILE_SCOPE();

  utils::logMessage(utils::LogSeverity::LOG_INFO, "Taking screenshot...");
  if (buffer.size() != std::size_t(target_.area())) {
    buffer.resize(target_.area());
  }
  BOOL rt = TRUE;
  {
    WSR_PROFILE_SCOPEN(BitBlt);
    rt = BitBlt(gdi_.memoryDc, 0, 0, target_.width, target_.height, gdi_.screenDc,
                gdi_.screenOffX + target_.x, gdi_.screenOffY + target_.y, SRCCOPY);
    utils::windowsRequire(rt, WSR_EXCEPTION(bbErrMsg));
  }
  {
    WSR_PROFILE_SCOPEN(GetDIBits);
    rt = GetDIBits(gdi_.memoryDc, gdi_.bitmap, 0, target_.height, buffer.data(),
                   reinterpret_cast<LPBITMAPINFO>(&gdi_.bitmapInfo), DIB_RGB_COLORS);
    utils::windowsRequire(rt, WSR_EXCEPTION(gdbErrMsg));
  }
}

std::vector<Rgba> Screenshot::take() const {
  auto buffer = std::vector<Rgba>();
  take(buffer);
  return buffer;
}

void Screenshot::setTarget(cv::Rect target) {
  WSR_EXCEPTMSG(negativeInputErrMsg) = "Invalid target received.";
  const bool positiveDimensions = target.width > 0 && target.height > 0;
  const bool positiveOffset = target.x >= 0 && target.y >= 0;
  const bool withinBoundsX = target.x + target.width <= screenX_;
  const bool withinBoundsY = target.y + target.height <= screenY_;
  const bool required = positiveDimensions && positiveOffset && withinBoundsX && withinBoundsY;
  utils::runtimeRequire(required, WSR_EXCEPTION(negativeInputErrMsg));

  utils::logMessage(utils::LogSeverity::LOG_INFO, "Setting screenshot target...");
  if (target.width != target_.width || target.height != target_.height) {
    utils::logMessage(utils::LogSeverity::LOG_INFO, "Setting new GDI data...");
    gdi_ = detail::GdiData(target.width, target.height);
  }
  target_ = target;
}

}  // namespace wsr
