/**
 * recognizer.hpp
 *
 * Declaration for the Recognizer class.
 */

#pragma once

#include "core/pch.hpp"
#include "core/reader.hpp"
#include "core/types.hpp"

namespace wsr {

class Recognizer {
  const Reader reader_ = {};
  std::pair<std::optional<cv::Rect>, bool> findLevelLetterWheel_(const cv::Mat &screen);
  std::optional<cv::Rect> findMainMenuLevelButton_(const cv::Mat &screen);
 public:
  std::optional<MainMenu> findMainMenu(const cv::Mat &screen);
  std::optional<Level> findLevel(const cv::Mat &screen);
};

}  // namespace wsr