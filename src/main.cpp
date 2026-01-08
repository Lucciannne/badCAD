#include "ui/application.h"
#include <iostream>

int main(int argc, char** argv) {
    std::cout << "badCAD - 3D CAD Application" << std::endl;
    std::cout << "=============================" << std::endl;

    badcad::Application app;

    if (!app.init()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return -1;
    }

    app.run();
    app.shutdown();

    std::cout << "\nApplication closed successfully" << std::endl;
    return 0;
}
