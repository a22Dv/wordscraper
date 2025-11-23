/**
 * recognizer.cpp
 *
 * Implementation for recognizer.hpp
 */

#include "core/recognizer.hpp"
#include "core/pch.hpp"
#include "core/reader.hpp"
#include "core/types.hpp"
#include "utils/utilities.hpp"

namespace {

std::pair<float, std::string> readWord(
    const wsr::Reader &reader, const cv::Mat &roi, float confLimit
) {
  WSR_PROFILE_SCOPE();
  WSR_ASSERT(roi.type() == CV_8UC3);
  constexpr int binSize = 5;

  cv::Mat thresh = {};
  cv::cvtColor(roi, thresh, cv::COLOR_RGB2GRAY);
  cv::threshold(thresh, thresh, 128, UINT8_MAX, cv::THRESH_OTSU);
  std::vector<std::vector<cv::Point>> contours = {};
  cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

  std::vector<cv::Rect> bboxes = {};
  bboxes.reserve(contours.size());
  std::transform(contours.begin(), contours.end(), std::back_inserter(bboxes), [](const auto &a) {
    return cv::boundingRect(a);
  });
  std::sort(bboxes.begin(), bboxes.end(), [](const auto &a, const auto &b) {
    const int aYBin = a.y / binSize;
    const int bYBin = b.y / binSize;
    if (aYBin == bYBin) {
      return a.x < b.x;
    }
    return aYBin < bYBin;
  });

  std::string str = {};
  int cbbox = 0;
  float strConf = {};
  for (const auto &bbox : bboxes) {
    const auto [conf, ch] = reader.match(thresh, bbox);
    if (conf <= confLimit) {
      continue;
    }
    strConf += conf;
    str.push_back(ch);
    ++cbbox;
  }
  strConf /= cbbox;
  return {strConf, str};
}

bool isPossibleWheel(const cv::Mat &roi) {
  WSR_ASSERT(roi.type() == CV_8UC1);
  WSR_PROFILE_SCOPE();
  constexpr int childrenCountLowerBound = 3;
  constexpr int childrenCountUpperBound = 36;
  const int roiArea = roi.size().area();
  cv::Mat roiNot = {};
  roiNot = ~roi;  // Focus on letters.

  cv::Mat mask = {roiNot.rows, roiNot.cols, CV_8UC1, cv::Scalar(0)};
  cv::circle(mask, cv::Point(roi.cols / 2, roi.rows / 2), roi.cols / 2 - 2, cv::Scalar(255), -1);
  roiNot &= mask;

  int childrenCount = 0;
  std::vector<std::vector<cv::Point>> contours = {};

  cv::findContours(roiNot, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
  for (const auto &contour : contours) {
    if (cv::boundingRect(contour).size().area() < roiArea * 0.01) {
      continue;
    }
    ++childrenCount;
  }
  return wsr::utils::inRange(childrenCount, childrenCountLowerBound, childrenCountUpperBound);
}

std::optional<cv::Rect> findWheel(const cv::Mat &roi) {
  WSR_ASSERT(roi.type() == CV_8UC1);
  WSR_PROFILE_SCOPE();
  constexpr double expectedAspectRatio = 1.0;
  constexpr double aspectRatioTolerance = 0.01;
  constexpr double areaTolerance = 0.01;
  const int roiArea = roi.size().area();
  const int noiseThreshArea = roiArea * 0.0005;

  std::vector<std::vector<cv::Point>> contours = {};
  cv::findContours(roi, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

  cv::Rect circle = {};
  for (const auto &contour : contours) {
    const cv::Rect bbox = cv::boundingRect(contour);
    const cv::Size bboxSize = bbox.size();
    if (bboxSize.area() < noiseThreshArea) {
      continue;
    }
    const bool aspectRatioInRange = wsr::utils::inRange(
        bboxSize.aspectRatio(),
        expectedAspectRatio * (1.0 - aspectRatioTolerance),
        expectedAspectRatio * (1.0 + aspectRatioTolerance)
    );
    if (!aspectRatioInRange) {
      continue;
    }
    const double area = cv::contourArea(contour);
    const double r = bboxSize.width / 2.0;
    const double expectedArea = std::numbers::pi * std::pow(r, 2);
    const bool areaInRange = wsr::utils::inRange(
        area, expectedArea * (1.0 - areaTolerance), expectedArea * (1.0 + areaTolerance)
    );
    if (!areaInRange) {
      continue;
    }
    if (!isPossibleWheel(roi(bbox))) {
      continue;
    }
    circle = circle.area() < bbox.area() ? bbox : circle;
  }
  if (circle.empty()) {
    return std::nullopt;
  }
  return circle;
}

std::vector<std::pair<char, cv::Rect>> findWheelLetters(
    const wsr::Reader &reader, const cv::Mat &wheel, bool onWhite
) {
  WSR_ASSERT(wheel.type() == CV_8UC3);
  WSR_PROFILE_SCOPE();
  const int wheelArea = std::numbers::pi * std::pow((wheel.cols / 2), 2);
  const int noiseThreshArea = wheelArea * 0.01;
  constexpr std::size_t minLetters = 3ULL;
  constexpr std::size_t maxLetters = 8ULL;

  cv::Mat letterWheel = {};
  cv::cvtColor(wheel, letterWheel, cv::COLOR_RGB2GRAY);

  if (onWhite) {  // Black letters on white wheel.
    cv::inRange(letterWheel, 0, 0, letterWheel);
  } else {        // White letters on black wheel.
    cv::inRange(letterWheel, UINT8_MAX, UINT8_MAX, letterWheel);
  }

  std::vector<std::vector<cv::Point>> contours = {};
  cv::findContours(letterWheel, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

  std::vector<std::pair<char, cv::Rect>> letters = {};
  for (const auto &contour : contours) {
    const cv::Rect bbox = cv::boundingRect(contour);
    const cv::Size bboxSize = bbox.size();

    if (bboxSize.area() < noiseThreshArea) {
      continue;
    }
    const auto [conf, ch] = reader.match(letterWheel, bbox);
    if (conf < 0.8) {
      continue;
    }
    letters.emplace_back(std::pair<char, cv::Rect>(ch, bbox));
  }
  if (!wsr::utils::inRange(letters.size(), minLetters, maxLetters)) {
    return {};
  }
  return letters;
}

cv::Rect getGridBounds(const std::vector<cv::Rect> &bboxes) {
  int minX = INT_MAX;
  int minY = INT_MAX;
  int maxX = 0;
  int maxY = 0;

  for (const auto &bbox : bboxes) {
    minX = std::min(minX, bbox.x);
    minY = std::min(minY, bbox.y);
    maxX = std::max(maxX, bbox.x + bbox.width);
    maxY = std::max(maxY, bbox.y + bbox.height);
  }
  return {minX, minY, maxX - minX, maxY - minY};
}

std::optional<wsr::Matrix<char>> getMatrix(
    const wsr::Reader &reader, const cv::Mat &roi, std::vector<cv::Rect> bboxes, bool onWhite
) {
  WSR_ASSERT(roi.type() == CV_8UC3);
  const cv::Rect gridBounds = getGridBounds(bboxes);
  cv::Mat roiMask = {roi.rows, roi.cols, CV_8UC1, cv::Scalar(0)};

  int avgWidth = 0;
  int avgHeight = 0;
  for (const auto &bbox : bboxes) {
    avgWidth += bbox.width;
    avgHeight += bbox.height;
    cv::rectangle(roiMask, bbox, cv::Scalar(255), -1);
  }
  avgWidth /= bboxes.size();
  avgHeight /= bboxes.size();
  const int padding = avgWidth * 0.05;

  cv::Mat roiColor = roi(gridBounds);
  roiMask = roiMask(gridBounds);  // Relative coordinates.

  const cv::Point centroid = {avgWidth / 2, avgHeight / 2};

  int cx = centroid.x;
  int cy = centroid.y;
  const int boundaryX = roiMask.cols;
  const int boundaryY = roiMask.rows;

  int gx = 0;
  int gy = 0;
  std::vector<char> data = {};

  while (cy < boundaryY) {
    int nx = 0;
    while (cx < boundaryX) {
      if (roiMask.at<std::uint8_t>(cy, cx) != 0) {
        cv::Rect safebox = cv::Rect(cx - centroid.x, cy - centroid.y, avgWidth, avgHeight);
        safebox.x += avgWidth * 0.05;
        safebox.y += avgHeight * 0.05;
        safebox.height *= 0.95;
        safebox.width *= 0.95;

        cv::Mat letter = roiColor(safebox);
        cv::Mat letterGray = {};
        cv::cvtColor(letter, letterGray, cv::COLOR_RGB2GRAY);

        cv::Scalar mean = {};
        cv::Scalar stddev = {};
        cv::meanStdDev(letterGray, mean, stddev);
        const double meanTMin = onWhite * UINT8_MAX * 0.99;
        const double meanTMax = double(1 + onWhite * UINT8_MAX);
        const bool meanInRange = wsr::utils::inRange(mean[0], meanTMin, meanTMax);
        const bool stddevInRange = wsr::utils::inRange(stddev[0], 0.0, 20.0);

        if (!meanInRange && !stddevInRange) {  // Is not a flat tile.
          const auto [conf, str] = readWord(reader, letter, 0.5f);
          if (!str.empty()) {
            data.push_back(str[0]);
          }
        } else {
          data.push_back('1');
        }
      } else {
        data.push_back('0');
      }
      ++nx;
      cx += avgWidth + padding;
    }
    if (gx && gx != nx) {
      return std::nullopt;
    }
    gx = nx;
    ++gy;
    cx = centroid.x;
    cy += avgHeight + padding;
  }

  std::string logString = {
    std::format("Grid Layout: {}", std::string_view(data.data(), data.size()))
  };
  wsr::utils::logMessage(wsr::utils::LogSeverity::LOG_DEBUG, logString);
  auto grid = wsr::Matrix<char>(gx, gy);
  for (int y = 0; y < gy; ++y) {
    for (int x = 0; x < gx; ++x) {
      grid[{x, y}] = data[y * gx + x];
    }
  }
  return grid;
}

std::optional<wsr::Matrix<char>> findMatrix(
    const wsr::Reader &reader, const cv::Mat &roi, bool onWhite
) {
  std::ignore = onWhite;
  WSR_ASSERT(roi.type() == CV_8UC3);

  const int roiArea = roi.size().area();
  const double noiseThreshArea = roiArea * 0.01;
  const double expectedAspectRatio = 1.0;
  const double aspectRatioTolerance = 0.03;

  cv::Mat blur = {};
  cv::GaussianBlur(roi, blur, cv::Size(3, 3), 0);
  cv::Mat canny = {};
  cv::Canny(blur, canny, 50, 150);

  std::vector<std::vector<cv::Point>> contours = {};
  std::vector<cv::Rect> bboxes = {};
  cv::findContours(canny, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
  for (const auto &contour : contours) {
    cv::Rect bbox = cv::boundingRect(contour);
    const cv::Size bboxSize = bbox.size();
    if (bboxSize.area() < noiseThreshArea) {
      continue;
    }
    const bool aspectRatioInRange = wsr::utils::inRange(
        bboxSize.aspectRatio(),
        expectedAspectRatio * (1.0 - aspectRatioTolerance),
        expectedAspectRatio * (1.0 + aspectRatioTolerance)
    );
    if (!aspectRatioInRange) {
      continue;
    }
    cv::rectangle(blur, bbox, cv::Scalar(0, 255, 0));
    bboxes.emplace_back(bbox);
  }
  const std::optional<wsr::Matrix<char>> matrix = getMatrix(reader, roi, bboxes, onWhite);
  if (!matrix) {
    return std::nullopt;
  }
  return *matrix;
}

}  // namespace

namespace wsr {

std::pair<std::optional<cv::Rect>, bool> Recognizer::findLevelLetterWheel_(const cv::Mat &screen) {
  WSR_ASSERT(screen.type() == CV_8UC3);
  WSR_PROFILE_SCOPE();

  cv::Mat threshold = {};
  cv::cvtColor(screen, threshold, cv::COLOR_RGB2GRAY);
  cv::inRange(threshold, 0, 0, threshold);  // Finds black letter wheels.

  bool onWhite = false;
  std::optional<cv::Rect> wheel = findWheel(threshold);
  if (!wheel) {
    // Finds white letter wheels.
    cv::inRange(threshold, UINT8_MAX, UINT8_MAX, threshold);
    wheel = findWheel(threshold);
    onWhite = true;
  }
  if (!wheel) {
    return {std::nullopt, false};
  }
  return {wheel, onWhite};
}

std::optional<cv::Rect> Recognizer::findMainMenuLevelButton_(const cv::Mat &screen) {
  WSR_ASSERT(screen.type() == CV_8UC3);
  WSR_PROFILE_SCOPE();

  const int cvArea = screen.size().area();
  const int noiseThreshArea = cvArea * 0.0005;
  const double expectedAspectRatio = 4.0;
  const double aspectRatioTolerance = 0.05;

  cv::Mat canny = {};
  cv::Canny(screen, canny, 150, 200);

  std::vector<std::vector<cv::Point>> contours = {};
  cv::findContours(canny, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

  cv::Rect bboxButton = {};
  for (const auto &contour : contours) {
    const cv::Rect bbox = cv::boundingRect(contour);
    const auto bboxSize = bbox.size();
    if (bboxSize.area() < noiseThreshArea) {
      continue;
    }
    const bool aspectRatioInRange = utils::inRange(
        bboxSize.aspectRatio(),
        expectedAspectRatio * (1.0 - aspectRatioTolerance),
        expectedAspectRatio * (1.0 + aspectRatioTolerance)
    );
    if (!aspectRatioInRange) {
      continue;
    }
    const auto [conf, word] = readWord(reader_, screen(bbox), 0.3);
    if (conf < 0.5) {
      continue;
    }
    if (word.find("LEVEL") == word.npos) {
      continue;
    }
    bboxButton = bbox;
  }
  if (!bboxButton.empty()) {
    return bboxButton;
  }
  return std::nullopt;
}

std::optional<MainMenu> Recognizer::findMainMenu(const cv::Mat &screen) {
  WSR_ASSERT(screen.type() == CV_8UC3);
  WSR_PROFILE_SCOPE();

  const std::optional<cv::Rect> cvButton = findMainMenuLevelButton_(screen);
  if (!cvButton.has_value()) {
    return std::nullopt;
  }

  cv::Rect mmLocation = {};
  mmLocation.x = cvButton->x - cvButton->width * 0.65;
  mmLocation.y = cvButton->y - cvButton->width * 2.85;
  mmLocation.height = cvButton->height * 17;
  mmLocation.width = cvButton->width * 2.32;

  const bool xValid = utils::inRange(mmLocation.x, 0, screen.cols);
  const bool yValid = utils::inRange(mmLocation.y, 0, screen.rows);
  const bool wValid = utils::inRange(mmLocation.x + mmLocation.width, 0, screen.cols);
  const bool hValid = utils::inRange(mmLocation.y + mmLocation.height, 0, screen.cols);
  if (!xValid || !yValid || !wValid || !hValid) {
    return std::nullopt;
  }

  return MainMenu{*cvButton, mmLocation};
}

std::optional<Level> Recognizer::findLevel(const cv::Mat &screen) {
  WSR_LOGMSG(noWheel) = "Could not find letter wheel...";
  WSR_LOGMSG(noMatrix) = "Could not find letter grid...";
  WSR_LOGMSG(noLetters) = "Could not find letters in letter wheel...";
  WSR_ASSERT(screen.type() == CV_8UC3);
  WSR_PROFILE_SCOPE();

  const auto [wheelOpt, onWhite] = findLevelLetterWheel_(screen);
  if (!wheelOpt) {
    utils::logMessage(utils::LogSeverity::LOG_INFO, noWheel);
    return std::nullopt;
  }
  const std::vector<std::pair<char, cv::Rect>> letters =
      findWheelLetters(reader_, screen(*wheelOpt), onWhite);
  if (letters.empty()) {
    utils::logMessage(utils::LogSeverity::LOG_INFO, noLetters);
    return std::nullopt;
  }
  cv::Rect wheel = *wheelOpt;
  cv::Rect levelLocation = {};
  levelLocation.x = wheel.x - wheel.width * 0.3;
  levelLocation.y = wheel.y - wheel.width * 1.55;
  levelLocation.width = wheel.width * 1.6;
  levelLocation.height = wheel.height * 2.85;

  std::vector<char> lettersFound = {};
  std::vector<cv::Rect> locationsFound = {};

  lettersFound.reserve(letters.size());
  locationsFound.reserve(letters.size());
  auto ltrIter = std::back_inserter(lettersFound);
  auto locIter = std::back_inserter(locationsFound);
  std::transform(letters.begin(), letters.end(), ltrIter, [](const auto &a) { return a.first; });
  std::transform(letters.begin(), letters.end(), locIter, [](const auto &a) { return a.second; });

  cv::Rect posGridLoc = {};
  posGridLoc.x = levelLocation.x;
  posGridLoc.y = levelLocation.y + wheel.width * 0.18;
  posGridLoc.width = levelLocation.width;
  posGridLoc.height = wheel.y - posGridLoc.y;

  const std::optional<Matrix<char>> gridOpt = findMatrix(reader_, screen(posGridLoc), onWhite);
  if (!gridOpt) {
    utils::logMessage(utils::LogSeverity::LOG_INFO, noMatrix);
    return std::nullopt;
  }
  return Level(levelLocation, wheel, *gridOpt, locationsFound, lettersFound);
}

}  // namespace wsr