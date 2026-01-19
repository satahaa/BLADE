#include "ServerWidget.h"
#include "QRCodeGen.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QFont>
#include <QApplication>
#include <QClipboard>
#include <QImage>
#include <QPixmap>

namespace blade {

// Helper function to generate a QR code QImage from a string
static QImage generateQrCodeImage(const QString& data, int size = 300) {
    using qrcodegen::QrCode;
    using qrcodegen::QrSegment;
    QrCode qr = QrCode::encodeText(data.toUtf8().constData(), QrCode::Ecc::LOW);
    int qrSize = qr.getSize();
    QImage image(qrSize, qrSize, QImage::Format_Mono);
    image.fill(1);
    for (int y = 0; y < qrSize; ++y) {
        for (int x = 0; x < qrSize; ++x) {
            image.setPixel(x, y, qr.getModule(x, y) ? 0 : 1);
        }
    }
    QImage scaled = image.scaled(size, size, Qt::KeepAspectRatio, Qt::FastTransformation);
    return scaled;
}

ServerWidget::ServerWidget(QWidget* parent) : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 60, 50, 60);
    mainLayout->setSpacing(30);

    // Add stretch at top
    mainLayout->addStretch(1);

    // QR Code section (now at the top)
    auto* qrWidget = new QWidget(this);
    qrWidget->setObjectName("qrContainer");
    auto* qrLayout = new QHBoxLayout(qrWidget);
    qrLayout->setSpacing(0);
    qrLayout->setContentsMargins(0, 0, 0, 0); // Remove margins for max space
    qrLabel_ = new QLabel(this);
    qrLabel_->setObjectName("qrCode");
    qrLabel_->setAlignment(Qt::AlignCenter);
    qrLabel_->setMinimumSize(400, 400); // Large minimum size
    qrLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    qrLabel_->setText("QR Code will be displayed here");
    qrLayout->addWidget(qrLabel_, 1, Qt::AlignVCenter | Qt::AlignHCenter);
    mainLayout->addWidget(qrWidget, 0, Qt::AlignTop);

    // Server Online label (bigger)
    auto* serverOnlineLabel = new QLabel("Server Online", this);
    serverOnlineLabel->setObjectName("serverOnlineLabel");
    serverOnlineLabel->setAlignment(Qt::AlignCenter);
    QFont serverOnlineFont;
    serverOnlineFont.setPointSize(32); // Bigger font
    serverOnlineFont.setBold(true);
    serverOnlineLabel->setFont(serverOnlineFont);
    mainLayout->addWidget(serverOnlineLabel);

    // IP Address/URL label (reuse urlLabel_)
    urlLabel_ = new QLabel(this);
    urlLabel_->setObjectName("urlText");
    urlLabel_->setAlignment(Qt::AlignCenter);
    urlLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    urlLabel_->setWordWrap(true);
    QFont urlFont;
    urlFont.setPointSize(18);
    urlLabel_->setFont(urlFont);
    mainLayout->addWidget(urlLabel_);

    // Add stretch at bottom
    mainLayout->addStretch(1);

    // Instructions
    auto* instructionsLabel = new QLabel("Scan the QR code or enter the URL in your browser", this);
    instructionsLabel->setObjectName("instructions");
    instructionsLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(instructionsLabel);
}

void ServerWidget::setServerUrl(const QString& url) const {
    urlLabel_->setText(url);
    // Use the available label size for the QR code, fallback to 400
    int qrSize = std::min(qrLabel_->width(), qrLabel_->height());
    if (qrSize < 100) qrSize = 400;
    QImage qrImg = generateQrCodeImage(url, qrSize);
    if (!qrImg.isNull()) {
        qrLabel_->setPixmap(QPixmap::fromImage(qrImg).scaled(qrLabel_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        qrLabel_->setText("");
    } else {
        qrLabel_->setText("Failed to generate QR code");
    }
}

} // namespace blade

