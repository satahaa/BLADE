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


namespace blade {

class LoginWidget : public QWidget {
    Q_OBJECT

public:
    explicit LoginWidget(QWidget* parent = nullptr);

signals:
    void startWithAuth(const QString& username, const QString& password);
    void startNoAuth();

private:
    QComboBox* authModeCombo_;
    QLineEdit* usernameEdit_;
    QLineEdit* passwordEdit_;
    QPushButton* startButton_;
};

} // namespace blade

#endif // BLADE_LOGINWIDGET_H

