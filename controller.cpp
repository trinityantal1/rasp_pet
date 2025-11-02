#include <iostream>
#include "controller.h"

Controller::Controller() {
    comm_gateway = new CommGateway();
    comm_gateway->init();
    isCameraLoop = false;
}

void Controller::checkIfAlive() {
    std::cout << "This program is alive\n";
}

void Controller::startCameraLoop() {
    std::cout << "Starting camera loop...\n";
    // Placeholder for camera loop start logic
}

void Controller::stopCameraLoop() {
    std::cout << "Stopping camera loop...\n";
    // Placeholder for camera loop stop logic
}

void Controller::testSMS() {
   comm_gateway->testSMS();
}

void Controller::testPhoneCall() {
    comm_gateway->testPhoneCall();
}

Controller::~Controller() {
    delete comm_gateway;
}