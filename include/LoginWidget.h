#ifndef BLADE_LOGINWIDGET_H
#define BLADE_LOGINWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QDir>
#include <QFileDialog>

namespace blade {

class LoginWidget : public QWidget {
    Q_OBJECT

public:
    explicit LoginWidget(QWidget* parent = nullptr);
    [[nodiscard]] QString getDownloadPath() const { return downloadPath_; }

signals:
    void startWithAuth(const QString& username, const QString& password);
    void startNoAuth();
    void downloadPathChanged(const QString& path);

private:
    QComboBox* authModeCombo_;
    QLineEdit* usernameEdit_;
    QLineEdit* passwordEdit_;
    QPushButton* startButton_;

    QLabel* downloadPathLabel_;
    QPushButton* selectPathButton_;
    QString downloadPath_;
    QSettings settings_;
};

} // namespace blade

#endif // BLADE_LOGINWIDGET_H

