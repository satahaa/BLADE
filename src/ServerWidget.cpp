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

// ---------- Helper: QR generator ----------
static QImage generateQrCodeImage(const QString& data, int size = 280) {
    using qrcodegen::QrCode;
    const QrCode qr = QrCode::encodeText(data.toUtf8().constData(), QrCode::Ecc::LOW);
    const int qrSize = qr.getSize();

    QImage image(qrSize, qrSize, QImage::Format_Mono);
    image.fill(1);
    for (int y = 0; y < qrSize; ++y) {
        for (int x = 0; x < qrSize; ++x) {
            image.setPixel(x, y, qr.getModule(x, y) ? 0 : 1);
        }
    }
    return image.scaled(size, size, Qt::KeepAspectRatio, Qt::FastTransformation);
}

// ---------- Helper widget: file row (name + progress) ----------
class FileProgressRow final : public QWidget {
public:
    explicit FileProgressRow(const QString& title, QWidget* parent = nullptr)
        : QWidget(parent) {

        setObjectName("fileItemRow");

        auto* root = new QVBoxLayout(this);
        root->setContentsMargins(14, 12, 14, 12);
        root->setSpacing(8);

        name_ = new QLabel(title, this);
        name_->setObjectName("fileName");
        name_->setWordWrap(true);

        bar_ = new QProgressBar(this);
        bar_->setObjectName("fileProgress");
        bar_->setRange(0, 100);
        bar_->setValue(0);
        bar_->setTextVisible(false);
        bar_->setFixedHeight(8);

        root->addWidget(name_);
        root->addWidget(bar_);
    }

    void setProgress(const int pct) const {
        bar_->setValue(std::clamp(pct, 0, 100));
    }

private:
    QLabel* name_ = nullptr;
    QProgressBar* bar_ = nullptr;
};

// ---------- Helper: section card with title ----------
static QWidget* makeSectionCard(const QString& title, QWidget* parent, QWidget** bodyOut = nullptr) {
    auto* card = new QWidget(parent);
    card->setProperty("surface", "card");  // for your QSS
    card->setObjectName("cardContainer");

    auto* v = new QVBoxLayout(card);
    v->setContentsMargins(22, 20, 22, 20);
    v->setSpacing(14);

    auto* t = new QLabel(title, card);
    t->setProperty("role", "sectionTitle");
    t->setObjectName("sectionTitle");
    v->addWidget(t);

    auto* body = new QWidget(card);
    body->setObjectName("sectionBody");
    auto* bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(10);

    v->addWidget(body);

    if (bodyOut) *bodyOut = body;
    return card;
}

