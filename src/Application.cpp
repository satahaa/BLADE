#include "Application.h"
#include "NetworkUtils.h"
#include "QRCodeGen.h"
#include "Logger.h"
#include <QQmlContext>
#include <QQuickItem>
#include <QPainter>
#include <QDir>
#include <QUrl>
#include <QQuickWindow>
#include <QTimer>

namespace blade {

ApplicationBridge::ApplicationBridge(QQmlApplicationEngine* engine, QObject* parent)
    : QObject(parent)
    , engine_(engine)
    , mainWindow_(nullptr)
    , server_(nullptr) {

    Logger::getInstance().info("ApplicationBridge initialized");
}

ApplicationBridge::~ApplicationBridge() {
    Logger::getInstance().info("Shutting down BLADE Application");
    if (server_) {
        server_->stop();
    }
}

QImage ApplicationBridge::generateQRCode(const std::string& url) {
    try {
        Logger::getInstance().debug("Generating QR code for URL: " + url);
        qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(url.c_str(), qrcodegen::QrCode::Ecc::MEDIUM);

        int size = qr.getSize();
        int border = 4;
        int scale = 8;
        int totalSize = (size + border * 2) * scale;

        QImage image(totalSize, totalSize, QImage::Format_RGB32);
        image.fill(Qt::white);

        QPainter painter(&image);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);

        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                if (qr.getModule(x, y)) {
                    painter.drawRect((x + border) * scale, (y + border) * scale, scale, scale);
                }
            }
        }

        Logger::getInstance().debug("QR code generated successfully");
        return image;
    } catch (const std::exception& e) {
        Logger::getInstance().error("Failed to generate QR code: " + std::string(e.what()));
        QImage error(300, 300, QImage::Format_RGB32);
        error.fill(Qt::white);
        QPainter painter(&error);
        painter.drawText(error.rect(), Qt::AlignCenter, "QR Error");
        return error;
    } catch (...) {
        Logger::getInstance().error("Failed to generate QR code: Unknown error");
        QImage error(300, 300, QImage::Format_RGB32);
        error.fill(Qt::white);
        QPainter painter(&error);
        painter.drawText(error.rect(), Qt::AlignCenter, "QR Error");
        return error;
    }
}

void ApplicationBridge::showServerView(const std::string& ip) {
    Logger::getInstance().info("Showing server view for IP: " + ip);

    if (!mainWindow_) {
        if (engine_->rootObjects().isEmpty()) {
            Logger::getInstance().error("No root objects in engine");
            return;
        }
        QObject* rootObject = engine_->rootObjects().first();
        mainWindow_ = qobject_cast<QQuickWindow*>(rootObject);
    }

    if (!mainWindow_) {
        Logger::getInstance().error("Failed to get main window");
        return;
    }

    std::string url = "http://" + ip;
    Logger::getInstance().debug("Generating QR code for URL: " + url);

    QImage qrImage = generateQRCode(url);

    // Save QR code to temporary file
    QString tempPath = QDir::temp().filePath("blade_qr.png");
    if (qrImage.save(tempPath)) {
        Logger::getInstance().debug("QR code saved to: " + tempPath.toStdString());
    } else {
        Logger::getInstance().error("Failed to save QR code");
    }

    // Call QML function to show server view
    bool invoked = QMetaObject::invokeMethod(mainWindow_, "showServerView",
                              Q_ARG(QVariant, QString::fromStdString(url)));

    if (!invoked) {
        Logger::getInstance().error("Failed to invoke showServerView");
        return;
    }

    Logger::getInstance().debug("Server view invoked successfully");

    // Set QR image after a short delay to ensure view is loaded
    QTimer::singleShot(100, [this, tempPath]() {
        if (mainWindow_) {
            QMetaObject::invokeMethod(mainWindow_, "setQRImage",
                                    Q_ARG(QVariant, tempPath));
            Logger::getInstance().debug("QR image set successfully");
        }
    });
}

void ApplicationBridge::startWithAuth(const QString& username, const QString& password) {
    Logger::getInstance().info("Starting server with authentication");

    QString user = username.trimmed();
    QString pass = password.trimmed();

    if (user.isEmpty() || pass.isEmpty()) {
        Logger::getInstance().warning("Authentication mode requires username and password");
        // TODO: Show QML dialog instead
        return;
    }

    Logger::getInstance().debug("Username: " + user.toStdString());

    if (!NetworkUtils::initialize()) {
        Logger::getInstance().error("Failed to initialize network");
        // TODO: Show QML dialog instead
        return;
    }

    server_ = std::make_unique<blade::Server>(8080, true, user.toStdString(), pass.toStdString());

    if (!server_->start()) {
        Logger::getInstance().error("Failed to start server");
        // TODO: Show QML dialog instead
        server_.reset();
        return;
    }

    std::string ip = NetworkUtils::getLocalIPAddress();
    Logger::getInstance().info("Server started successfully on IP: " + ip);
    showServerView(ip);
}

void ApplicationBridge::startNoAuth() {
    Logger::getInstance().info("Starting server without authentication");

    if (!NetworkUtils::initialize()) {
        Logger::getInstance().error("Failed to initialize network");
        // TODO: Show QML dialog instead
        return;
    }

    server_ = std::make_unique<blade::Server>(8080, false, "", "");

    if (!server_->start()) {
        Logger::getInstance().error("Failed to start server");
        // TODO: Show QML dialog instead
        server_.reset();
        return;
    }

    std::string ip = NetworkUtils::getLocalIPAddress();
    Logger::getInstance().info("Server started successfully on IP: " + ip);
    showServerView(ip);
}

} // namespace blade

