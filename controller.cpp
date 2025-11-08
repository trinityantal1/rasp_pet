#include <iostream>
#include "controller.h"

Controller::Controller() {
    cam_controller = new CameraController();
    cam_controller->init();
    nn_controller = new NNController();
    nn_controller->init();
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

void displayMenu() {
    std::cout << "\n=== Console Menu ===\n";
    std::cout << "1. Check if alive\n";
    std::cout << "2. Start camera loop\n";
    std::cout << "3. Stop camera loop\n";
    std::cout << "4. Test SMS message\n";
    std::cout << "5. Test phone call\n";
    std::cout << "6. Exit\n";
    std::cout << "Enter your choice (1-6): ";
}

void Controller::consoleMenuHandler() {

    int choice;

    while (true) {
        displayMenu();
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input. Please enter a number between 1 and 6.\n";
            continue;
        }

        switch (choice) {
            int a;
            case 1:
                this->checkIfAlive();
                break;
            case 2:
                this->startCameraLoop();
                break;
            case 3:
                this->stopCameraLoop();
                break;
            case 4:
                this->testSMS();
                break;
            case 5:
                this->testPhoneCall();
                break;
            case 6:
                std::cout << "Exiting program.\n";
                return;
            default:
                std::cout << "Invalid choice. Please select a number between 1 and 6.\n";
        }
    }
}

Controller::~Controller() {
    delete comm_gateway;
}