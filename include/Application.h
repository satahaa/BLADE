#ifndef BLADE_APPLICATION_H
#define BLADE_APPLICATION_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QImage>
#include <memory>
#include "Server.h"

namespace blade {

class ApplicationBridge : public QObject {
    Q_OBJECT

public:
    explicit ApplicationBridge(QQmlApplicationEngine* engine, QObject* parent = nullptr);
    ~ApplicationBridge() override;

    Q_INVOKABLE void startWithAuth(const QString& username, const QString& password);
    Q_INVOKABLE void startNoAuth();

private:
    void showServerView(const std::string& ip);
    QImage generateQRCode(const std::string& url);

    QQmlApplicationEngine* engine_;
    QQuickWindow* mainWindow_;
    std::unique_ptr<blade::Server> server_;
};

} // namespace blade

#endif // BLADE_APPLICATION_H

