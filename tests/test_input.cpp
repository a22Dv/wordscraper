#include "core/input.hpp"
#include "core/pch.hpp"

int main() {
  wsr::Input input = {};
  const auto duration = std::chrono::milliseconds(1000);
  const std::vector<std::pair<int, int>> coords = {
    // Move to:
    std::pair(1920 / 2, 1200 / 2),  // Center
    std::pair(1920 / 2, 0), // Top
    std::pair(1920, 1200 / 2), // Right
    std::pair(0, 1200 / 2), // Left
    std::pair(1920 / 2, 1200), // Bottom
    std::pair(1920 / 2, 1200 / 2), // Center
  };

  for (auto& c : coords) {
    const auto [x, y] = c;
      input.moveMouseTo(x, y, duration);
  }
}