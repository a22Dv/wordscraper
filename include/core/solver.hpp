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

enum class QueryType : std::uint8_t {
  QUERY_SUBSETS,
  QUERY_SUPERSETS,
  QUERY_EQUALITY,
  QUERY_INEQUALITY
};

struct LevelData {
  Matrix<char> layout = {};
  std::vector<std::string_view> words = {};
};

struct DictionaryEntry {
  Signature signature = {};
  std::string_view view = {};
  std::size_t frequency = {};
};

class Database {
  std::string dictionaryData_ = {};
  std::string levelData_ = {};
  std::vector<DictionaryEntry> dictionaryEntries_ = {};

  // Index is the map layout.
  std::unordered_map<std::size_t, LevelData> levelEntriesMap_ = {};

  void sortFields_();
  void parseDictionaryData_();
  void parseLevelData_();

 public:
  Database();

  // Query the entries database for a matching answer key.
  std::optional<LevelData> query(const Matrix<char> &grid, std::string_view letters) const;

  // Query the dictionary for entries that match the criteria given a query type.
  std::vector<DictionaryEntry> query(std::string_view letters, QueryType type) const;
};

}  // namespace wsr::detail

namespace wsr {

class Solver {
  detail::Database database_ = {};

  // Manually solves the grid with the letters based on
  // the current known vocabulary via constraint propagation.
  std::vector<std::string_view> fallbackDictionarySolve_(
      const Matrix<char> &grid, std::string_view letters
  ) const;

  // Solves a level if grid structure is found in the
  // database. Returns an empty vector upon failure.
  std::vector<std::string_view> querySolve_(
      const Matrix<char> &grid, std::string_view letters
  ) const;

 public:

  // Solves a given level and returns the answers. Returns an empty vector upon failure.
  std::vector<std::string_view> solve(const Matrix<char> &grid, std::string_view letters);
};

}  // namespace wsr