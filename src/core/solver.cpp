/**
 * solver.cpp
 *
 * Implementation for the Solver class.
 */

#include "core/solver.hpp"
#include "core/pch.hpp"
#include "core/types.hpp"
#include "utils/utilities.hpp"

namespace fs = std::filesystem;

namespace {

/**
 * Parses a matrix and returns all the segments that comprise it.
 * Matrix should have '\0' for out-of-bounds cells and a non-zero
 * value if filled in.
 */
std::vector<wsr::AlignedSegment> getSegments(wsr::Matrix<char> &matrix) {
  WSR_ASSERT(matrix.sizeX() <= std::numeric_limits<int>::max());
  WSR_ASSERT(matrix.sizeY() <= std::numeric_limits<int>::max());

  std::vector<wsr::AlignedSegment> segments = {};
  segments.reserve(matrix.sizeX() * matrix.sizeY());

  bool inSegmentX = false;
  wsr::Point segmentStartX = {};
  std::vector<bool> inSegmentY(matrix.sizeX());
  std::vector<wsr::Point> segmentStartY(matrix.sizeX());

  for (int y = 0; std::size_t(y) < matrix.sizeY(); ++y) {
    for (int x = 0; std::size_t(x) < matrix.sizeX(); ++x) {
      const char val = matrix[{x, y}];
      if (inSegmentX && val == '\0') {
        inSegmentX = false;
        segments.emplace_back(segmentStartX, wsr::Point{x, y});
      } else if (!inSegmentX && val != '\0') {
        inSegmentX = true;
        segmentStartX = {x, y};
      }
      if (inSegmentY[x] && val == '\0') {
        inSegmentY[x] = false;
        segments.emplace_back(segmentStartY[x], wsr::Point{x, y});
      } else if (!inSegmentY[x] && val != '\0') {
        inSegmentY[x] = true;
        segmentStartY[x] = {x, y};
      }
    }
    if (inSegmentX) {
      segments.emplace_back(segmentStartX, wsr::Point(matrix.sizeX(), y));
    }
  }
  for (int x = 0; std::size_t(x) < matrix.sizeX(); ++x) {
    if (inSegmentY[x]) {
      segments.emplace_back(segmentStartY[x], wsr::Point{x, int(matrix.sizeY())});
    }
  }
  segments.shrink_to_fit();
  return segments;
}

/**
 * Restricts a given list of entries associated by the indices based on a
 * given board state in a matrix.
 *
 * TODO: Implementation.
 */
std::vector<std::size_t> restrictList(
    const wsr::Solver &solver, const wsr::Matrix<char> grid, const std::vector<std::size_t> &indices
) {
  std::ignore = solver;
  std::ignore = grid;
  std::ignore = indices;
  return indices;  // Do nothing.
}

}  // namespace

namespace wsr::detail {

Signature::Signature(std::string_view string) {
  WSR_EXCEPTMSG(invalidCharErrMsg) = "Non-alphabetic character in input string.";
  for (std::size_t i = 0; i < string.size(); ++i) {
    const char ch = string[i] & ~0x20;
    utils::runtimeRequire(utils::inRange(ch, 'A', 'Z'), WSR_EXCEPTION(invalidCharErrMsg));
    const std::size_t idx = ch - 'A';
    partial_ |= 1U << idx;
    utils::runtimeRequire(full_[idx] < UINT8_MAX, WSR_EXCEPTION(invalidCharErrMsg));
    ++full_[idx];
  }
}

bool Signature::operator==(const Signature &rhs) const noexcept {
  if (partial_ != rhs.partial_) {
    return false;
  }
  return full_ == rhs.full_;
}

bool Signature::operator!=(const Signature &rhs) const noexcept {
  return !(*this == rhs);
}

bool Signature::operator<=(const Signature &rhs) const noexcept {
  if (partial_ & ~rhs.partial_) {
    return false;
  }
  for (std::size_t i = 0; i < alphaCount; ++i) {
    if (full_[i] > rhs.full_[i]) {
      return false;
    }
  }
  return true;
}

bool Signature::operator>=(const Signature &rhs) const noexcept {
  return rhs <= *this;
}

bool Signature::operator>(const Signature &rhs) const noexcept {
  for (std::size_t i = 0; i < alphaCount; ++i) {
    if (full_[i] == rhs.full_[i]) {
      continue;
    } else if (full_[i] > rhs.full_[i]) {
      return false;
    } else {
      return true;
    }
  }
  return false;
}

bool Signature::operator<(const Signature &rhs) const noexcept {
  return rhs > *this;
}

}  // namespace wsr::detail

