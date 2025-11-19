#include "core/screenshot.hpp"
#include "core/pch.hpp"

using namespace wsr;

int main() {
  Screenshot ssObj = {};
  std::vector<Rgba> ss = ssObj.take();
  cv::Mat ssMat = {ssObj.screenY(), ssObj.screenX(), CV_8UC4, ss.data()};
  cv::imshow("", ssMat);
  cv::waitKey(0);
}