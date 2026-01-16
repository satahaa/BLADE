#include "MainWindow.h"
#include "NetworkUtils.h"
#include "Logger.h"
#include <QMessageBox>
#include <QApplication>

namespace blade {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , server_(nullptr) {

    setWindowTitle("BLADE - Bi-Directional LAN Asset Distribution Engine");
    setFixedSize(800, 900);

    // Create stacked widget for view switching
    stackWidget_ = new QStackedWidget(this);
    setCentralWidget(stackWidget_);

    // Create login widget
    loginWidget_ = new LoginWidget(this);
    connect(loginWidget_, &LoginWidget::startWithAuth, this, &MainWindow::onStartWithAuth);
    connect(loginWidget_, &LoginWidget::startNoAuth, this, &MainWindow::onStartNoAuth);
    stackWidget_->addWidget(loginWidget_);

    // Create server widget
    serverWidget_ = new ServerWidget(this);
    stackWidget_->addWidget(serverWidget_);

    // Show login view initially
    stackWidget_->setCurrentWidget(loginWidget_);

    Logger::getInstance().info("MainWindow initialized");
}

void MainWindow::onStartWithAuth(const QString& username, const QString& password) {
    Logger::getInstance().info("Starting server with authentication");

    if (username.trimmed().isEmpty() || password.trimmed().isEmpty()) {
        showError("Authentication mode requires both username and password.");
        return;
    }

    if (startServer(true, username, password)) {
        std::string ip = NetworkUtils::getLocalIPAddress();
        QString url = QString::fromStdString("http://" + ip);
        showServerView(url);
    }
}

void MainWindow::onStartNoAuth() {
    Logger::getInstance().info("Starting server without authentication");

    if (startServer(false)) {
        std::string ip = NetworkUtils::getLocalIPAddress();
        QString url = QString::fromStdString("http://" + ip);
        showServerView(url);
    }
}

bool MainWindow::startServer(bool withAuth, const QString& username, const QString& password) {
    if (!NetworkUtils::initialize()) {
        Logger::getInstance().error("Failed to initialize network");
        showError("Failed to initialize network. Please check your connection.");
        return false;
    }

    try {
        if (withAuth) {
            server_ = std::make_unique<Server>(8080, true,
                                                      username.toStdString(),
                                                      password.toStdString());
        } else {
            server_ = std::make_unique<Server>(8080, false, "", "");
        }

        if (!server_->start()) {
            Logger::getInstance().error("Failed to start server");
            showError("Failed to start the server. Please check the logs for more details.");
            server_.reset();
            return false;
        }

        std::string ip = NetworkUtils::getLocalIPAddress();
        Logger::getInstance().info("Server started successfully on IP: " + ip);
        return true;

    } catch (const std::exception& e) {
        Logger::getInstance().error("Exception starting server: " + std::string(e.what()));
        showError("Error starting server: " + QString::fromStdString(e.what()));
        return false;
    }
}

void MainWindow::showServerView(const QString& url) const {
    Logger::getInstance().info("Showing server view");
    serverWidget_->setServerUrl(url);
    stackWidget_->setCurrentWidget(serverWidget_);
}

void MainWindow::showError(const QString& message) {
    Logger::getInstance().warning("Showing error: " + message.toStdString());
    QMessageBox::warning(this, "BLADE - Error", message);
}

} // namespace blade

