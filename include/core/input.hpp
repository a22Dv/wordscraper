/**
 * input.hpp
 *
 * Declaration for Input class.
 */

#pragma once

#include "core/pch.hpp"

namespace wsr {

/**
 * Handles mouse input requests.
 * This is not a thread-safe class.
 * Access must be locked behind a mutex in
 * multithreaded contexts.
 */
class Input {
  static constexpr int sampleRate_ = 120;
  
  int screenX_ = {};
  int screenY_ = {};
  int cx_ = {};
  int cy_ = {};
  INPUT createMInput_(int x, int y, int flags) const;
  void leftDown_();
  void leftUp_();
  void setCPos_();

 public:
  Input();
  void leftClick();
  void moveMouseTo(int x, int y, std::chrono::milliseconds duration);
  void dragLeftTo(int x, int y, std::chrono::milliseconds duration);
};

}  // namespace wsr