//
// Created by satahaa on 01-Feb-26.
//

#ifndef BLADE_TOAST_H
#define BLADE_TOAST_H
#pragma once
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

class Toast final : public QWidget {
    Q_OBJECT
public:
    static void showText(QWidget* parent, const QString& text, int msec = 2000) {
        if (!parent) return;

        auto* w = new Toast();
        w->setParent(parent);
        w->setAttribute(Qt::WA_StyledBackground, true);
        w->setObjectName("toast");               // style via your external QSS
        w->setWindowFlags(Qt::FramelessWindowHint);
        w->setFixedHeight(42);

        auto* layout = new QHBoxLayout(w);
        layout->setContentsMargins(14, 10, 14, 10);

        auto* label = new QLabel(text, w);
        label->setObjectName("toastText");
        label->setWordWrap(true);
        layout->addWidget(label);

        w->adjustSize();

        // bottom-center
        const int x = (parent->width() - w->width()) / 2;
        const int y = parent->height() - w->height() - 18;
        w->move(qMax(10, x), qMax(10, y));
        w->show();
        w->raise();

        // fade out
        auto* eff = new QGraphicsOpacityEffect(w);
        w->setGraphicsEffect(eff);
        eff->setOpacity(1.0);

        auto* anim = new QPropertyAnimation(eff, "opacity", w);
        anim->setDuration(220);
        anim->setStartValue(1.0);
        anim->setEndValue(0.0);

        QTimer::singleShot(msec, w, [w, anim]() {
            anim->start();
            connect(anim, &QPropertyAnimation::finished, w, &QWidget::deleteLater);
        });
    }
};

#endif //BLADE_TOAST_H