ServerWidget::ServerWidget(QWidget* parent) : QWidget(parent) {
    setObjectName("dashboardRoot");
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);

    // ================= Root layout =================
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(56, 44, 56, 44);
    mainLayout->setSpacing(18);

    // ================= Top bar: Back + Online =================
    auto* topBar = new QWidget(this);
    topBar->setObjectName("topBar");
    auto* topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    topBarLayout->setSpacing(12);

    backButton_ = new QPushButton("Back", topBar);
    backButton_->setObjectName("secondaryButton");
    backButton_->setCursor(Qt::PointingHandCursor);

    onlineLabel_ = new QLabel("Online", topBar);
    onlineLabel_->setObjectName("onlineLabel");
    onlineLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // Big + green
    QFont onlineFont;
    onlineFont.setPointSize(24);
    onlineFont.setBold(true);
    onlineLabel_->setFont(onlineFont);
    onlineLabel_->setStyleSheet("color: #10b981;"); // match your palette

    topBarLayout->addWidget(backButton_, 0, Qt::AlignLeft);
    topBarLayout->addWidget(onlineLabel_, 0, Qt::AlignLeft);
    topBarLayout->addStretch(1);

    mainLayout->addWidget(topBar);

    // ================= Top content row: QR left, URL right =================
    auto* topRow = new QWidget(this);
    topRow->setObjectName("topRow");
    auto* topRowLayout = new QHBoxLayout(topRow);
    topRowLayout->setContentsMargins(0, 0, 0, 0);
    topRowLayout->setSpacing(18);

    // ---- QR Card ----
    QWidget* qrBody = nullptr;
    auto* qrCard = makeSectionCard("Pairing QR", this, &qrBody);
    qrCard->setObjectName("qrContainer");

    auto* qrBodyLayout = qobject_cast<QVBoxLayout*>(qrBody->layout());
    qrLabel_ = new QLabel(qrBody);
    qrLabel_->setObjectName("qrCode");
    qrLabel_->setAlignment(Qt::AlignCenter);
    qrLabel_->setMinimumSize(300, 300);
    qrLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    qrLabel_->setText("QR will appear here");

    qrBodyLayout->addWidget(qrLabel_);

    // ---- URL Card ----
    QWidget* urlBody = nullptr;
    auto* urlCard = makeSectionCard("Browser URL", this, &urlBody);
    urlCard->setObjectName("urlContainer");

    auto* urlBodyLayout = qobject_cast<QVBoxLayout*>(urlBody->layout());

    auto* urlRow = new QWidget(urlBody);
    urlRow->setObjectName("urlRow");
    urlRow->setProperty("role", "row");
    auto* urlRowLayout = new QHBoxLayout(urlRow);
    urlRowLayout->setContentsMargins(14, 10, 14, 10);
    urlRowLayout->setSpacing(10);

    urlLabel_ = new QLabel(urlRow);
    urlLabel_->setObjectName("urlText");
    urlLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    urlLabel_->setWordWrap(true);
    urlLabel_->setText("http://0.0.0.0:0000");

    copyUrlButton_ = new QPushButton("Copy", urlRow);
    copyUrlButton_->setObjectName("secondaryButton");
    copyUrlButton_->setCursor(Qt::PointingHandCursor);

    urlRowLayout->addWidget(urlLabel_, 1);
    urlRowLayout->addWidget(copyUrlButton_, 0);

    urlBodyLayout->addWidget(urlRow);

    auto* hint = new QLabel("Scan QR or open the URL to connect a device.", urlBody);
    hint->setProperty("role", "hint");
    hint->setObjectName("instructions");
    urlBodyLayout->addWidget(hint);

    // Add cards to top row
    topRowLayout->addWidget(qrCard, 1);
    topRowLayout->addWidget(urlCard, 1);

    mainLayout->addWidget(topRow);

    // ================= Actions row: Select files + Send =================
    auto* actionsRow = new QWidget(this);
    actionsRow->setObjectName("actionsRow");
    auto* actionsLayout = new QHBoxLayout(actionsRow);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(12);

    selectFilesButton_ = new QPushButton("Select Files", actionsRow);
    selectFilesButton_->setObjectName("secondaryButton");
    selectFilesButton_->setCursor(Qt::PointingHandCursor);

    sendButton_ = new QPushButton("Send", actionsRow);
    sendButton_->setObjectName("primaryButton");
    sendButton_->setCursor(Qt::PointingHandCursor);
    sendButton_->setEnabled(false);

    actionsLayout->addWidget(selectFilesButton_, 0, Qt::AlignLeft);
    actionsLayout->addStretch(1);
    actionsLayout->addWidget(sendButton_, 0, Qt::AlignRight);

    mainLayout->addWidget(actionsRow);

    // ================= Bottom row: Selected / Received sections =================
    auto* bottomRow = new QWidget(this);
    bottomRow->setObjectName("bottomRow");
    auto* bottomLayout = new QHBoxLayout(bottomRow);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(18);

    // ---- Selected Files Section ----
    QWidget* outgoingBody = nullptr;
    auto* outgoingCard = makeSectionCard("Selected Files", this, &outgoingBody);
    outgoingCard->setObjectName("outgoingContainer");

    outgoingList_ = new QWidget(outgoingBody);
    outgoingList_->setObjectName("outgoingList");
    outgoingList_->setProperty("role", "deviceList"); // reusing list styling hook if you have one

    outgoingListLayout_ = new QVBoxLayout(outgoingList_);
    outgoingListLayout_->setContentsMargins(0, 0, 0, 0);
    outgoingListLayout_->setSpacing(10);
    outgoingListLayout_->addStretch(1);

    auto* outgoingScroll = new QScrollArea(outgoingBody);
    outgoingScroll->setObjectName("outgoingScroll");
    outgoingScroll->setWidgetResizable(true);
    outgoingScroll->setFrameShape(QFrame::NoFrame);
    outgoingScroll->setWidget(outgoingList_);

    qobject_cast<QVBoxLayout*>(outgoingBody->layout())->addWidget(outgoingScroll);

    // ---- Received Files Section ----
    QWidget* incomingBody = nullptr;
    auto* incomingCard = makeSectionCard("Received Files", this, &incomingBody);
    incomingCard->setObjectName("incomingContainer");

    incomingList_ = new QWidget(incomingBody);
    incomingList_->setObjectName("incomingList");

    incomingListLayout_ = new QVBoxLayout(incomingList_);
    incomingListLayout_->setContentsMargins(0, 0, 0, 0);
    incomingListLayout_->setSpacing(10);
    incomingListLayout_->addStretch(1);

    auto* incomingScroll = new QScrollArea(incomingBody);
    incomingScroll->setObjectName("incomingScroll");
    incomingScroll->setWidgetResizable(true);
    incomingScroll->setFrameShape(QFrame::NoFrame);
    incomingScroll->setWidget(incomingList_);

    qobject_cast<QVBoxLayout*>(incomingBody->layout())->addWidget(incomingScroll);

    bottomLayout->addWidget(outgoingCard, 1);
    bottomLayout->addWidget(incomingCard, 1);

    mainLayout->addWidget(bottomRow, 1);

    // ================= Signals / actions =================
    connect(backButton_, &QPushButton::clicked, this, [this]() {
        emit backRequested(); // you implement: stop server + navigate to login
    });

    connect(copyUrlButton_, &QPushButton::clicked, this, [this]() {
        QApplication::clipboard()->setText(urlLabel_->text());
    });

    connect(selectFilesButton_, &QPushButton::clicked, this, [this]() {
        const QStringList files = QFileDialog::getOpenFileNames(this, "Select Files");
        if (!files.isEmpty()) {
            setSelectedFiles(files);
        }
    });

    connect(sendButton_, &QPushButton::clicked, this, [this]() {
        if (!selectedFiles_.isEmpty())
            emit sendFilesRequested(selectedFiles_);
    });
}

