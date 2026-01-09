#include "Application.h"
#include "Logger.h"

int main(const int argc, char* argv[]) {
    try {
        blade::Application app(argc, argv);
        return app.run();
    } catch (const std::exception& e) {
        blade::Logger::getInstance().error(std::string("Fatal error in main: ") + e.what());
        return 1;
    }
}

