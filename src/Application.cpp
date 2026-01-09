#include "Application.h"
#include "MainWindow.h"
#include "Logger.h"
#include <QFile>
#include <QIcon>

namespace blade {

Application::Application(int argc, char* argv[])
    : qapp_(std::make_unique<QApplication>(argc, argv))
    , mainWindow_(nullptr) {

    QApplication::setOrganizationName("BLADE");
    QApplication::setOrganizationDomain("blade.local");
    QApplication::setApplicationName("BLADE - Bi-Directional LAN Asset Distribution Engine");

    // Set application icon
    if (QIcon appIcon(":/blade.ico"); !appIcon.isNull()) {
        QApplication::setWindowIcon(appIcon);
    }

    Logger::getInstance().info("=== BLADE Application Starting (Qt Widgets) ===");

    // Load stylesheet
    if (QFile styleFile(":/styles-widgets.qss"); styleFile.open(QFile::ReadOnly)) {
        const QString styleSheet = QLatin1String(styleFile.readAll());
        qapp_->setStyleSheet(styleSheet);
        Logger::getInstance().info("Stylesheet loaded successfully");
    } else {
        Logger::getInstance().warning("Failed to load stylesheet");
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
        return 1;
    }
}

} // namespace blade

