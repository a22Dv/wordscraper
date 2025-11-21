#include "core/pch.hpp"
#include "core/reader.hpp"
#include "utils/utilities.hpp"


int main() {
  wsr::Reader reader = {};
  cv::Mat image =
      cv::imread("C:/dev/repositories/wordscraper/build/debug/tests/data/templates/L.png");
  cv::cvtColor(image, image, cv::COLOR_RGB2RGBA);
  cv::bitwise_not(image, image);
  reader.match(image, cv::Rect(0, 0, image.rows, image.cols));
}