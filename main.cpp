#include <iostream>
#include "controller.h"

// To compile: g++ main.cpp controller.cpp communication_gateway.cpp -o rasp_pet $(pkg-config --cflags --libs libcamera libcurl)
// ./rasp_pet

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

int main() {
    Controller controller;
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
                controller.checkIfAlive();
                break;
            case 2:
                controller.startCameraLoop();
                break;
            case 3:
                controller.stopCameraLoop();
                break;
            case 4:
                controller.testSMS();
                break;
            case 5:
                controller.testPhoneCall();
                break;
            case 6:
                std::cout << "Exiting program.\n";
                return 0;
            default:
                std::cout << "Invalid choice. Please select a number between 1 and 6.\n";
        }
    }

    return 0;
}