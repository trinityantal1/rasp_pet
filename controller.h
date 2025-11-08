#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sys/mman.h>
#include <thread>

#include <libcamera/libcamera.h>
#include <libcamera/controls.h>
#include <libcamera/property_ids.h>

#include "camera_controller.h"
#include "nn_controller.h"
#include "communication_gateway.h"

using namespace libcamera;
using namespace std::chrono_literals;
using namespace std::chrono;

class Controller {
public:
    Controller();
    ~Controller();
    void checkIfAlive();
    void startCameraLoop();
    void stopCameraLoop();
    void testSMS();
    void testPhoneCall();
    void consoleMenuHandler();
private:
    CameraController* cam_controller;
    NNController* nn_controller;
    CommGateway* comm_gateway;

    std::shared_ptr<Camera> camera;
    std::unique_ptr<CameraManager> cm;
    std::unique_ptr<CameraConfiguration> config;
    std::unique_ptr<FrameBufferAllocator> allocator;
    std::unique_ptr<Request> request;
    std::mutex mutex;
    std::condition_variable cv;
    bool isCameraLoop;
    steady_clock::time_point start_time;
    void requestComplete(Request *request);
};

#endif // CONTROLLER_H