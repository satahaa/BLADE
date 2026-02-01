#include "MainWindow.h"
#include "NetworkUtils.h"
#include "Logger.h"
#include "Toast.h"
#include <QMessageBox>
#include <QApplication>
#include <QTimer>
#include <QCloseEvent>
#include <QThread>

namespace blade {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , server_(nullptr) {

    setWindowTitle("BLADE");
    setFixedSize(900, 900);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);

    setStyleSheet("QMainWindow { border-radius: 32px; }");

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setCentralWidget(central);

    auto* titleBar = new TitleBar(this);
    layout->addWidget(titleBar);

    stackWidget_ = new QStackedWidget(this);
    layout->addWidget(stackWidget_);
    stackWidget_->setObjectName("stackWidget");
    stackWidget_->setAutoFillBackground(false);
    stackWidget_->setAttribute(Qt::WA_StyledBackground, false);
    stackWidget_->setStyleSheet("#stackWidget { background: transparent; border: none; }");

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
    connect(serverWidget_, &ServerWidget::backRequested, this, [this]() {
        if (server_) {
            server_->stop();
            server_.reset();
        }
        stackWidget_->setCurrentWidget(loginWidget_);
    });
}

void MainWindow::onStartWithAuth(const QString& password) {
    Logger::getInstance().info("Starting server with authentication");

    if (password.trimmed().isEmpty()) {
        showError("Authentication mode requires a password.");
        return;
    }

    if (startServer(true, password)) {
        const std::string ip = NetworkUtils::getLocalIPAddress();
        const QString url = QString::fromStdString("http://" + ip);
        showServerView(url);
    }
}

void MainWindow::onStartNoAuth() {
    Logger::getInstance().info("Starting server without authentication");

    if (startServer(false)) {
        const std::string ip = NetworkUtils::getLocalIPAddress();
        const QString url = QString::fromStdString("http://" + ip);
        showServerView(url);
    }
}

bool MainWindow::startServer(const bool withAuth, const QString& password) {
    if (!NetworkUtils::initialize()) {
        Logger::getInstance().error("Failed to initialize network");
        showError("Failed to initialize network. Please check your connection.");
        return false;
    }

    try {
        if (withAuth) {
            server_ = std::make_unique<Server>(8080, true, password.toStdString());
        } else {
            server_ = std::make_unique<Server>(8080, false, "");
        }

        if (!server_->start()) {
            Logger::getInstance().error("Failed to start server");
            showError("Failed to start the server. Please check the logs for more details.");
            server_.reset();
            return false;
        }

        const std::string ip = NetworkUtils::getLocalIPAddress();
        Logger::getInstance().info("Server started successfully on IP: " + ip);
        if (server_) {
            server_->setDownloadDirectory(loginWidget_->getDownloadPath().toStdString());
            Logger::getInstance().debug("Connecting downloadPathChanged signal...");
            connect(loginWidget_, &LoginWidget::downloadPathChanged, this, [this](const QString& path) {
                if (server_) server_->setDownloadDirectory(path.toStdString());
            });
            Logger::getInstance().debug("downloadPathChanged signal connected");

            Logger::getInstance().debug("Connecting sendFilesRequested signal...");
            connect(serverWidget_, &ServerWidget::sendFilesRequested,
                    this, &MainWindow::onSendFilesRequested);
            Logger::getInstance().debug("sendFilesRequested signal connected");

            Logger::getInstance().debug("Setting outgoing progress callback...");
            server_->setOutgoingProgressCallback([w = serverWidget_](const std::string& path, int pct) {
                const QString qPath = QString::fromStdString(path);

                QMetaObject::invokeMethod(w, [w, qPath, pct]() {
                    w->setOutgoingProgress(qPath, pct);
                }, Qt::QueuedConnection);
            });
            Logger::getInstance().debug("Outgoing progress callback set");

            Logger::getInstance().debug("Setting incoming progress callback...");
            server_->setIncomingProgressCallback([w = serverWidget_](const std::string& filename, int pct) {
                const QString qFilename = QString::fromStdString(filename);

                QMetaObject::invokeMethod(w, [w, qFilename, pct]() {
                    w->setReceivedProgress(qFilename, pct);
                }, Qt::QueuedConnection);
            });
            Logger::getInstance().debug("Incoming progress callback set");

            Logger::getInstance().debug("Setting incoming file callback...");
            server_->setIncomingFileCallback([w = serverWidget_](const std::string& filename, uint64_t fileSize) {
                const QString qFilename = QString::fromStdString(filename);

                QMetaObject::invokeMethod(w, [w, qFilename, fileSize]() {
                    w->setReceivedFile(qFilename, fileSize);
                }, Qt::QueuedConnection);
            });
            Logger::getInstance().debug("Incoming file callback set");
            Logger::getInstance().debug("Outgoing progress callback set");

        }
        Logger::getInstance().debug("startServer returning true");
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

void MainWindow::forceClose() {
    Logger::getInstance().info("[CLOSE] forceClose() called, exiting application");
    // This will force the application to exit immediately after server shutdown
    QApplication::exit(0);
}

void MainWindow::onSendFilesRequested(const QStringList& files) const {
    Logger::getInstance().info("onSendFilesRequested slot called with " + std::to_string(files.size()) + " files");

    if (!server_) {
        Logger::getInstance().error("onSendFilesRequested: server_ is null");
        return;
    }

    // Check if any client is connected
    const bool hasClients = server_->hasConnectedClients();
    Logger::getInstance().debug("hasConnectedClients() returned: " + std::string(hasClients ? "true" : "false"));

    if (!hasClients) {
        Toast::showText(serverWidget_, "No device connected", 2500);
        Logger::getInstance().warning("Send files requested but no client connected");
        return;
    }

    std::vector<std::string> paths;
    paths.reserve(files.size());
    for (const auto& f : files) {
        paths.push_back(f.toStdString());
        Logger::getInstance().debug("Queueing file: " + f.toStdString());
    }

    server_->sendFilesToClient(paths);
    Logger::getInstance().info("Files sent to sendFilesToClient()");
}

void MainWindow::closeEvent(QCloseEvent* event) {
    Logger::getInstance().info("[CLOSE] MainWindow::closeEvent called");
    if (server_) {
        Logger::getInstance().info("[CLOSE] Server is running, calling stop()");
        server_->stop();
        Logger::getInstance().info("[CLOSE] Server::stop() returned");
        server_.reset();
        Logger::getInstance().info("[CLOSE] server_.reset() done");
    }
    Logger::getInstance().info("[CLOSE] Accepting close event");
    event->accept();
    QMainWindow::closeEvent(event);
}

} // namespace blade

