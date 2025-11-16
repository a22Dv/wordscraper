/**
 * screenshot.hpp
 *
 * Declaration for the screenshot class.
 */

#pragma once

#include "pch.hpp"
#include "types.hpp"


namespace wsc
{

class Screenshot
{
 private:
  mutable std::condition_variable cycle_ = {};
  mutable std::mutex cycleMutex_ = {};
  mutable std::atomic<bool> acknowledged_ = {};

  std::vector<RGBA> buffer_ = {};
  std::thread thread_ = {};
  std::atomic<std::chrono::milliseconds> interval_ = {};
  std::atomic<DWORD> threadStatus_ = ERROR_SUCCESS;
  std::atomic<bool> terminate_ = {};
  int screenX_ = {};
  int screenY_ = {};

  void threadExec_() noexcept;

 public:
  Screenshot(std::chrono::milliseconds interval = std::chrono::milliseconds(100));
  Screenshot(const Screenshot &) = delete;
  Screenshot &operator=(const Screenshot &) = delete;
  Screenshot(Screenshot &&) = delete;
  Screenshot &operator=(Screenshot &&) = delete;
  ~Screenshot();

  std::chrono::milliseconds interval() const noexcept;
  int screenX() const noexcept;
  int screenY() const noexcept;
  void setInterval(std::chrono::milliseconds interval) noexcept;

  std::vector<RGBA> take() const;
  void take(std::vector<RGBA>& buffer) const;
};

}  // namespace wsc