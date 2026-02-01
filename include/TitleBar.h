//
// Created by satahaa on 22-Jan-26.
//

#ifndef BLADE_TITLEBAR_H
#define BLADE_TITLEBAR_H
#pragma once
#include <QWidget>
#include <QToolButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QIcon>
#include <QMainWindow>

class TitleBar : public QWidget {
    Q_OBJECT
public:
    explicit TitleBar(QWidget* parent = nullptr) : QWidget(parent) {
        setFixedHeight(40);
        setStyleSheet("background: #23272e;");

        auto* layout = new QHBoxLayout(this);
        layout->setContentsMargins(8, 0, 8, 0);
        layout->addStretch();
        layout->setSpacing(6);

        auto* minimizeBtn = new QToolButton(this);
        minimizeBtn->setFixedSize(40, 40); // Bigger button
        minimizeBtn->setIconSize(QSize(20, 20));
        minimizeBtn->setIcon(QIcon(":icons/minimize.svg"));
        minimizeBtn->setToolTip("Minimize");
        minimizeBtn->setStyleSheet(minimizeBtnStyle);
        connect(minimizeBtn, &QToolButton::clicked, this, [this] {
            if (auto* mw = qobject_cast<QMainWindow*>(window())) mw->showMinimized();
        });

        auto* closeBtn = new QToolButton(this);
        closeBtn->setFixedSize(40, 40);
        closeBtn->setIconSize(QSize(20, 20));
        closeBtn->setIcon(QIcon(":icons/close.svg"));
        closeBtn->setToolTip("Close");
        closeBtn->setStyleSheet(closeBtnStyle);
        connect(closeBtn, &QToolButton::clicked, this, [this] {
            if (auto* mw = qobject_cast<QMainWindow*>(window())) mw->close();
        });

        layout->addWidget(minimizeBtn);
        layout->addWidget(closeBtn);
    }

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            m_dragPos = event->globalPosition().toPoint();
            m_dragging = true;
        }
    }
    void mouseMoveEvent(QMouseEvent* event) override {
        if (m_dragging) {
            if (QWidget* win = window()) {
                const QPoint diff = event->globalPosition().toPoint() - m_dragPos;
                win->move(win->pos() + diff);
                m_dragPos = event->globalPosition().toPoint();
            }
        }
    }
    void mouseReleaseEvent(QMouseEvent*) override { m_dragging = false; }
private:
    QString closeBtnStyle =  "QToolButton { border: none; background: transparent; padding-top: 12px;}"
                        "QToolButton:hover { background:#5c2828; padding-top: 12px; }"
                        "QToolButton:pressed { background: transparent; padding-top: 12px; }";
    QString minimizeBtnStyle =  "QToolButton { border: none; background: transparent; padding-bottom: 12px; }"
                        "QToolButton:hover { background: #3d4552; padding-bottom: 12px; }"
                        "QToolButton:pressed { background: transparent; padding-bottom: 12px; }";
    QPoint m_dragPos;
    bool m_dragging = false;
};
#endif //BLADE_TITLEBAR_H