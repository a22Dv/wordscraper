/**
 * recognizer.hpp
 *
 * Declaration for the Recognizer class.
 */

#pragma once

#include "core/pch.hpp"

namespace wsr {

class Recognizer {
  std::optional<cv::Rect> findLevelLetterWheel_(const cv::Mat& screen) const;
  std::optional<cv::Rect> findMainMenuLevelButton_(const cv::Mat& screen) const;
 public:
  std::optional<cv::Rect> findMainMenu(const cv::Mat& screen) const;
  std::optional<cv::Rect> findLevel(const cv::Mat& screen) const;
};

}  // namespace wsr