#include "core/recognizer.hpp"
#include "core/screenshot.hpp"
#include "core/types.hpp"

int main() {
  cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);
  wsr::Screenshot ss = {};
  std::vector<wsr::Rgb> shot = ss.take();
  const int w = ss.target().width;
  const int h = ss.target().height;
  cv::Mat ssMat = {h, w, CV_8UC3, shot.data()};

  wsr::Recognizer recog = {};
  const auto mm = recog.findMainMenu(ssMat);
  const auto lvl = recog.findLevel(ssMat);

  if (mm) {
    std::cout << "MAIN MENU: " << mm->location << '\n';
  } else {
    std::cout << "MAIN MENU NOT FOUND" << '\n';
  }
  if (lvl) {
    std::cout << "LEVEL: " << lvl->location << '\n';
  } else {
    std::cout << "LEVEL NOT FOUND" << '\n';
  }
  std::this_thread::sleep_for(std::chrono::seconds(1));
}