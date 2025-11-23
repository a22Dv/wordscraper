#include "core/recognizer.hpp"
#include "core/screenshot.hpp"
#include "core/types.hpp"

int main() {
  wsr::Screenshot ss = {};
  std::vector<wsr::Rgb> shot = ss.take();
  const int w = ss.target().width;
  const int h = ss.target().height;
  cv::Mat ssMat = {h, w, CV_8UC3, shot.data()};
  WSR_IMGSHOW(ssMat);
}