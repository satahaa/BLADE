#ifndef BLADE_MAINWINDOW_H
#define BLADE_MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <memory>
#include "LoginWidget.h"
#include "ServerWidget.h"
#include "Server.h"
#include "TitleBar.h"

namespace blade {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onStartWithAuth(const QString& password);
    void onStartNoAuth();
    void onSendFilesRequested(const QStringList& files) const;

    static void forceClose();

private:
    void showServerView(const QString& url) const;
    void showError(const QString& message);
    bool startServer(bool withAuth, const QString& password = "");

    QStackedWidget* stackWidget_;
    LoginWidget* loginWidget_;
    ServerWidget* serverWidget_;
    std::unique_ptr<Server> server_;
};

} // namespace blade

#endif // BLADE_MAINWINDOW_H

