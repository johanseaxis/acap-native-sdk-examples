/**
 * Copyright (C) 2020 Axis Communications AB, Lund, Sweden
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 /**
  * - object_detection -
  *
  * This application loads a larod model which takes an image as input and
  * outputs values corresponding to the class, score and location of detected 
  * objects in image.
  *
  * The application expects three arguments on the command line in the
  * following order: MODEL WIDTH HEIGHT
  *
  * First argument, MODEL, is a string describing path to the model.
  *
  * Second argument, WIDTH, is an integer for the input width.
  *
  * Third argument, HEIGHT, is an integer for input height.
  */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include "argparse.h"
#include "imgconverter.h"
#include "imgprovider.h"
#include "imgutils.h"
#include "larod.h"
#include "vdo-frame.h"
#include "vdo-types.h"


char class_name[90][20] = {
  "person",
  "bicycle",   
  "car",
  "motorcycle",
  "airplane",
  "bus",
  "train",
  "truck",
  "boat",
  "traffic light",
  "fire hydrant",
  "stop sign",
  "parking meter",
  "bench",
  "bird",
  "cat",
  "dog",
  "horse",
  "sheep",
  "cow",
  "elephant",
  "bear",
  "zebra",
  "giraffe",
  "backpack",
  "umbrella",
  "handbag",
  "tie",
  "suitcase",
  "frisbee",
  "skis",
  "snowboard",
  "sports ball",
  "kite",
  "baseball bat",
  "baseball glove",
  "skateboard",
  "surfboard",
  "tennis racket",
  "bottle",
  "wine glass",
  "cup",
  "fork",
  "knife",
  "spoon",
  "bowl",
  "banana",
  "apple",
  "sandwich",
  "orange",
  "broccoli",
  "carrot",
  "hot dog",
  "pizza",
  "donut",
  "cake",
  "chair",
  "couch",
  "potted plant",
  "bed",
  "dining table",
  "toilet",
  "tv",
  "laptop",
  "mouse",
  "remote",
  "keyboard",
  "cell phone",
  "microwave",
  "oven",
  "toaster",
  "sink",
  "refrigerator",
  "book",
  "clock",
  "vase",
  "scissors",
  "teddy bear",
  "hair drier",
  "toothbrush"
};


/// Set by signal handler if an interrupt signal sent to process.
/// Indicates that app should stop asap and exit gracefully.
volatile sig_atomic_t stopRunning = false;

/**
 * brief Invoked on SIGINT. Makes app exit cleanly asap if invoked once, but
 * forces an immediate exit without clean up if invoked at least twice.
 *
 * param sig What signal has been sent. Will always be SIGINT.
 */
void sigintHandler(int sig) {
    // Force an exit if SIGINT has already been sent before.
    if (stopRunning) {
        syslog(LOG_INFO, "Interrupted again, exiting immediately without clean up.");

        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Interrupted, starting graceful termination of app. Another "
            "interrupt signal will cause a forced exit.");

    // Tell the main thread to stop running inferences asap.
    stopRunning = true;
}

/**
 * brief Creates a temporary fd and truncated to correct size and mapped.
 *
 * This convenience function creates temp files to be used for input and output.
 *
 * param fileName Pattern for how the temp file will be named in file system.
 * param fileSize How much space needed to be allocated (truncated) in fd.
 * param mappedAddr Pointer to the address of the fd mapped for this process.
 * param Pointer to the generated fd.
 * return Positive errno style return code (zero means success).
 */
bool createAndMapTmpFile(char* fileName, size_t fileSize, void** mappedAddr,
                         int* convFd) {
    syslog(LOG_INFO, "%s: Setting up a temp fd with pattern %s and size %zu", __func__,
            fileName, fileSize);

    int fd = mkstemp(fileName);
    if (fd < 0) {
        syslog(LOG_ERR, "%s: Unable to open temp file %s: %s", __func__, fileName,
                 strerror(errno));
        goto error;
    }

    // Allocate enough space in for the fd.
    if (ftruncate(fd, (off_t) fileSize) < 0) {
        syslog(LOG_ERR, "%s: Unable to truncate temp file %s: %s", __func__, fileName,
                 strerror(errno));
        goto error;
    }

    // Remove since we don't actually care about writing to the file system.
    if (unlink(fileName)) {
        syslog(LOG_ERR, "%s: Unable to unlink from temp file %s: %s", __func__,
                 fileName, strerror(errno));
        goto error;
    }

    // Get an address to fd's memory for this process's memory space.
    void* data =
        mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (data == MAP_FAILED) {
        syslog(LOG_ERR, "%s: Unable to mmap temp file %s: %s", __func__, fileName,
                 strerror(errno));
        goto error;
    }

    *mappedAddr = data;
    *convFd = fd;

    return true;

error:
    if (fd >= 0) {
        close(fd);
    }

    return false;
}

