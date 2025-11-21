/**
 * types.hpp
 *
 * Project-wide generic types.
 */

#include "core/pch.hpp"
#include "utils/utilities.hpp"

#pragma once

namespace wsr {

struct Rgba {
  std::uint8_t r = {};
  std::uint8_t g = {};
  std::uint8_t b = {};
  std::uint8_t a = {};
};

struct Point {
  int x = {};
  int y = {};

  Point &operator+=(Point rhs) noexcept {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }
  Point operator+(Point rhs) const noexcept {
    Point lhs = *this;
    return lhs += rhs;
  }
  Point operator-=(Point rhs) noexcept {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }
  Point operator-(Point rhs) const noexcept {
    Point lhs = *this;
    return lhs -= rhs;
  }
  Point &operator*=(Point rhs) noexcept {
    x *= rhs.x;
    y *= rhs.y;
    return *this;
  }
  Point operator*(Point rhs) const noexcept {
    Point lhs = *this;
    return lhs *= rhs;
  }
  Point operator/=(Point rhs) noexcept {
    x /= rhs.x;
    y /= rhs.y;
    return *this;
  }
  Point operator/(Point rhs) const noexcept {
    Point lhs = *this;
    return lhs /= rhs;
  }
  bool operator==(Point rhs) const noexcept {
    return x == rhs.x && y == rhs.y;
  }
  bool operator!=(Point rhs) const noexcept {
    return !(*this == rhs);
  }
};

class AlignedSegment {
  Point begin_ = {};
  Point end_ = {};
  Point unit_ = {};
  int distance_ = {};

 public:
  AlignedSegment() = default;
  AlignedSegment(Point begin, Point end) : begin_(begin), end_(end) {
    const int dX = end.x - begin.x;
    const int dY = end.y - begin.y;
    const int aDX = std::abs(dX);
    const int aDY = std::abs(dY);
    WSR_ASSERT(dX == 0 || dY == 0 || aDX == aDY);

    unit_.x = (dX > 0) - (dX < 0);
    unit_.y = (dY > 0) - (dY < 0);
    distance_ = aDX + aDY - (aDX * (aDX == aDY)); // Chebyshev distance.
  }
  Point begin() const noexcept {
    return begin_;
  }
  Point end() const noexcept {
    return end_;
  }
  Point unit() const noexcept {
    return unit_;
  }
  Point next(Point c) const noexcept {
    if (c == end_) {
      return end_;
    }
    return c + unit_;
  }
  int distance() const noexcept {
    return distance_;
  }
};

template <typename T>
class Matrix {
  WSR_EXCEPTMSG(oorErrMsg_) = "Index out of range.";
  std::vector<T> data_ = {};
  std::size_t sizeX_ = {};
  std::size_t sizeY_ = {};

 public:
  Matrix(std::size_t x = {}, std::size_t y = {}) : sizeX_(x), sizeY_(y) {
    data_.resize(sizeX_ * sizeY_);
  }
  const std::vector<T> &data() const noexcept {
    return data_;
  }
  std::size_t sizeX() const noexcept {
    return sizeX_;
  }
  std::size_t sizeY() const noexcept {
    return sizeY_;
  }
  T &at(std::size_t x, std::size_t y) {
    if (x >= sizeX_ || y >= sizeY_) {
      throw std::out_of_range(WSR_EXCEPTION(oorErrMsg_));
    }
    return data_[y * sizeX_ + x];
  }
  const T &at(std::size_t x, std::size_t y) const {
    if (x >= sizeX_ || y >= sizeY_) {
      throw std::out_of_range(WSR_EXCEPTION(oorErrMsg_));
    }
    return data_[y * sizeX_ + x];
  }
  T &at(Point pt) {
    WSR_ASSERT(std::size_t(pt.x) >= 0 && std::size_t(pt.y) >= 0);
    WSR_ASSERT(std::size_t(pt.x) < sizeX_ && std::size_t(pt.y) < sizeY_);
    return at(std::size_t(pt.x), std::size_t(pt.y));
  }
  const T &at(Point pt) const {
    WSR_ASSERT(std::size_t(pt.x) >= 0 && std::size_t(pt.y) >= 0);
    WSR_ASSERT(std::size_t(pt.x) < sizeX_ && std::size_t(pt.y) < sizeY_);
    return at(std::size_t(pt.x), std::size_t(pt.y));
  }
  T &operator[](Point pt) {
    WSR_ASSERT(std::size_t(pt.x) >= 0 && std::size_t(pt.y) >= 0);
    WSR_ASSERT(std::size_t(pt.x) < sizeX_ && std::size_t(pt.y) < sizeY_);
    return data_[std::size_t(pt.y) * sizeX_ + std::size_t(pt.x)];
  }
  const T &operator[](Point pt) const {
    WSR_ASSERT(std::size_t(pt.x) >= 0 && std::size_t(pt.y) >= 0);
    WSR_ASSERT(std::size_t(pt.x) < sizeX_ && std::size_t(pt.y) < sizeY_);
    return data_[std::size_t(pt.y) * sizeX_ + std::size_t(pt.x)];
  }
  void reserve(std::size_t capacity) {
    data_.reserve(capacity);
  }
  // Does not preserve original index of existing elements.
  void resize(std::size_t nx, std::size_t ny) {
    data_.resize(nx * ny);
    sizeX_ = nx;
    sizeY_ = ny;
  }
};

}  // namespace wsr