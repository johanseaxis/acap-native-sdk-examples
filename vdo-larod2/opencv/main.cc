/**
 * @file Small example of using OpenCV together with larod.
 */

#include <condition_variable>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <mutex>

#include "argparse.h"
#include "log.h"
#include "misc.hh"
#include "opencvutils.hh"

//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc/types_c.h"
//#include "opencv2/opencv.hpp"

using namespace std;

larodError* error = nullptr;

condition_variable nnCv;
mutex nnMtx;
bool nnJobIsDone = false;

void nnCallback(void*, larodError* error) {
    if (error) {
        logError("There was an error with the inference");
    }
    unique_lock<mutex> lck(nnMtx);
    nnJobIsDone = true;
    nnCv.notify_all();
}

void connectionDeleter(larodConnection* conn) {
    if (!larodDisconnect(&conn, &error)) {
        logError("Could not disconnect from larod: %s", error->msg);
        larodClearError(&error);
    }
}

unique_ptr<larodConnection, function<void(larodConnection*)>>
    createLarodConnection() {
    larodConnection* conn = nullptr;

    if (!larodConnect(&conn, &error)) {
        throw runtime_error("Could not connect to larod");
    }

    return {conn, connectionDeleter};
}

unique_ptr<larodModel, function<void(larodModel*)>>
    createLarodModel(larodConnection* conn, const int fd, const larodChip chip,
                     const char* name, const larodMap* params) {
    larodModel* model = larodLoadModel(conn, fd, chip, LAROD_ACCESS_PRIVATE,
                                       name, params, &error);
    if (!model) {
        throw runtime_error("Could not load model");
    }

    return {model, [](larodModel* m) { larodDestroyModel(&m); }};
}

unique_ptr<larodMap, function<void(larodMap*)>> createLarodMap() {
    larodMap* map = larodCreateMap(&error);
    if (!map) {
        throw runtime_error("Could not create preprocessing larodMap");
    }

    return {map, [](larodMap* m) { larodDestroyMap(&m); }};
}

struct Tensors {
    larodTensor** tensors;
    size_t num;

    Tensors(larodTensor** tensors, size_t num) : tensors(tensors), num(num) {}
    ~Tensors() { larodDestroyTensors(&tensors, num); }
    Tensors(Tensors&& o) noexcept : tensors(o.tensors), num(o.num) {
        o.num = 0;
        o.tensors = nullptr;
    }
    Tensors(const Tensors&) = delete;
    Tensors& operator=(const Tensors&) = delete;
    Tensors&& operator=(Tensors&&) = delete;
};

tuple<Tensors, Tensors> createModelTensors(larodModel* model) {
    size_t numInputs = 0;
    size_t numOutputs = 0;
    larodTensor** inputs = larodCreateModelInputs(model, &numInputs, &error);
    if (!inputs) {
        throw runtime_error("Could not create model inputs");
    }
    Tensors inputTensors(inputs, numInputs);

    larodTensor** outputs = larodCreateModelOutputs(model, &numOutputs, &error);
    if (!outputs) {
        throw runtime_error("Could not create model outputs");
    }
    Tensors outputTensors(outputs, numOutputs);

    return {move(inputTensors), move(outputTensors)};
}

size_t getTensorBufferSize(larodTensor* tensor) {
    const larodTensorPitches* pitches = larodGetTensorPitches(tensor, &error);
    if (!pitches) {
        throw runtime_error("Could not get pitches of tensor");
    }

    // The zero component of the pitches corresponds to the total buffer size
    // including any padding, see @c larodTensorPitches.
    return pitches->pitches[0];
}

unique_ptr<larodJobRequest, function<void(larodJobRequest*)>>
    createLarodJobRequest(const larodModel* model, larodTensor** inputTensors,
                          size_t numInputs, larodTensor** outputTensors,
                          size_t numOutputs, larodMap* params) {
    larodJobRequest* jobReq =
        larodCreateJobRequest(model, inputTensors, numInputs, outputTensors,
                              numOutputs, params, &error);
    if (!jobReq) {
        throw runtime_error("Could not create job request");
    }

    return {jobReq, [](larodJobRequest* r) { larodDestroyJobRequest(&r); }};
}

