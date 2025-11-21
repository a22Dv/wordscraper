/**
 * input.cpp
 *
 * Implementation for input methods.
 */

#include "core/input.hpp"
#include "core/pch.hpp"
#include "utils/utilities.hpp"

namespace {

WSR_EXCEPTMSG(sInErrMsg) = "Could not send input.";
WSR_EXCEPTMSG(sysMDimErrMsg) = "Could not retrieve screen dimesions.";
WSR_EXCEPTMSG(clmpErrMsg) = "X/Y outside of bounds. Clamping...";
WSR_EXCEPTMSG(cPosErrMsg) = "Could not retrieve current cursor position.";
WSR_EXCEPTMSG(fPosErrMsg) = "Final SendInput() request did not succeed.";

}  // namespace

namespace wsr {

INPUT Input::createMInput_(int x, int y, int flags) const {
  WSR_ASSERT(x >= 0 && y >= 0);
  WSR_ASSERT(screenX_ > 0 && screenY_ > 0);
  WSR_ASSERT(x <= screenX_ && y <= screenY_);

  constexpr std::uint16_t nmax = std::numeric_limits<std::uint16_t>::max();
  const std::uint16_t nx = std::uint16_t((float(x) / screenX_) * nmax);
  const std::uint16_t ny = std::uint16_t((float(y) / screenY_) * nmax);

  INPUT input = {};
  input.type = INPUT_MOUSE;
  input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | flags;
  input.mi.dx = nx;
  input.mi.dy = ny;
  return input;
}

void Input::leftDown_() {
  setCPos_();
  INPUT input = createMInput_(cx_, cy_, MOUSEEVENTF_LEFTDOWN);
  UINT sRt = SendInput(1, &input, sizeof(INPUT));
  if (!sRt) {
    utils::logMessage(utils::LogSeverity::LOG_ERROR, sInErrMsg);
  }
}

void Input::leftUp_() {
  setCPos_();
  INPUT input = createMInput_(cx_, cy_, MOUSEEVENTF_LEFTUP);
  UINT sRt = SendInput(1, &input, sizeof(INPUT));
  if (!sRt) {
    utils::logMessage(utils::LogSeverity::LOG_ERROR, sInErrMsg);
  }
}

void Input::setCPos_() {
  POINT cPos = {};
  BOOL rt = GetCursorPos(&cPos);
  utils::windowsRequire(rt, WSR_EXCEPTION(cPosErrMsg));
  cx_ = cPos.x;
  cy_ = cPos.y;
}

Input::Input() {
  setCPos_();
  screenX_ = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  utils::windowsRequire(screenX_, WSR_EXCEPTION(sysMDimErrMsg));
  screenY_ = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  utils::windowsRequire(screenY_, WSR_EXCEPTION(sysMDimErrMsg));
}

void Input::leftClick() {
  leftDown_();
  leftUp_();
}

void Input::moveMouseTo(int x, int y, std::chrono::milliseconds duration) {
  setCPos_();
  WSR_ASSERT(duration >= std::chrono::milliseconds(0));
  if (x < 0 || y < 0 || x > screenX_ || y > screenY_) [[unlikely]] {
    utils::logMessage(utils::LogSeverity::LOG_ERROR, clmpErrMsg);
    x = std::clamp(x, 0, screenX_);
    y = std::clamp(y, 0, screenY_);
  }
  const int segments = sampleRate_ * (float(duration.count()) / 1000);
  if (duration == std::chrono::milliseconds(0) || segments == 0) {
    INPUT moveInput = createMInput_(x, y, MOUSEEVENTF_MOVE);
    UINT sRt = SendInput(1, &moveInput, sizeof(INPUT));
    utils::runtimeRequire(sRt, WSR_EXCEPTION(sInErrMsg));
    setCPos_();
    return;
  }
  const float dXPerSegment = float(x - cx_) / segments;
  const float dYPerSegment = float(y - cy_) / segments;
  const auto durSeg = std::chrono::duration_cast<std::chrono::nanoseconds>(duration) / segments;
  bool lastSucceeded = true;
  for (int i = 1; i <= segments; ++i) {
    // createMInput_() recieves coordinates relative to current screen size
    // and translates them by itself to [0-UINT16_MAX].
    INPUT moveInput = createMInput_(int(cx_ + i * dXPerSegment), int(cy_ + i * dYPerSegment), MOUSEEVENTF_MOVE);
    UINT sRt = SendInput(1, &moveInput, sizeof(INPUT));
    if (!sRt) [[unlikely]] {
      utils::logMessage(utils::LogSeverity::LOG_ERROR, sInErrMsg);
    }
    if (i == segments && !sRt) [[unlikely]] {
      lastSucceeded = false;
    }
    utils::preciseSleepFor(durSeg);
  }
  utils::windowsRequire(lastSucceeded, WSR_EXCEPTION(fPosErrMsg));
  setCPos_();
}

void Input::dragLeftTo(int x, int y, std::chrono::milliseconds duration) {
  leftDown_();
  moveMouseTo(x, y, duration);
  leftUp_();
}

}  // namespace wsr