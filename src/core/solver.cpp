/**
 * solver.cpp
 *
 * Implementation for the Solver class.
 */

#include "core/solver.hpp"
#include <memory>
#include "core/pch.hpp"
#include "utils/utilities.hpp"

namespace fs = std::filesystem;

namespace {

/**
 * Hashes a given layout string.
 */
std::size_t hashLayout(std::string_view layout) {
  constexpr uint32_t prime = 37U;
  size_t hash = 0;
  uint32_t powP = prime;
  for (auto ch : layout) {
    if (ch != '0' && ch != '1') {
      ch = '1'; // Handle cases where the board is partially filled.
    }
    hash += uint32_t(ch) * powP;
    powP *= prime;
  }
  hash %= (0xFFFFFFFFULL + 1ULL);
  return hash;
};

/**
 * Returns a list of the space-separated words found in a section of
 * the passed in data.
 */
std::vector<std::string_view> getWords(
    const std::string &data, std::size_t wordsBegin, std::size_t wordsEnd
) {
  WSR_ASSERT(wordsBegin <= wordsEnd);
  constexpr std::size_t averageWordSize = 5;

  std::vector<std::string_view> words = {};
  words.reserve((wordsEnd - wordsBegin) / averageWordSize);

  const auto endIter = data.begin() + wordsEnd;
  auto wordStartIter = data.begin() + wordsBegin;
  for (auto iter = wordStartIter; iter != endIter; ++iter) {
    if (*iter == ' ') {
      words.emplace_back(wordStartIter, iter);
      wordStartIter = iter + 1;
      continue;
    }
  }
  if (wordStartIter != endIter) {
    words.emplace_back(wordStartIter, endIter);
  }
  return words;
}

/**
 * Returns the layout string formatted into a matrix.
 */
wsr::Matrix<char> getLayoutMatrix(std::size_t w, std::size_t h, std::string_view layoutString) {
  WSR_ASSERT(w <= std::numeric_limits<int>::max());
  WSR_ASSERT(h <= std::numeric_limits<int>::max());

  wsr::Matrix<char> matrix(w, h);
  for (int y = 0; std::size_t(y) < h; ++y) {
    for (int x = 0; std::size_t(x) < w; ++x) {
      matrix[{x, y}] = char(layoutString[y * int(w) + x] - '0');
    }
  }
  return matrix;
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

void Database::sortFields_() {
  std::sort(dictionaryEntries_.begin(), dictionaryEntries_.end(), [](const auto &a, const auto &b) {
    const std::size_t sizeA = a.view.size();
    const std::size_t sizeB = b.view.size();
    if (sizeA != sizeB) {
      return sizeA < sizeB;
    }
    return a.frequency > b.frequency;
  });
}

void Database::parseDictionaryData_() {
  WSR_EXCEPTMSG(parseFailErrMsg) = "Failed to parse text file.";
  WSR_LOGMSG(parseDictStart) = "Constructing database dictionary data...";
  WSR_PROFILE_SCOPE();
  constexpr std::size_t expectedLineCount = 333333;
  dictionaryEntries_.reserve(expectedLineCount);

  utils::logMessage(utils::LogSeverity::LOG_INFO, parseDictStart);
  for (std::size_t i = 0; i < dictionaryData_.size();) {
    const std::size_t wordEnd = dictionaryData_.find_first_of('\t', i);
    const std::size_t numEnd = dictionaryData_.find_first_of('\n', wordEnd + 1);
    const char *pNStart = std::to_address(dictionaryData_.begin() + wordEnd + 1);
    const char *pNEnd = std::to_address(dictionaryData_.begin() + numEnd);
    std::size_t frequency = {};
    const auto fchr = std::from_chars(pNStart, pNEnd, frequency);
    utils::runtimeRequire(fchr.ptr == pNEnd, WSR_EXCEPTION(parseFailErrMsg));

    const std::string_view view = {dictionaryData_.begin() + i, dictionaryData_.begin() + wordEnd};
    dictionaryEntries_.emplace_back(Signature(view), view, frequency);
    i = numEnd + 1;
  }
}

void Database::parseLevelData_() {
  WSR_EXCEPTMSG(parseFailErrMsg) = "Failed to parse text file.";
  WSR_LOGMSG(parseLevelStart) = "Constructing database level data...";
  WSR_PROFILE_SCOPE();
  constexpr std::size_t expectedLevelCount = 6000;
  levelEntriesMap_.reserve(expectedLevelCount);
  utils::logMessage(utils::LogSeverity::LOG_INFO, parseLevelStart);

  for (std::size_t i = 0; i < levelData_.size();) {
    const std::size_t widthEnd = levelData_.find_first_of(' ', i);
    const std::size_t heightEnd = levelData_.find_first_of(' ', widthEnd + 1);

    std::size_t width = {};
    std::size_t height = {};

    const char *wStart = std::to_address(levelData_.begin() + i);
    const char *wEnd = std::to_address(levelData_.begin() + widthEnd);
    const char *hStart = std::to_address(levelData_.begin() + widthEnd + 1);
    const char *hEnd = std::to_address(levelData_.begin() + heightEnd);
    const auto wFchr = std::from_chars(wStart, wEnd, width);
    const auto hFchr = std::from_chars(hStart, hEnd, height);

    utils::runtimeRequire(wFchr.ptr == wEnd, WSR_EXCEPTION(parseFailErrMsg));
    utils::runtimeRequire(hFchr.ptr == hEnd, WSR_EXCEPTION(parseFailErrMsg));

    const auto iterLayoutBegin = levelData_.begin() + heightEnd + 1;
    const auto iterLayoutEnd = iterLayoutBegin + width * height;
    const std::string_view layout = {iterLayoutBegin, iterLayoutEnd};

    const std::size_t wordsBegin = heightEnd + 1 + width * height + 1;  // + 1 to skip \t.
    const std::size_t wordsEnd = [this, &wordsBegin] {
      const std::size_t nline = levelData_.find_first_of('\n', wordsBegin);
      return nline != levelData_.npos ? nline : levelData_.size();
    }();
    const wsr::Matrix<char> matrix = getLayoutMatrix(width, height, layout);
    const std::vector<std::string_view> words = getWords(levelData_, wordsBegin, wordsEnd);
    const std::size_t hash = hashLayout(layout);
    levelEntriesMap_[hash] = {matrix, words};
    i = wordsEnd + 1;
  }
}

Database::Database() {
  WSR_EXCEPTMSG(constructFailErrMsg) = "Database construction failed.";
  WSR_LOGMSG(constructStart) = "Constructing database.";
  WSR_PROFILE_SCOPE();

  utils::logMessage(utils::LogSeverity::LOG_INFO, constructStart);
  try {
    const fs::path root = utils::getRoot();
    const fs::path dictionaryFilePath = root / "data" / "words.txt";
    const fs::path levelEntriesFilePath = root / "data" / "data.txt";

    std::ifstream dictionaryStream = {};
    std::ifstream levelEntriesStream = {};

    // ifstream::failbit not set to avoid issues regarding translation.
    levelEntriesStream.exceptions(std::ifstream::badbit);
    dictionaryStream.exceptions(std::ifstream::badbit);

    levelEntriesStream.open(levelEntriesFilePath);
    dictionaryStream.open(dictionaryFilePath);

    const std::size_t dictionaryFileSize = fs::file_size(dictionaryFilePath);
    const std::size_t levelEntriesFileSize = fs::file_size(levelEntriesFilePath);
    dictionaryData_.resize(dictionaryFileSize);  // Zero-initialized.
    levelData_.resize(levelEntriesFileSize);     // Zero-initialized.

    dictionaryStream.read(dictionaryData_.data(), dictionaryFileSize);
    levelEntriesStream.read(levelData_.data(), levelEntriesFileSize);

    // Filesize is greater than true length due to byte->text \r\n translation.
    const std::size_t tEndDictionaryData = std::strlen(dictionaryData_.data());
    const std::size_t tEndLevelData = std::strlen(levelData_.data());
    dictionaryData_.resize(tEndDictionaryData);
    levelData_.resize(tEndLevelData);
    dictionaryData_.shrink_to_fit();
    levelData_.shrink_to_fit();

    parseDictionaryData_();
    parseLevelData_();
    sortFields_();
  } catch (...) {
    utils::logMessage(utils::LogSeverity::LOG_CRITICAL, constructFailErrMsg);
    throw;
  }
}

std::optional<LevelData> Database::query(const Matrix<char> &grid, std::string_view letters) const {
  WSR_PROFILE_SCOPE();
  const std::string_view gridData = {grid.data().data(), grid.sizeX() * grid.sizeY()};
  const std::size_t levelHash = hashLayout(gridData);
  const auto iter = levelEntriesMap_.find(levelHash);
  if (iter == levelEntriesMap_.end()) {
    return std::nullopt;
  }
  const std::vector<std::string_view> &words = iter->second.words;
  const std::string_view longestEntry = *std::max_element( // Wordscapes' longest word uses all the available letters.
      words.begin(), words.end(), [](auto a, auto b) { return a.size() < b.size(); }
  );
  if (Signature(letters) != Signature(longestEntry)) {
    return std::nullopt;
  }
  return iter->second;
}

std::vector<DictionaryEntry> Database::query(std::string_view letters, QueryType type) const {
  WSR_PROFILE_SCOPE();
  const auto comparator = [type](const Signature &a, const Signature &b) {
    switch (type) {
      case QueryType::QUERY_SUBSETS:
        return a >= b;
      case QueryType::QUERY_SUPERSETS:
        return a <= b;
      case QueryType::QUERY_EQUALITY:
        return a == b;
      case QueryType::QUERY_INEQUALITY:
        return a != b;
    }
  };
  std::vector<DictionaryEntry> queryResult = {};
  const auto letterSig = Signature(letters);
  for (const auto &entry : dictionaryEntries_) {
    if (comparator(letterSig, entry.signature)) {
      queryResult.emplace_back(entry);
    }
  }
  return queryResult;
}

}  // namespace wsr::detail

namespace wsr {

std::vector<std::string_view> Solver::fallbackDictionarySolve_(
    const Matrix<char> &grid, std::string_view letters
) const {
  WSR_PROFILE_SCOPE();
  std::ignore = grid;
  /**
   * TODO: Implementation.
   * NOTE:
   * This is a placeholder implementation.
   * The actual dictionary solver will use information from the grid to constrain
   * the possible answers from the query result as much as possible.
   */
  const auto queryResult = database_.query(letters, detail::QueryType::QUERY_SUBSETS);
  std::vector<std::string_view> words = {};
  words.reserve(queryResult.size());
  std::transform(
      queryResult.begin(), queryResult.end(), std::back_inserter(words), [](const auto &a) {
        return a.view;
      }
  );
  return words;
}

std::vector<std::string_view> Solver::querySolve_(
    const Matrix<char> &grid, std::string_view letters
) const {
  WSR_PROFILE_SCOPE();
  const std::optional<detail::LevelData> level = database_.query(grid, letters);
  if (level.has_value()) {
    return level->words;
  }
  return {};
}

std::vector<std::string_view> Solver::solve(const Matrix<char> &grid, std::string_view letters) {
  WSR_LOGMSG(logQueryGridSuccess) = "Query successful. Found matching entry...";
  WSR_LOGMSG(logQueryGridFail) = "Query unsuccessful. Using dictionary fallback...";
  WSR_PROFILE_SCOPE();
  const auto queryResult = querySolve_(grid, letters);
  if (!queryResult.empty()) {
    utils::logMessage(utils::LogSeverity::LOG_INFO, logQueryGridSuccess);
    return queryResult;
  }
  utils::logMessage(utils::LogSeverity::LOG_ERROR, logQueryGridFail);
  return fallbackDictionarySolve_(grid, letters);
}

}  // namespace wsr