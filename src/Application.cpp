#include "Application.h"
#include "MainWindow.h"
#include "Logger.h"
#include <QFile>
#include <QIcon>
#include <QMessageBox>
#include <QDir>

namespace blade {

Application::Application(int argc, char* argv[])
    : qapp_(std::make_unique<QApplication>(argc, argv))
    , mainWindow_(nullptr) {

    try {
        QApplication::setOrganizationName("BLADE");
        QApplication::setOrganizationDomain("blade.local");
        QApplication::setApplicationName("BLADE");

        // Set application icon
        if (QIcon appIcon(":/blade.ico"); !appIcon.isNull()) {
            QApplication::setWindowIcon(appIcon);
        }

        Logger::getInstance().info("=== BLADE Application Starting (Qt Widgets) ===");

        // Load stylesheet from current working directory
        if (QFile styleFile(QDir::current().filePath("styles-widgets.qss")); styleFile.open(QFile::ReadOnly)) {
            const QString styleSheet = QLatin1String(styleFile.readAll());
            qapp_->setStyleSheet(styleSheet);
            Logger::getInstance().info("Stylesheet loaded successfully from CWD");
        } else {
            Logger::getInstance().warning("Failed to load stylesheet from CWD");
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Application Error",
            QString("Failed to initialize application: %1").arg(e.what()));
        throw;
    }
}

Application::~Application() = default;

int Application::run() {
    try {
        // Create and show main window
        mainWindow_ = std::make_unique<MainWindow>();
        mainWindow_->show();

        Logger::getInstance().info("Application initialized successfully");
        Logger::getInstance().info("Starting event loop");

        return QApplication::exec();

    } catch (const std::exception& e) {
        Logger::getInstance().error(std::string("Fatal error in run(): ") + e.what());
        QMessageBox::critical(nullptr, "Fatal Error",
            QString("Application error: %1").arg(e.what()));
        return 1;
    }
}

} // namespace blade