namespace wsr {

void Solver::sortFields_() {
  std::vector<std::size_t> indices(strings_.size());
  std::iota(indices.begin(), indices.end(), 0);
  std::sort(indices.begin(), indices.end(), [this](auto a, auto b) {
    const std::size_t aSize = strings_[a].size();
    const std::size_t bSize = strings_[b].size();
    if (aSize != bSize) {
      return aSize < bSize;
    }
    return frequencies_[a] > frequencies_[b];
  });

  decltype(frequencies_) freqSorted = {};
  decltype(signatures_) sigSorted = {};
  decltype(strings_) strSorted = {};

  freqSorted.reserve(frequencies_.size());
  sigSorted.reserve(signatures_.size());
  strSorted.reserve(strings_.size());

  for (std::size_t i = 0; i < indices.size(); ++i) {
    const std::size_t idx = indices[i];
    freqSorted.emplace_back(frequencies_[idx]);
    sigSorted.emplace_back(signatures_[idx]);
    strSorted.emplace_back(strings_[idx]);
  }

  std::swap(frequencies_, freqSorted);
  std::swap(signatures_, sigSorted);
  std::swap(strings_, strSorted);
}

void Solver::parseTextFile_(std::size_t fileSize) {
  WSR_EXCEPTMSG(parseFailErrMsg) = "Could not parse text file.";

  for (std::size_t i = 0; i < fileSize;) {
    const std::size_t wordEndIdx = data_.find_first_of('\t', i);
    if (wordEndIdx == data_.npos) {
      break;
    }
    const std::size_t numEndIdx = data_.find_first_of('\n', wordEndIdx);

    const bool inBounds = wordEndIdx < fileSize && numEndIdx < fileSize;
    utils::runtimeRequire(inBounds, WSR_EXCEPTION(parseFailErrMsg));
    
    const std::string_view word = {data_.begin() + i, data_.begin() + wordEndIdx};
    const char *addrNSt = std::to_address(data_.begin() + wordEndIdx + 1);
    const char *addrNEnd = std::to_address(data_.begin() + numEndIdx);
    std::size_t freq = {};
    const auto fch = std::from_chars(addrNSt, addrNEnd, freq);
    utils::runtimeRequire(fch.ptr == addrNEnd, WSR_EXCEPTION(parseFailErrMsg));

    signatures_.emplace_back(word);
    strings_.emplace_back(word);
    frequencies_.emplace_back(freq);

    i = numEndIdx + 1;  // Skip \n.
  }
}

Solver::Solver() {
  WSR_PROFILE_SCOPE();
  constexpr std::size_t expectedLCount = 333333;

  fs::path dataPath = utils::getRoot() / "data" / "words.txt";
  std::ifstream data = {};
  data.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  data.open(dataPath);

  const std::size_t fileSize = fs::file_size(dataPath);
  data_ = std::string(fileSize, '\0');
  data.read(data_.data(), fileSize);

  signatures_.reserve(expectedLCount);
  strings_.reserve(expectedLCount);
  frequencies_.reserve(expectedLCount);

  parseTextFile_(fileSize);
  sortFields_();
}

std::vector<std::string_view> Solver::solve(Matrix<char> grid, std::string_view letters) {
  const std::vector<AlignedSegment> segments = getSegments(grid);
  const auto lettersSignature = detail::Signature(letters);
  std::vector<std::size_t> assocIdx = {};
  assocIdx.reserve(128);

  for (std::size_t i = 0; i < signatures_.size(); ++i) {
    if (signatures_[i] <= lettersSignature) {
      assocIdx.emplace_back(i);
    }
  }

  std::vector<std::size_t> restrictedSubset = restrictList(*this, grid, assocIdx);

  std::vector<std::string_view> strings = {};
  strings.reserve(restrictedSubset.size());
  std::transform(
      restrictedSubset.begin(),
      restrictedSubset.end(),
      std::back_inserter(strings),
      [this](auto i) { return strings_[i]; }
  );
  return strings;
}

}  // namespace wsr