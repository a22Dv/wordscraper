#include "core/solver.hpp"

int main() {
  wsr::Solver solver = {};
  auto results = solver.solve({}, "Hello");
  for (auto s : results) {
    std::cout << s << '\n';
  }
}