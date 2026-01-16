#include "ServerWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFont>
#include <QApplication>
#include <QClipboard>

namespace blade {

ServerWidget::ServerWidget(QWidget* parent) : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 60, 50, 60);
    mainLayout->setSpacing(30);

    // Add stretch at top
    mainLayout->addStretch(1);

    // Status indicator
    auto* statusWidget = new QWidget(this);
    statusWidget->setObjectName("statusIndicator");
    statusWidget->setMinimumHeight(56);
    auto* statusLayout = new QHBoxLayout(statusWidget);

    auto* statusIcon = new QLabel("✓", this);
    statusIcon->setObjectName("statusIcon");
    statusLayout->addWidget(statusIcon);

    statusLabel_ = new QLabel("Server Running", this);
    statusLabel_->setObjectName("statusText");
    statusLayout->addWidget(statusLabel_);
    statusLayout->addStretch();

    mainLayout->addWidget(statusWidget, 0, Qt::AlignCenter);

    mainLayout->addSpacing(20);

    // Title
    auto* titleLabel = new QLabel("Your Server is Live!", this);
    titleLabel->setObjectName("serverTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Subtitle
    auto* subtitleLabel = new QLabel("Share the URL or QR code to transfer files", this);
    subtitleLabel->setObjectName("serverSubtitle");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitleLabel);

    mainLayout->addSpacing(20);

    // URL section
    auto* urlWidget = new QWidget(this);
    urlWidget->setObjectName("urlContainer");
    auto* urlLayout = new QVBoxLayout(urlWidget);
    urlLayout->setSpacing(15);
    urlLayout->setContentsMargins(30, 30, 30, 30);

    urlLabel_ = new QLabel(this);
    urlLabel_->setObjectName("urlText");
    urlLabel_->setAlignment(Qt::AlignCenter);
    urlLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    urlLabel_->setWordWrap(true);
    urlLayout->addWidget(urlLabel_);

    auto* copyButton = new QPushButton("Copy URL", this);
    copyButton->setObjectName("copyButton");
    copyButton->setMinimumHeight(48);
    copyButton->setCursor(Qt::PointingHandCursor);
    urlLayout->addWidget(copyButton);

    mainLayout->addWidget(urlWidget);

    mainLayout->addSpacing(20);

    // QR Code section
    auto* qrWidget = new QWidget(this);
    qrWidget->setObjectName("qrContainer");
    auto* qrLayout = new QVBoxLayout(qrWidget);
    qrLayout->setSpacing(15);
    qrLayout->setContentsMargins(30, 30, 30, 30);

    qrLabel_ = new QLabel(this);
    qrLabel_->setObjectName("qrCode");
    qrLabel_->setAlignment(Qt::AlignCenter);
    qrLabel_->setMinimumSize(300, 300);
    qrLabel_->setText("QR Code will be displayed here");
    qrLayout->addWidget(qrLabel_, 0, Qt::AlignCenter);

    mainLayout->addWidget(qrWidget);

    // Add stretch at bottom
    mainLayout->addStretch(1);

    // Instructions
    auto* instructionsLabel = new QLabel("Scan the QR code or enter the URL in your browser", this);
    instructionsLabel->setObjectName("instructions");
    instructionsLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(instructionsLabel);

    // Copy button functionality
    connect(copyButton, &QPushButton::clicked, this, [this]() {
        QApplication::clipboard()->setText(urlLabel_->text());
    });
}

void ServerWidget::setServerUrl(const QString& url) const {
    urlLabel_->setText(url);
}

} // namespace blade

