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

void Controller::CheckIfAlive() {
    std::cout << "This program is alive\n";
}

void Controller::StartCameraLoop() {
    std::cout << "Starting camera loop...\n";
    cam_controller->StartCameraLoop(this);
}

void Controller::StopCameraLoop() {
    std::cout << "Stopping camera loop...\n";
    cam_controller->StopCameraLoop();
}

void Controller::CaptureCamera() {
    std::cout << "Capturing camera snapshot\n";
    cam_controller->TakePhotoToFile("takephoto.bmp");
}
void Controller::TestCategorise() {
    std::cout << "Testing categorization\n";
    nn_controller->categorise("../rasp_pet_resource/capture_dog1.bmp");
}

void Controller::TestSMS() {
   comm_gateway->SendSMS("Testing SMS");
}

void Controller::TestPhoneCall() {
    comm_gateway->SendPhoneCall("Testing Phone Call");
}

void Controller::DisplayMenu() {
    std::cout << "\n=== Console Menu ===\n";
    std::cout << "1. Check if alive\n";
    std::cout << "2. Start camera loop\n";
    std::cout << "3. Stop camera loop\n";
    std::cout << "4. Test capture camera snapshot\n";
    std::cout << "5. Test categorise\n";
    std::cout << "6. Test SMS message\n";
    std::cout << "7. Test phone call\n";
    if (comm_gateway->isSMSorVoice) {
        std::cout << "8. Currently using SMS mode. Switch to Voice mode\n";
    }
    else {
        std::cout << "8. Currently using Voice mode. Switch to SMS mode\n";
    }
    std::cout << "9. Exit\n";
    std::cout << "Enter your choice (1-9): ";
}

void Controller::ConsoleMenuHandler() {

    std::cout << "Main Thread ID: " << std::this_thread::get_id() << "\n"; // to check if camera loop will run in separate thread
    int choice;

    while (true) {
        DisplayMenu();
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
                this->CheckIfAlive();
                break;
            case 2:
                this->StartCameraLoop();
                break;
            case 3:
                this->StopCameraLoop();
                break;
            case 4:
                this->CaptureCamera();
                break;
            case 5:
                this->TestCategorise();
                break;
            case 6:
                this->TestSMS();
                break;
            case 7:
                this->TestPhoneCall();
                break;
            case 8:
                comm_gateway->isSMSorVoice = !comm_gateway->isSMSorVoice;
                break;
            case 9:
                std::cout << "Exiting program.\n";
                return;
            default:
                std::cout << "Invalid choice. Please select a number between 1 and 8.\n";
        }
    }
}

void Controller::AnalyseImage(const std::string& imgFilename) {
    std::cout << "Analysing image: " << imgFilename << std::endl;

    bool hasCatOrDog = nn_controller->categorise(imgFilename);
    std::cout << (hasCatOrDog ? "Cat or dog detected!" : "No cat or dog detected.") << std::endl;

    if (hasCatOrDog) {
        comm_gateway->SendAlert();
        // comm_gateway->SendPhoneCall("Cat or dog detected. Open the door.");
    }
}

Controller::~Controller() {
    delete comm_gateway;
}