/**
 * reader.cpp
 *
 * Implementation for the Reader class.
 */

#include "core/reader.hpp"
#include "core/pch.hpp"
#include "utils/utilities.hpp"

namespace fs = std::filesystem;

namespace {

constexpr std::size_t alphaCount = 26ULL;

/**
 * A helper function that checks if a file entry
 * meets certain criteria to loading.
 */
bool isEntryQualified(const fs::directory_entry &entry) {
  WSR_LOGMSG(ignoreExtMsg) = "Ignored file. Non-matching extension.";
  WSR_LOGMSG(ignoreLenMsg) = "Ignored file. Length > 1.";
  if (!entry.is_regular_file()) {
    return false;
  }
  const fs::path pathToEntry = entry.path();
  const fs::path filename = pathToEntry.filename().replace_extension("");
  const fs::path extension = pathToEntry.extension();
  if (extension != ".png") {
    wsr::utils::logMessage(wsr::utils::LogSeverity::LOG_INFO, ignoreExtMsg);
    return false;
  }
  const std::string filenameStr = filename.string();
  if (filenameStr.size() != 1) {
    wsr::utils::logMessage(wsr::utils::LogSeverity::LOG_INFO, ignoreLenMsg);
    return false;
  }
  return wsr::utils::inRange(filenameStr[0], 'A', 'Z');
}

/**
 * Matches a template with a given template and its inverse, and returns the
 * largest confidence score of either inverse or not.
 */
float matchTemplateWInv(const cv::Mat &roi, const cv::Mat &tmplt) {
  WSR_ASSERT(roi.rows == tmplt.rows && roi.cols == tmplt.cols);
  WSR_ASSERT(roi.type() == tmplt.type() && roi.type() == CV_8UC1);

  int accuDiff = 0;
  int accuInvDiff = 0;
  for (int y = {}; y < roi.rows; ++y) {
    for (int x = {}; x < roi.cols; ++x) {
      const std::size_t idx = std::size_t(y * roi.cols + x);
      accuDiff += std::abs(roi.data[idx] - tmplt.data[idx]);
      accuInvDiff += std::abs(roi.data[idx] - (UINT8_MAX - tmplt.data[idx]));
    }
  }
  const int diff = std::min(accuDiff, accuInvDiff);
  float conf = float(diff) / (roi.rows * roi.cols * UINT8_MAX);  // Normalize.
  return ((1.0f - std::min(conf, 1.0f - conf)) - 0.5f) / 0.5f;   // Rescale.
}

/**
 * Sorts the given templates alphabetically based on their given name.
 */
void sortTemplates(const std::vector<char> &names, std::vector<cv::Mat> &templates) {
  WSR_ASSERT(names.size() == templates.size() && templates.size() == alphaCount);
  std::vector<std::size_t> idx(alphaCount);
  std::iota(idx.begin(), idx.end(), 0ULL);
  std::sort(idx.begin(), idx.end(), [&names](auto a, auto b) { return names[a] < names[b]; });
  std::vector<cv::Mat> sortedTemplates = {};
  sortedTemplates.reserve(alphaCount);
  for (auto i : idx) {
    sortedTemplates.push_back(templates[i]);
  }
  std::swap(templates, sortedTemplates);
}

}  // namespace

namespace wsr {

Reader::Reader() {
  WSR_EXCEPTMSG(tmpDecodeErrMsg) = "Could not decode file to a valid image.";
  WSR_EXCEPTMSG(tmpMissingErrMsg) = "Incomplete template count.";
  WSR_EXCEPTMSG(tmpInvalidErrMsg) = "Invalid template dimensions.";

  const fs::path dataPath = utils::getRoot() / "data" / "templates";
  std::vector<char> names = {};

  templates_.reserve(alphaCount);
  names.reserve(alphaCount);
  for (const auto &entry : fs::directory_iterator(dataPath)) {
    if (!isEntryQualified(entry)) {
      continue;
    }
    const auto entryPath = entry.path();
    std::ifstream dataStream = {};
    dataStream.exceptions(std::ifstream::badbit | std::ifstream::failbit);
    dataStream.open(entryPath, std::ifstream::binary);

    const std::size_t fileSize = fs::file_size(entryPath);
    std::vector<char> data(fileSize);
    dataStream.read(data.data(), data.size());

    cv::Mat templateImg = cv::imdecode(data, cv::IMREAD_GRAYSCALE);
    utils::runtimeRequire(!templateImg.empty(), WSR_EXCEPTION(tmpDecodeErrMsg));

    const bool matchedRows = templateImg.rows == templateSideLength_;
    const bool matchedCols = templateImg.cols == templateSideLength_;
    utils::runtimeRequire(matchedRows && matchedCols, WSR_EXCEPTION(tmpInvalidErrMsg));

    std::string entryPathStr = entryPath.filename().string();
    templates_.push_back(std::move(templateImg));
    names.push_back(entryPathStr[0]);
    utils::logMessage(utils::LogSeverity::LOG_INFO, std::format("Loaded file: {}", entryPathStr));
  }
  utils::runtimeRequire(templates_.size() == alphaCount, WSR_EXCEPTION(tmpMissingErrMsg));
  sortTemplates(names, templates_);
}

std::pair<float, char> Reader::match(cv::Mat image, cv::Rect bbox) const {
  WSR_ASSERT(bbox.x >= 0 && bbox.y >= 0);
  WSR_ASSERT(image.type() == CV_8UC4);
  WSR_ASSERT(bbox.x + bbox.width <= image.cols && bbox.y + bbox.height <= image.rows);
  cv::Mat roi = image(bbox).clone();
  cv::cvtColor(roi, roi, cv::COLOR_RGBA2GRAY);
  cv::resize(roi, roi, cv::Size(templateSideLength_, templateSideLength_));

  char maxCh = 'A';
  float maxConfidence = 0.0f;
  for (char c = 'A'; c <= 'Z'; ++c) {
    const float conf = matchTemplateWInv(roi, templates_[c - 'A']);
    if (maxConfidence < conf) {
      maxConfidence = conf;
      maxCh = c;
    }
  }
  utils::logMessage(
      utils::LogSeverity::LOG_DEBUG, std::format("'{}': {:.2f}%", maxCh, maxConfidence * 100.0f)
  );
  return {maxConfidence, maxCh};
}

}  // namespace wsr