// ================= Public API you can call from outside =================

void ServerWidget::setServerUrl(const QString& url) {
    serverUrl_ = url;
    urlLabel_->setText(url);

    // Render QR
    if (const QImage qrImg = generateQrCodeImage(url); !qrImg.isNull()) {
        constexpr int target = 280;
        qrLabel_->setPixmap(QPixmap::fromImage(qrImg).scaled(target, target, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        qrLabel_->setText("");
    } else {
        qrLabel_->setText("Failed to generate QR code");
    }
}

void ServerWidget::setSelectedFiles(const QStringList& files) {
    selectedFiles_ = files;
    sendButton_->setEnabled(!selectedFiles_.isEmpty());

    // Clear old rows
    qDeleteAll(outgoingRows_);
    outgoingRows_.clear();

    // Remove all items but stretch
    while (outgoingListLayout_->count() > 0) {
        auto* item = outgoingListLayout_->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    // Rebuild list
    for (const QString& path : selectedFiles_) {
        const QString name = QFileInfo(path).fileName();
        auto* row = new FileProgressRow(name, outgoingList_);
        outgoingRows_.insert(path, row);
        outgoingListLayout_->addWidget(row);
    }
    outgoingListLayout_->addStretch(1);
}

void ServerWidget::setOutgoingProgress(const QString& filePath, const int percent) {
    if (outgoingRows_.contains(filePath))
        outgoingRows_[filePath]->setProgress(percent);
}

void ServerWidget::addReceivedFile(const QString& fileIdOrName) {
    if (incomingRows_.contains(fileIdOrName))
        return;

    // Remove layout stretch temporarily
    if (incomingListLayout_->count() > 0) {
        QLayoutItem* last = incomingListLayout_->itemAt(incomingListLayout_->count() - 1);
        if (last && last->spacerItem()) {
            incomingListLayout_->takeAt(incomingListLayout_->count() - 1);
            delete last;
        }
    }

    auto* row = new FileProgressRow(fileIdOrName, incomingList_);
    incomingRows_.insert(fileIdOrName, row);
    incomingListLayout_->addWidget(row);
    incomingListLayout_->addStretch(1);
}

void ServerWidget::setReceivedProgress(const QString& fileIdOrName, int percent) {
    if (!incomingRows_.contains(fileIdOrName))
        addReceivedFile(fileIdOrName);

    incomingRows_[fileIdOrName]->setProgress(percent);
}

} // namespace blade

