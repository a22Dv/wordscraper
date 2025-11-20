#include "core/screenshot.hpp"
#include "core/pch.hpp"
#include "utils/utilities.hpp"

using namespace wsr;

int main() {
  Screenshot ssObj = {};
  std::vector<Rgba> ss = ssObj.take();
  cv::Mat ssMat = {ssObj.screenY(), ssObj.screenX(), CV_8UC4, ss.data()};
  WSR_IMGSHOW(ssMat);
}