/**
 * recognizer.hpp
 *
 * Declaration for the Recognizer class.
 */

#pragma once

#include "core/pch.hpp"

namespace wsr {

class Recognizer {

  static std::optional<cv::Rect> findLevelLetterWheel_(const cv::Mat &screen);
  static std::optional<cv::Rect> findMainMenuLevelButton_(const cv::Mat &screen);
  
 public:
  static std::optional<cv::Rect> findMainMenu(const cv::Mat &screen);
  static std::optional<cv::Rect> findLevel(const cv::Mat &screen);
};

}  // namespace wsr