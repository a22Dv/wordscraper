#include "core/pch.hpp"
#include "core/screenshot.hpp"
#include "utils/utilities.hpp"


using namespace wsr;

int main() {
  Screenshot ssObj = {};
  std::vector<Rgb> ss = ssObj.take();
  cv::Mat ssMat = {ssObj.screenY(), ssObj.screenX(), CV_8UC3, ss.data()};
  WSR_IMGSHOW(ssMat);
}