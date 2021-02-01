#include "opencvutils.hh"

#include "log.h"

using namespace std;

tuple<int64_t, int64_t> getVideoDims(const cv::VideoCapture& cap) {
    using cvp = cv::VideoCaptureProperties;

    int64_t width = static_cast<int64_t>(cap.get(cvp::CAP_PROP_FRAME_WIDTH));
    int64_t height = static_cast<int64_t>(cap.get(cvp::CAP_PROP_FRAME_HEIGHT));

    return {width, height};
}

void setVideoDims(cv::VideoCapture& cap, size_t width, size_t height) {
    using cvp = cv::VideoCaptureProperties;

    logInfo("Setting video stream dimensions to %zu, %zu", width, height);

    if (!cap.set(cvp::CAP_PROP_FRAME_WIDTH, static_cast<double>(width)) &&
        !cap.set(cvp::CAP_PROP_FRAME_HEIGHT, static_cast<double>(height))) {
        logWarning("Failed to set video dimensions");
    }
}

void printFrameProps(const cv::Mat& frame) {
    logInfo("Frame properties:");
    logInfo("num pixels = %zu", frame.total());
    logInfo("byte size = %zu", frame.total() * frame.elemSize());
    logInfo("rows = %d", frame.rows);
    logInfo("cols = %d", frame.cols);
    logInfo("data = %p", frame.data);
    logInfo("element type = %d", frame.type());
    logInfo("depth = %d", frame.depth());
    logInfo("channels = %d", frame.channels());
}

void printCaptureProps(const cv::VideoCapture& cap) {
    using cvp = cv::VideoCaptureProperties;

    logInfo("VideoCapture properties:");
    logInfo("convertToRgb = %d",
            static_cast<int>(cap.get(cvp::CAP_PROP_CONVERT_RGB)));
    logInfo("fourcc = %#010x",
            static_cast<unsigned int>(cap.get(cvp::CAP_PROP_FOURCC)));
    auto [width, height] = getVideoDims(cap);
    logInfo("width = %zu", width);
    logInfo("height = %zu", height);
}

void drawBoundingBoxes(cv::Mat& frame, const vector<FdBuffer>& buffers) {
    // Here, we assume the output to be as in COCO SSD MobileNet v1: Four
    // tensors that specify.
    // 0. Locations: Multidimensional array of [10][4] floating point values
    // between 0 and 1, the inner arrays representing bounding boxes in the form
    // [top, left, bottom, right].
    // 1. Classes: Array of 10 integers (output as floating point values) each
    // indicating the index of a class label from the labels file.
    // 2. Scores Array of 10 floating point values between 0 and 1 representing
    // probability that a class was detected.
    // 3. Number and detections Array of length 1 containing a floating point
    // value expressing the total number of detection results.

    if (buffers.size() != 4) {
        throw invalid_argument("Wrong number of buffers for ssd");
    }

    auto boxPtr = reinterpret_cast<const float*>(buffers[0].data);
    auto scorePtr = reinterpret_cast<const float*>(buffers[2].data);

    const static float MIN_SCORE = 0.5;
    const cv::Scalar BOX_COLOR(0, 255, 0);
    const cv::Scalar TEXT_COLOR(255, 255, 0);

    // Add boxes around detections with a score above MIN_SCORE.
    float width = static_cast<float>(frame.cols);
    float height = static_cast<float>(frame.rows);
    for (size_t i = 0; i < 10; ++i) {
        if (*(scorePtr + i) > MIN_SCORE) {
            cv::Point topLeft(static_cast<int>(boxPtr[4 * i + 1] * width),
                              static_cast<int>(boxPtr[4 * i] * height));
            cv::Point bottomRight(static_cast<int>(boxPtr[4 * i + 3] * width),
                                  static_cast<int>(boxPtr[4 * i + 2] * height));
            cv::rectangle(frame, bottomRight, topLeft, BOX_COLOR);
        }
    }
}
