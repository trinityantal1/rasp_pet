#include <iostream>
#include "controller.h"

Controller::Controller() {
    cam_controller = new CameraController();
    cam_controller->init();
    nn_controller = new NNController();
    nn_controller->init();
    comm_gateway = new CommGateway();
    comm_gateway->init();

}

void Controller::checkIfAlive() {
    std::cout << "This program is alive\n";
}

void Controller::startCameraLoop() {
    std::cout << "Starting camera loop...\n";
}

void Controller::stopCameraLoop() {
    std::cout << "Stopping camera loop...\n";
}

void Controller::captureCamera() {
    std::cout << "Capturing camera snapshot\n";
    cam_controller->TakePhotoToFile("capture_1.bmp");
}
void Controller::testCategorise() {
    std::cout << "Testing categorization\n";
    nn_controller->categorise("../rasp_pet_resource/capture_dog1.bmp");
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
    std::cout << "4. Test capture camera snapshot\n";
    std::cout << "5. Test categorise\n";
    std::cout << "6. Test SMS message\n";
    std::cout << "7. Test phone call\n";
    std::cout << "8. Exit\n";
    std::cout << "Enter your choice (1-8): ";
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
                this->captureCamera();
                break;
            case 5:
                this->testCategorise();
                break;
            case 6:
                this->testSMS();
                break;
            case 7:
                this->testPhoneCall();
                break;
            case 8:
                std::cout << "Exiting program.\n";
                return;
            default:
                std::cout << "Invalid choice. Please select a number between 1 and 8.\n";
        }
    }
}

Controller::~Controller() {
    delete comm_gateway;
}