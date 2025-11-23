/**
 * recognizer.cpp
 *
 * Implementation for recognizer.hpp
 */

#include "core/recognizer.hpp"
#include "core/pch.hpp"
#include "utils/utilities.hpp"


namespace wsr {

std::optional<cv::Rect> findLevelLetterWheel_(const cv::Mat &screen) {
  std::ignore = screen;
  return {};
}
std::optional<cv::Rect> findMainMenuLevelButton_(const cv::Mat &screen) {
  std::ignore = screen;
  return {};
}

std::optional<cv::Rect> Recognizer::findMainMenu(const cv::Mat &screen) {
  std::ignore = screen;
  return {};
}

std::optional<cv::Rect> Recognizer::findLevel(const cv::Mat &screen) {
  
  std::ignore = screen;
  return {};
}

}  // namespace wsr