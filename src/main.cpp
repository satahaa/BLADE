#include "Application.h"
#include "Logger.h"
#include <QMessageBox>
#include <QApplication>

int main(int argc, char* argv[]) {
    try {
        blade::Application app(argc, argv);
        return app.run();
    } catch (const std::exception& e) {
        blade::Logger::getInstance().error(std::string("Fatal error: ") + e.what());
        // Create a minimal QApplication just for the error dialog
        if (!QApplication::instance()) {
            QApplication tempApp(argc, argv);
            QMessageBox::critical(nullptr, "Fatal Error",
                QString("Failed to start application: %1").arg(e.what()));
        } else {
            QMessageBox::critical(nullptr, "Fatal Error",
                QString("Failed to start application: %1").arg(e.what()));
        }
        return 1;
    }
}

