#include "Application.h"
#include "Logger.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QFile>
#include <exception>

int main(int argc, char* argv[]) {
    try {
        QGuiApplication qapp(argc, argv);
        qapp.setOrganizationName("BLADE");
        qapp.setOrganizationDomain("blade.local");
        qapp.setApplicationName("BLADE File Transfer");

        // Set application icon
        QIcon appIcon(":/blade.ico");
        if (!appIcon.isNull()) {
            qapp.setWindowIcon(appIcon);
        }

        blade::Logger::getInstance().info("=== BLADE Application Starting ===");

        blade::Logger::getInstance().info("Initializing QML engine");

        QQmlApplicationEngine engine;

        blade::ApplicationBridge* appBridge = nullptr;
        try {
            blade::Logger::getInstance().info("Creating ApplicationBridge");
            appBridge = new blade::ApplicationBridge(&engine);

            // Expose bridge to QML
            engine.rootContext()->setContextProperty("applicationBridge", appBridge);

            blade::Logger::getInstance().info("Loading QML interface");
            const QUrl url(QStringLiteral("qrc:/qml/main.qml"));

            QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                             &qapp, [url](QObject *obj, const QUrl &objUrl) {
                if (!obj && url == objUrl) {
                    blade::Logger::getInstance().error("Failed to load QML");
                    QCoreApplication::exit(-1);
                }
            }, Qt::QueuedConnection);

            engine.load(url);

            if (engine.rootObjects().isEmpty()) {
                blade::Logger::getInstance().error("No root objects in QML engine");
                return -1;
            }

            blade::Logger::getInstance().info("Application initialized successfully");

        } catch (const std::exception& e) {
            blade::Logger::getInstance().error(std::string("Exception during initialization: ") + e.what());
            return 3;
        } catch (...) {
            blade::Logger::getInstance().error("Unknown exception during initialization");
            return 4;
        }

        blade::Logger::getInstance().info("Starting event loop");
        int result = qapp.exec();

        blade::Logger::getInstance().info("Application exiting with code: " + std::to_string(result));
        delete appBridge;
        return result;
    } catch (const std::exception& e) {
        blade::Logger::getInstance().error(std::string("Fatal exception: ") + e.what());
        return 1;
    } catch (...) {
        blade::Logger::getInstance().error("Fatal exception: Unknown error");
        return 2;
    }
}



