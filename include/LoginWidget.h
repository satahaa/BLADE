#ifndef BLADE_LOGINWIDGET_H
#define BLADE_LOGINWIDGET_H

#include <QComboBox>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QFileDialog>
#include <QCheckBox>

namespace blade {

class LoginWidget : public QWidget {
    Q_OBJECT

public:
    explicit LoginWidget(QWidget* parent = nullptr);
    [[nodiscard]] QString getDownloadPath() const { return downloadPath_; }

signals:
    void startWithAuth(const QString& password);
    void startNoAuth();
    void downloadPathChanged(const QString& path);

private:
    QPushButton* authToggleButton_;
    QLineEdit* passwordEdit_;
    QPushButton* startButton_;


    QToolButton *authToggleSwitch_ = nullptr;
    QToolButton* passwordVisButton_ = nullptr;
    QLabel* headerIcon_ = nullptr;
    QToolButton* authToggleIcon_ = nullptr;

    QLabel* downloadPathLabel_;
    QPushButton* selectPathButton_;
    QString downloadPath_;
    QSettings settings_;
};

} // namespace blade

#endif // BLADE_LOGINWIDGET_H

