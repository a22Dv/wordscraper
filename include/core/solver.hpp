/**
 * solver.hpp
 *
 * Declaration for the Solver class.
 */

#pragma once

#include "core/pch.hpp"
#include "core/types.hpp"

namespace wsr::detail {

class Signature {
  static constexpr std::size_t alphaCount = 26ULL;
  std::uint32_t partial_ = {};
  std::array<std::uint8_t, alphaCount> full_ = {};

 public:
  Signature() = default;
  Signature(std::string_view string);

  bool operator==(const Signature &rhs) const noexcept;
  bool operator!=(const Signature &rhs) const noexcept;

  // Checks if LHS is a subset of RHS.
  bool operator<=(const Signature &rhs) const noexcept;

  // Checks if LHS is a superset of RHS.
  bool operator>=(const Signature &rhs) const noexcept;

  // Operates on the lexicographic ordering of the sorted
  // input strings during Signature construction.
  bool operator>(const Signature &rhs) const noexcept;

  // Operates on the lexicographic ordering of the sorted
  // input strings during Signature construction.
  bool operator<(const Signature &rhs) const noexcept;
};

}  // namespace wsr::detail

namespace wsr {

class Solver {
  std::string data_ = {};
  std::vector<std::string_view> strings_ = {};
  std::vector<detail::Signature> signatures_ = {};
  std::vector<std::size_t> frequencies_ = {};

  void sortFields_();
  void parseTextFile_(std::size_t fileSize);
 public:
  Solver();
  std::vector<std::string_view> solve(Matrix<char> grid, std::string_view letters);
};

}  // namespace wsr