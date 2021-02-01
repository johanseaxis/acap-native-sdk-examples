#pragma once

#include <tuple>
#include <vector>

#include "misc.hh"
#include "opencv2/opencv.hpp"

/**
 * The image format of the video stream is set by a fourcc code. Read more at
 * https://www.fourcc.org/. For reference, the YUYV code is 0x56595559.
 *
 * Following macro is taken from libyuv.
 */
#define FOURCC(a, b, c, d)                                                     \
    ((static_cast<uint32_t>(a)) | (static_cast<uint32_t>(b) << 8) |            \
     (static_cast<uint32_t>(c) << 16) | /* NOLINT */                           \
     (static_cast<uint32_t>(d) << 24))  /* NOLINT */

std::tuple<int64_t, int64_t> getVideoDims(const cv::VideoCapture& cap);

/**
 * @brief Set video stream dimensions.
 *
 * If opencv uses Video4Linux as the backend, one can get the available formats
 * with "v4l2-ctl -d0 --list-formats-ext".
 */
void setVideoDims(cv::VideoCapture& cap, size_t width, size_t height);

void printFrameProps(const cv::Mat& frame);

void printCaptureProps(const cv::VideoCapture& cap);

void drawBoundingBoxes(cv::Mat& frame, const std::vector<FdBuffer>& buffers);