void runVideoStream(const args_t& args) {
    logDebug(args.debug, "Setting up video capture");
    cv::VideoCapture cap;
    if (!cap.open(0 /* /dev/video0 */))
        return;
    setVideoDims(cap, args.width, args.height);

    logDebug(args.debug, "Setting capture mode to YUYV");
    if (!cap.set(cv::VideoCaptureProperties::CAP_PROP_FOURCC,
                 FOURCC('Y', 'U', 'Y', 'V'))) {
        throw runtime_error("Could not set capture mode");
    }
    if (!cap.set(cv::VideoCaptureProperties::CAP_PROP_CONVERT_RGB, 0)) {
        throw runtime_error("Could not set capture mode");
    }

    if (args.debug) {
        printCaptureProps(cap);
    }

    auto conn = createLarodConnection();

    // Load Neural-Network model from file and create a model handle.
    const char* filePath =
        "/larod/models/mobilenet-v2-ssd/"
        "ssd_mobilenet_v2_coco_quant_postprocess_edgetpu.larod";
    int nnModelFd = open(filePath, O_RDONLY);
    if (nnModelFd < 0) {
        throw invalid_argument("Could not open model file: " +
                               string(strerror(errno)));
    }
    auto nnModel = createLarodModel(conn.get(), nnModelFd, LAROD_CHIP_TPU,
                                    "Mobilenet-v2-ssd", nullptr);

    // Construct a larodMap that defines the preprocessing and create a model
    // handle.
    auto ppMap = createLarodMap();
    auto cropMap = createLarodMap();
    auto cvMap = createLarodMap();

    const auto [srcWidth, srcHeight] = getVideoDims(cap);

    constexpr int64_t nnWidth = 300;
    constexpr int64_t nnHeight = nnWidth;

    const int64_t cropX = ((srcWidth - srcHeight) / 2);
    const int64_t cropY = 0;
    const int64_t cropW = srcHeight;
    const int64_t cropH = cropW;
    if (!larodMapSetIntArr4(cropMap.get(), "image.input.crop", cropX, cropY,
                            cropW, cropH, &error)) {
        throw runtime_error("Failed to set larodMap parameters");
    }

    if (!larodMapSetIntArr2(ppMap.get(), "image.input.size", srcWidth,
                            srcHeight, &error) ||
        !larodMapSetStr(ppMap.get(), "image.input.format", "yuyv", &error) ||
        !larodMapSetStr(ppMap.get(), "image.output.format", "rgb-interleaved",
                        &error) ||
        !larodMapSetIntArr2(ppMap.get(), "image.output.size", nnWidth, nnHeight,
                            &error)) {
        throw runtime_error("Failed to set larodMap parameters");
    }
    auto ppModel = createLarodModel(conn.get(), -1, LAROD_CHIP_LIBYUV,
                                    "yuyvToRgb", ppMap.get());

    if (!larodMapSetIntArr2(cvMap.get(), "image.input.size", srcWidth,
                            srcHeight, &error) ||
        !larodMapSetStr(cvMap.get(), "image.input.format", "yuyv", &error) ||
        !larodMapSetStr(cvMap.get(), "image.output.format", "rgb-interleaved",
                        &error) ||
        !larodMapSetIntArr2(cvMap.get(), "image.output.size", srcHeight,
                            srcHeight, &error)) {
        throw runtime_error("Failed to set larodMap parameters");
    }
    auto cvModel = createLarodModel(conn.get(), -1, LAROD_CHIP_LIBYUV,
                                    "yuyvToRgb", cvMap.get());

    // Get input/outputs for each model.
    auto [ppInputs, ppOutputs] = createModelTensors(ppModel.get());
    auto [nnInputs, nnOutputs] = createModelTensors(nnModel.get());
    auto [cvInputs, cvOutputs] = createModelTensors(cvModel.get());

    // Create fd that holds each tensor buffer. The buffer size is (width *
    // height) times 2 (3) for YUYV (RGB).
    size_t yuyvBufferSize = getTensorBufferSize(ppInputs.tensors[0]);
    size_t rgbBufferSize = getTensorBufferSize(ppOutputs.tensors[0]);
    size_t cvBufferSize = getTensorBufferSize(cvOutputs.tensors[0]);
    if (srcWidth * srcHeight * 2 != static_cast<int64_t>(yuyvBufferSize)) {
        throw runtime_error(
            "Mismatch between expected buffer size and video stream");
    }
    FdBuffer yuyvBuffer(yuyvBufferSize);
    FdBuffer rgbBuffer(rgbBufferSize);
    FdBuffer cvBuffer(cvBufferSize);
    vector<FdBuffer> nnOutputBuffers;
    for (size_t i = 0; i < nnOutputs.num; ++i) {
        size_t bufferSize = getTensorBufferSize(nnOutputs.tensors[i]);
        nnOutputBuffers.emplace_back(bufferSize);
    }

    // We attach cv::Mat classes on the buffer that is filled with camera
    // data; as well as the one that is shown in the end.
    cv::Mat yuyvFrame(static_cast<int>(srcHeight), static_cast<int>(srcWidth),
                      CV_8UC2, yuyvBuffer.data);
    cv::Mat rgbFrame(static_cast<int>(srcHeight), static_cast<int>(srcHeight),
                     CV_8UC3, cvBuffer.data);

    // Set the file descriptors for each tensor.
    if (!larodSetTensorFd(ppInputs.tensors[0], yuyvBuffer.fd, &error)) {
        throw runtime_error("Could not set fd for preprocessing input");
    }
    if (!larodSetTensorFd(ppOutputs.tensors[0], rgbBuffer.fd, &error)) {
        throw runtime_error("Could not set fd for preprocessing input");
    }
    if (!larodSetTensorFd(nnInputs.tensors[0], rgbBuffer.fd, &error)) {
        throw runtime_error("Could not set fd for neural-network input");
    }
    if (!larodSetTensorFd(cvInputs.tensors[0], yuyvBuffer.fd, &error)) {
        throw runtime_error("Could not set fd for neural-network input");
    }
    if (!larodSetTensorFd(cvOutputs.tensors[0], cvBuffer.fd, &error)) {
        throw runtime_error("Could not set fd for neural-network input");
    }
    for (size_t i = 0; i < nnOutputs.num; ++i) {
        larodSetTensorFd(nnOutputs.tensors[i], nnOutputBuffers[i].fd, &error);
    }

    logDebug(args.debug, "Creating job requests");
    auto nnReq =
        createLarodJobRequest(nnModel.get(), nnInputs.tensors, nnInputs.num,
                              nnOutputs.tensors, nnOutputs.num, nullptr);
    auto ppReq =
        createLarodJobRequest(ppModel.get(), ppInputs.tensors, ppInputs.num,
                              ppOutputs.tensors, ppOutputs.num, cropMap.get());
    auto cvReq =
        createLarodJobRequest(cvModel.get(), cvInputs.tensors, cvInputs.num,
                              cvOutputs.tensors, cvOutputs.num, cropMap.get());

    logDebug(args.debug, "Starting video-stream loop");
    static const string windowName("webcam");
    cv::namedWindow(windowName);

    // Capture first frame.
    cap >> yuyvFrame;
    if (yuyvFrame.empty()) {
        throw runtime_error("Got empty frame");
    }
    if (args.debug) {
        printFrameProps(yuyvFrame);
    }

    while (true) {
        TIMER("One frame");

        // Preprocessing (synchronous).
        if (!larodRunJob(conn.get(), ppReq.get(), &error)) {
            throw runtime_error("Could not run preprocessing");
        }

        // NN inference (asynchronous)
        nnJobIsDone = false;
        if (!larodRunJobAsync(conn.get(), nnReq.get(), nnCallback, nullptr,
                              &error)) {
            throw runtime_error("Could not run job async");
        }

        // Preprocessing (synchronous).
        if (!larodRunJob(conn.get(), cvReq.get(), &error)) {
            throw runtime_error("Could not run preprocessing");
        }

        cap >> yuyvFrame;
        if (yuyvFrame.empty()) {
            throw runtime_error("Got empty frame");
        }

        if (cv::waitKey(5) == 27 /* ESC key */) {
            logInfo("Quitting video-stream loop");
            break;
        }

        unique_lock<mutex> lck(nnMtx);
        nnCv.wait(lck, []() { return nnJobIsDone; });

        drawBoundingBoxes(rgbFrame, nnOutputBuffers);

        cv::imshow(windowName, rgbFrame);
    }
}

int main(int argc, char** argv) {
    args_t args;
    bool ret = parseArgs(argc, argv, &args);
    if (!ret) {
        return -1;
    }

    logInfo("larod opencv example");
    try {
        runVideoStream(args);
    } catch (exception& e) {
        logError("%s", e.what());
        if (error) {
            logError("larod error: %s", error->msg);
            larodClearError(&error);
        }
    }

    logDebug(args.debug, "Done");
}
