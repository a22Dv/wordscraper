/**
 * input.hpp
 *
 * Declaration for input methods.
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
  int screenX_ = {};
  int screenY_ = {};
  int cx_ = {};
  int cy_ = {};
  int sampleRate_ = 1000;
  INPUT createMInput_(int x, int y, int flags);
  void leftDown_();
  void leftUp_();

 public:
  Input();
  void leftClick();
  void moveMouseTo(int x, int y, std::chrono::milliseconds duration);
  void dragLeftTo(int x, int y, std::chrono::milliseconds duration);
};

}  // namespace wsr