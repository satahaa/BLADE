#ifndef BLADE_MAINWINDOW_H
#define BLADE_MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <memory>
#include "LoginWidget.h"
#include "ServerWidget.h"
#include "Server.h"

namespace blade {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onStartWithAuth(const QString& username, const QString& password);
    void onStartNoAuth();

private:
    void showServerView(const QString& url);
    void showError(const QString& message);
    bool startServer(bool withAuth, const QString& username = "", const QString& password = "");

    QStackedWidget* stackWidget_;
    LoginWidget* loginWidget_;
    ServerWidget* serverWidget_;
    std::unique_ptr<blade::Server> server_;
};

} // namespace blade

#endif // BLADE_MAINWINDOW_H