/**
 * brief Sets up and configures a connection to larod, and loads a model.
 *
 * Opens a connection to larod, which is tied to larodConn. After opening a
 * larod connection the chip specified by larodChip is set for the
 * connection. Then the model file specified by larodModelFd is loaded to the
 * chip, and a corresponding larodModel object is tied to model.
 *
 * param larodChip Speficier for which larod chip to use.
 * param larodModelFd Fd for a model file to load.
 * param larodConn Pointer to a larod connection to be opened.
 * param model Pointer to a larodModel to be obtained.
 * return false if error has occurred, otherwise true.
 */
bool setupLarod(const larodChip larodChip, const int larodModelFd,
                larodConnection** larodConn, larodModel** model) {
    larodError* error = NULL;
    larodConnection* conn = NULL;
    larodModel* loadedModel = NULL;
    bool ret = false;

    // Set up larod connection.
    if (!larodConnect(&conn, &error)) {
        syslog(LOG_ERR, "%s: Could not connect to larod: %s", __func__, error->msg);
        goto end;
    }

    // Set chip if user has specified non-default action.
    if (larodChip != 0) {
        if (!larodSetChip(conn, larodChip, &error)) {
            syslog(LOG_ERR, "%s: Could not select chip %d: %s", __func__, larodChip,
                     error->msg);
            goto error;
        }
    }

    loadedModel = larodLoadModel(conn, larodModelFd, LAROD_ACCESS_PRIVATE,
                                 "object_detection", &error);
    if (!loadedModel) {
        syslog(LOG_ERR, "%s: Unable to load model: %s", __func__, error->msg);
        goto error;
    }

    *larodConn = conn;
    *model = loadedModel;

    ret = true;

    goto end;

error:
    if (conn) {
        larodDisconnect(&conn, NULL);
    }

end:
    if (error) {
        larodClearError(&error);
    }

    return ret;
}

//void get_tensor_information()
/**
 * brief Main function that starts a stream with different options.
 */
