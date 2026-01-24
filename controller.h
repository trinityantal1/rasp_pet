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
    void CheckIfAlive();
    void StartCameraLoop();
    void StopCameraLoop();
    void CaptureCamera();
    void TestCategorise();
    void TestSMS();
    void TestPhoneCall();
    void ConsoleMenuHandler();
private:
    CameraController* cam_controller;
    NNController* nn_controller;
    CommGateway* comm_gateway;

};

#endif // CONTROLLER_H