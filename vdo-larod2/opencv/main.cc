/**
 * @file Small example of using OpenCV together with larod.
 */
#include "argparse.h"
#include "log.h"
#include "misc.hh"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/types_c.h"
#include "opencv2/opencv.hpp"

/**
 * The image format of the video stream is set by a fourcc code. Read more at
 * https://www.fourcc.org/. For reference, the YUYV code is 0x56595559.
 */
#define FOURCC(a, b, c, d)                                                     \
    ((static_cast<uint32_t>(a)) | (static_cast<uint32_t>(b) << 8) |            \
     (static_cast<uint32_t>(c) << 16) | /* NOLINT */                           \
     (static_cast<uint32_t>(d) << 24))  /* NOLINT */

using namespace std;

tuple<size_t, size_t> getVideoDims(const cv::VideoCapture& cap) {
    using cvp = cv::VideoCaptureProperties;

    size_t width = static_cast<size_t>(cap.get(cvp::CAP_PROP_FRAME_WIDTH));
    size_t height = static_cast<size_t>(cap.get(cvp::CAP_PROP_FRAME_HEIGHT));

    return {width, height};
}

/**
 * @brief Set video stream dimensions.
 *
 * If opencv uses Video4Linux as the backend, one can get the available formats
 * with "v4l2-ctl -d0 --list-formats-ext".
 */
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

void runVideoStream(const args_t& args) {
    logDebug(args.debug, "Setting up video capture");
    cv::VideoCapture cap;
    if (!cap.open(0 /* /dev/video0 */))
        return;
    setVideoDims(cap, args.width, args.height);

    if (args.runInference) {
        logDebug(args.debug, "Setting capture mode to YUYV");
        if (!cap.set(cv::VideoCaptureProperties::CAP_PROP_FOURCC,
                     FOURCC('Y', 'U', 'Y', 'V'))) {
            logError("Could not set capture mode");
            exit(-1);
        }
        if (!cap.set(cv::VideoCaptureProperties::CAP_PROP_CONVERT_RGB, 0)) {
            logError("Could not set capture mode");
            exit(-1);
        }
    }

    if (args.debug) {
        printCaptureProps(cap);
    }

    static const string windowName("webcam");
    cv::namedWindow(windowName);

    // Get first frame.
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) {
        logError("Got empty frame");
    }
    printFrameProps(frame);

    // Create fd that holds each frame. The buffer size is
    // (width * height) times 2 for YUYV or 3 for RGB.
    auto [width, height] = getVideoDims(cap);
    size_t bufferSize =
        args.runInference ? width * height * 2 : width * height * 3;
    FdBuffer fdBuffer(bufferSize);
    frame.data =
        fdBuffer.data; // Maybe this leaks something allocated in frame?

    logDebug(args.debug, "Starting video-stream loop");
    while (true) {
        TIMER("One frame");

        cap >> frame;
        if (frame.empty()) {
            throw runtime_error("Got empty frame");
        }

        if (args.runInference) {
            // TODO:
            //  Crop, scale and YUYV -> RGB_INTERLEAVED conversion.
            //  Run inference.
            //  Perform postprocessing of results.
            //  Create a cv::Mat to display using the converted RGB buffer.
            break;
        }

        cv::imshow(windowName, frame);

        if (cv::waitKey(5) == 27 /* ESC key */) {
            logInfo("Quitting video-stream loop");
            break;
        }
    }
}

int main(int argc, char** argv) {
    args_t args;
    bool ret = parseArgs(argc, argv, &args);
    if (!ret) {
        return -1;
    }

    logInfo("larod opencv example");
    runVideoStream(args);

    logInfo("Done");
    return 0;
}
