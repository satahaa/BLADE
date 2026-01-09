#ifndef BLADE_SERVERWIDGET_H
#define BLADE_SERVERWIDGET_H

#include <QWidget>
#include <QLabel>

namespace blade {

class ServerWidget : public QWidget {
    Q_OBJECT

public:
    explicit ServerWidget(QWidget* parent = nullptr);

    void setServerUrl(const QString& url);

private:
    QLabel* statusLabel_;
    QLabel* urlLabel_;
    QLabel* qrLabel_;
};

} // namespace blade

#endif // BLADE_SERVERWIDGET_H