int main(int argc, char** argv) {
    // Hardcode to use three image "color" channels (eg. RGB).
    const unsigned int CHANNELS = 3;

    // Name patterns for the temp file we will create.
    char CONV_INP_FILE_PATTERN[] = "/tmp/larod.in.test-XXXXXX";
    char CROP_FILE_PATTERN[] = "/tmp/crop.test-XXXXXX";
    char CONV_OUT1_FILE_PATTERN[] = "/tmp/larod.out1.test-XXXXXX";
    char CONV_OUT2_FILE_PATTERN[] = "/tmp/larod.out2.test-XXXXXX";
    char CONV_OUT3_FILE_PATTERN[] = "/tmp/larod.out3.test-XXXXXX";
    char CONV_OUT4_FILE_PATTERN[] = "/tmp/larod.out4.test-XXXXXX";

    bool ret = false;
    ImgProvider_t* provider = NULL;
    ImgProvider_t* provider_crop = NULL;
    larodError* error = NULL;
    larodConnection* conn = NULL;
    larodTensor** inputTensors = NULL;
    size_t numInputs = 0;
    larodTensor** outputTensors = NULL;
    size_t numOutputs = 0;
    larodInferenceRequest* infReq = NULL;
    void* larodInputAddr = MAP_FAILED;
    void* cropAddr = MAP_FAILED;
    void* larodOutput1Addr = MAP_FAILED;
    void* larodOutput2Addr = MAP_FAILED;
    void* larodOutput3Addr = MAP_FAILED;
    void* larodOutput4Addr = MAP_FAILED;
    int larodModelFd = -1;
    int larodInputFd = -1;
    int cropFd = -1;
    int larodOutput1Fd = -1;
    int larodOutput2Fd = -1;
    int larodOutput3Fd = -1;
    int larodOutput4Fd = -1;
    args_t args;

    // Open the syslog to report messages for "object_detection"
    openlog("object_detection", LOG_PID|LOG_CONS, LOG_USER);

    syslog(LOG_INFO, "Starting ...");

    // Register an interrupt handler which tries to exit cleanly if invoked once
    // but exits immediately if further invoked.
    signal(SIGINT, sigintHandler);

    if (!parseArgs(argc, argv, &args)) {
        goto end;
    }

    unsigned int streamWidth = 0;
    unsigned int streamHeight = 0;
    if (!chooseStreamResolution(args.width, args.height, &streamWidth,
                                &streamHeight)) {
        syslog(LOG_ERR, "%s: Failed choosing stream resolution", __func__);
        goto end;
    }

    syslog(LOG_INFO, "Creating VDO image provider and creating stream %d x %d",
            streamWidth, streamHeight);
    provider = createImgProvider(streamWidth, streamHeight, 2, VDO_FORMAT_YUV);
    if (!provider) {
      syslog(LOG_ERR, "%s: Failed to create ImgProvider", __func__);
        goto end;
    }

    provider_crop = createImgProvider(1920, 1080, 2, VDO_FORMAT_YUV);
    if (!provider_crop) {
      syslog(LOG_ERR, "%s: Failed to create crop ImgProvider", __func__);
        goto end;
    }

    larodModelFd = open(args.modelFile, O_RDONLY);
    if (larodModelFd < 0) {
        syslog(LOG_ERR, "Unable to open model file %s: %s", args.modelFile,
                 strerror(errno));
        goto end;
    }

    syslog(LOG_INFO, "Setting up larod connection with chip %d and model %s", args.chip,
            args.modelFile);
    larodModel* model = NULL;
    if (!setupLarod(args.chip, larodModelFd, &conn, &model)) {
        goto end;
    }

    syslog(LOG_INFO, "Creating temporary files and memmaps for inference input and "
            "output tensors");


    // Allocate space for input tensor
    if (!createAndMapTmpFile(CONV_INP_FILE_PATTERN,
                             args.width * args.height * CHANNELS,
                             &larodInputAddr, &larodInputFd)) {
        goto end;
    }

    // Allocate space for input tensor 2
    if (!createAndMapTmpFile(CROP_FILE_PATTERN,
                             1920 * 1080 * CHANNELS,
                             &cropAddr, &cropFd)) {
        goto end;
    }

    // Allocate space for output tensor 1 ()
    if (!createAndMapTmpFile(CONV_OUT1_FILE_PATTERN, args.outputBytes,
                             &larodOutput1Addr, &larodOutput1Fd)) {
        goto end;
    }

    // Allocate space for output tensor 2 ()
    if (!createAndMapTmpFile(CONV_OUT2_FILE_PATTERN, args.outputBytes,
                             &larodOutput2Addr, &larodOutput2Fd)) {
        goto end;
    }

    // Allocate space for output tensor 3 ()
    if (!createAndMapTmpFile(CONV_OUT3_FILE_PATTERN, args.outputBytes,
                             &larodOutput3Addr, &larodOutput3Fd)) {
        goto end;
    }

    // Allocate space for output tensor 4 ()
    if (!createAndMapTmpFile(CONV_OUT4_FILE_PATTERN, args.outputBytes,
                             &larodOutput4Addr, &larodOutput4Fd)) {
        goto end;
    }

    syslog(LOG_INFO, "Create input tensors");
    inputTensors = larodCreateModelInputs(model, &numInputs, &error);
    if (!inputTensors) {
        syslog(LOG_ERR, "Failed retrieving input tensors: %s", error->msg);
        goto end;
    }

    syslog(LOG_INFO, "Set input tensors");
    if (!larodSetTensorFd(inputTensors[0], larodInputFd, &error)) {
        syslog(LOG_ERR, "Failed setting input tensor fd: %s", error->msg);
        goto end;
    }

    syslog(LOG_INFO, "Create output tensors");
    outputTensors = larodCreateModelOutputs(model, &numOutputs, &error);
    if (!outputTensors) {
        syslog(LOG_ERR, "Failed retrieving output tensors: %s", error->msg);
        goto end;
    }


    syslog(LOG_INFO, "Set output tensors");
    if (!larodSetTensorFd(outputTensors[0], larodOutput1Fd, &error)) {
        syslog(LOG_ERR, "Failed setting output tensor fd: %s", error->msg);
        goto end;
    }

    if (!larodSetTensorFd(outputTensors[1], larodOutput2Fd, &error)) {
        syslog(LOG_ERR, "Failed setting output tensor fd: %s", error->msg);
        goto end;
    }

    if (!larodSetTensorFd(outputTensors[2], larodOutput3Fd, &error)) {
        syslog(LOG_ERR, "Failed setting output tensor fd: %s", error->msg);
        goto end;
    }

    if (!larodSetTensorFd(outputTensors[3], larodOutput4Fd, &error)) {
        syslog(LOG_ERR, "Failed setting output tensor fd: %s", error->msg);
        goto end;
    }

    infReq = larodCreateInferenceRequest(model, inputTensors, numInputs, outputTensors,
                                         numOutputs, &error);
    if (!infReq) {
        syslog(LOG_ERR, "Failed creating inference request: %s", error->msg);
        goto end;
    }

    syslog(LOG_INFO, "Found %x input tensors and %x output tensors", numInputs, numOutputs);
    syslog(LOG_INFO, "Start fetching video frames from VDO");
    if (!startFrameFetch(provider)) {
        goto end;
    }

    if (!startFrameFetch(provider_crop)) {
        goto end;
    }

    while (true) {
        struct timeval startTs, endTs;
        unsigned int elapsedMs = 0;

        // Get latest frame from image pipeline.
        VdoBuffer* buf = getLastFrameBlocking(provider);
        if (!buf) {
            goto end;
        }

        VdoBuffer* buf_crop = getLastFrameBlocking(provider_crop);
        if (!buf_crop) {
            goto end;
        }

        // Get data from latest frame.
        uint8_t* nv12Data = (uint8_t*) vdo_buffer_get_data(buf);
        uint8_t* nv12Data_crop = (uint8_t*) vdo_buffer_get_data(buf_crop);

        // Covert image data from NV12 format to interleaved uint8_t RGB format.
        gettimeofday(&startTs, NULL);

        if (!convertCropScaleU8yuvToRGB(nv12Data, streamWidth, streamHeight,
                                        (uint8_t*) larodInputAddr, args.width,
                                        args.height)) {
            syslog(LOG_ERR, "%s: Failed img scale/convert in "
                     "convertCropScaleU8yuvToRGB() (continue anyway)",
                     __func__);
        }

        convertU8yuvToRGBlibYuv(1920, 1080, nv12Data_crop, (uint8_t*) cropAddr);

        gettimeofday(&endTs, NULL);

        elapsedMs = (unsigned int) (((endTs.tv_sec - startTs.tv_sec) * 1000) +
                                    ((endTs.tv_usec - startTs.tv_usec) / 1000));
        syslog(LOG_INFO, "Converted image in %u ms", elapsedMs);

        // Since larodOutputAddr points to the beginning of the fd we should
        // rewind the file position before each inference.
        if (lseek(larodOutput1Fd, 0, SEEK_SET) == -1) {
            syslog(LOG_ERR, "Unable to rewind output file position: %s",
                     strerror(errno));

            goto end;
        }

        if (lseek(larodOutput2Fd, 0, SEEK_SET) == -1) {
            syslog(LOG_ERR, "Unable to rewind output file position: %s",
                     strerror(errno));

            goto end;
        }

        if (lseek(larodOutput3Fd, 0, SEEK_SET) == -1) {
            syslog(LOG_ERR, "Unable to rewind output file position: %s",
                     strerror(errno));

            goto end;
        }

        if (lseek(larodOutput4Fd, 0, SEEK_SET) == -1) {
            syslog(LOG_ERR, "Unable to rewind output file position: %s",
                     strerror(errno));

            goto end;
        }

        gettimeofday(&startTs, NULL);
        if (!larodRunInference(conn, infReq, &error)) {
            syslog(LOG_ERR, "Unable to run inference on model %s: %s (%d)",
                     args.modelFile, error->msg, error->code);
            goto end;
        }
        gettimeofday(&endTs, NULL);

        elapsedMs = (unsigned int) (((endTs.tv_sec - startTs.tv_sec) * 1000) +
                                    ((endTs.tv_usec - startTs.tv_usec) / 1000));
        syslog(LOG_INFO, "Ran inference for %u ms", elapsedMs);

        float* locations = (float*) larodOutput1Addr;
        float* classes = (float*) larodOutput2Addr;
        float* scores = (float*) larodOutput3Addr;
        float* numberofdetections = (float*) larodOutput4Addr;
        // Threshold of the class of detected objects
        float MIN_SCORE = 0.1;
        
        if ((int) numberofdetections[0] == 0) {
           syslog(LOG_INFO,"No object is detected");
        }
        else {
            
            syslog(LOG_INFO,"There are %d objects", (int) numberofdetections[0]);
            for (int i = 0; i < numberofdetections[0]; i++){

                float top = locations[4*i];
                float left = locations[4*i+1];
                float bottom = locations[4*i+2];
                float right = locations[4*i+3];

                unsigned int crop_x = left * 1920; 
                unsigned int crop_y = top * 1080;
                unsigned int crop_w = (right - left) * 1920;
                unsigned int crop_h = (bottom - top) * 1080;
     
                if( scores[i] > MIN_SCORE ){

                    syslog(LOG_INFO, "Object %d: Classes: %s - Scores: %f - Locations: [%f,%f,%f,%f]",
                       i, class_name[(int) classes[i]], scores[i], top, left, bottom, right);
                    unsigned char* crop_buffer = crop_interleaved(cropAddr, 1920, 1080, 3,
                                                                  crop_x, crop_y, crop_w, crop_h);

                    unsigned long jpeg_size = 0;
                    unsigned char* jpeg_buffer = NULL;
                    struct jpeg_compress_struct jpeg_conf;
                    set_jpeg_configuration(crop_w, crop_h, 3, 80, &jpeg_conf);
                    buffer_to_jpeg(crop_buffer, &jpeg_conf, &jpeg_size, &jpeg_buffer);
                    char file_name[32];
                    snprintf(file_name, sizeof(char) * 32, "/tmp/detection_%i.jpg", i);
                    jpeg_to_file(file_name, jpeg_buffer, jpeg_size);
                    free(crop_buffer);
                    free(jpeg_buffer);
                }
            }
 
        }          

        // Release frame reference to provider.
        returnFrame(provider, buf);
        returnFrame(provider_crop, buf_crop);
    }

    syslog(LOG_INFO, "Stop streaming video from VDO");
    if (!stopFrameFetch(provider)) {
        goto end;
    }

    ret = true;

end:
    if (provider) {
        destroyImgProvider(provider);
    }
        if (provider_crop) {
        destroyImgProvider(provider_crop);
    }
    // Only the model handle is released here. We count on larod service to
    // release the privately loaded model when the session is disconnected in
    // larodDisconnect().
    larodDestroyModel(&model);
    if (conn) {
        larodDisconnect(&conn, NULL);
    }
    if (larodModelFd >= 0) {
        close(larodModelFd);
    }
    if (larodInputAddr != MAP_FAILED) {
        munmap(larodInputAddr, args.width * args.height * CHANNELS);
    }
    if (larodInputFd >= 0) {
        close(larodInputFd);
    }
     if (cropAddr != MAP_FAILED) {
        munmap(cropAddr, 1920 * 1080 * CHANNELS);
    }
    if (cropFd >= 0) {
        close(cropFd);
    }
    if (larodOutput1Addr != MAP_FAILED) {
        munmap(larodOutput1Addr, args.outputBytes);
    }

    if (larodOutput2Addr != MAP_FAILED) {
        munmap(larodOutput2Addr, args.outputBytes);
    }

    if (larodOutput3Addr != MAP_FAILED) {
        munmap(larodOutput3Addr, args.outputBytes);
    }

    if (larodOutput4Addr != MAP_FAILED) {
        munmap(larodOutput4Addr, args.outputBytes);
    }

    if (larodOutput1Fd >= 0) {
        close(larodOutput1Fd);
    }

    if (larodOutput2Fd >= 0) {
        close(larodOutput2Fd);
    }

    if (larodOutput3Fd >= 0) {
        close(larodOutput3Fd);
    }

    if (larodOutput4Fd >= 0) {
        close(larodOutput4Fd);
    }

    larodDestroyInferenceRequest(&infReq);
    larodDestroyTensors(&inputTensors, numInputs);
    larodDestroyTensors(&outputTensors, numOutputs);
    larodClearError(&error);

    // Close application logging to syslog
    closelog();

    return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}
