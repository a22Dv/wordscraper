/**
 * reader.hpp
 *
 * Declaration for the Reader class.
 */

#pragma once

#include "core/pch.hpp"

namespace wsr {

class Reader {
    static constexpr std::size_t templateSideLength_ = 32;
    std::vector<cv::Mat> templates_ = {};
  public:
    Reader();

    /**
     * Returns a confidence score and the predicted character
     * found in an image, within the passed bounding box.
     */
    std::pair<float, char> match(cv::Mat image, cv::Rect bbox) const;
};

}  // namespace wsr