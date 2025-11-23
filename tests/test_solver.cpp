#include "core/solver.hpp"

int main() {
  wsr::Solver solver = {};

  constexpr int lx = 9;
  constexpr int ly = 7;
  std::string_view layout = "000111101111100101010101111011100001000001111000001000000001000";
  std::string_view letters = "DSOLI";

  wsr::Matrix<char> matrix(lx, ly);
  
  for (int y = 0; y < ly; ++y) {
    for (int x = 0; x < lx; ++x) {
      const int i = y * lx + x;
      matrix[{x, y}] = layout[i];
    }
  }
  const auto result = solver.solve(matrix, letters);
  for (auto str : result) {
    std::cout << str << '\n';
  }
}