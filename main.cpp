#include <iostream>
#include "controller.h"

// To compile: 
// g++ main.cpp controller.cpp camera_controller.cpp nn_controller.cpp communication_gateway.cpp -o rasp_pet $(pkg-config --cflags --libs libcamera libcurl opencv4)
// ./rasp_pet

int main() {
    Controller controller;
    controller.ConsoleMenuHandler();

    return 0;
}