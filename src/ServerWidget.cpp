#include "ServerWidget.h"
#include "Logger.h"
#include <QClipboard>
#include <QToolButton>
#include <QProgressBar>
#include <QScrollArea>
#include <QTimer>
#include <QRCodeGen.h>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QDebug>

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

static QFrame* makeCard(QWidget* parent, const QString& objName) {
    auto* card = new QFrame(parent);
    card->setObjectName(objName);
    card->setFrameShape(QFrame::NoFrame);
    card->setAttribute(Qt::WA_StyledBackground, true);
    return card;
}

static QWidget* makeSectionTitle(const QString& title, QWidget* parent) {
    auto* t = new QLabel(title, parent);
    t->setObjectName("sectionTitle");
    return t;
}

    static QString humanSize(qint64 bytes) {
    return QLocale().formattedDataSize(bytes); // e.g. "2.4 MB"
}

    static QString iconForPath(const QString& filePath) {
    QFileInfo fi(filePath);
    const QString ext = fi.suffix().toLower();

    // Quick common extensions first (fast + predictable)
    static const QSet<QString> images = {"png","jpg","jpeg","webp","bmp","gif","svg"};
    static const QSet<QString> videos = {"mp4","mkv","mov","avi","webm","flv","wmv"};
    static const QSet<QString> audio  = {"mp3","wav","ogg","flac","m4a","aac"};
    static const QSet<QString> docs   = {"pdf","doc","docx","ppt","pptx","xls","xlsx","txt","rtf"};
    static const QSet<QString> code   = {"cpp","c","h","hpp","cc","cxx","py","js","ts","java","kt","cs","go","rs","php","rb","swift","html","css","json","xml","yaml","yml","md","sh"};

    if (images.contains(ext)) return ":/icons/image.svg";
    if (videos.contains(ext)) return ":/icons/video.svg";
    if (audio.contains(ext))  return ":/icons/audio.svg";
    if (docs.contains(ext))   return ":/icons/text.svg";
    if (code.contains(ext))   return ":/icons/source_code.svg";

    // Fallback: try MIME type (works well for many cases)
    QMimeDatabase db;
    const QMimeType mt = db.mimeTypeForFile(fi);
    const QString m = mt.name(); // e.g. "image/png"

    if (m.startsWith("image/")) return ":/icons/image.svg";
    if (m.startsWith("video/")) return ":/icons/video.svg";
    if (m.startsWith("audio/")) return ":/icons/audio.svg";
    if (m == "application/pdf") return ":/icons/pdf.svg";
    if (m.startsWith("text/"))  return ":/icons/text.svg";

    return ":/icons/file.svg"; // final fallback
}

// Forward-declared in ServerWidget.h; full definition lives here.
class FileCard final : public QWidget {
public:
    explicit FileCard(const QString& filePath, const bool showRemove, QWidget* parent = nullptr, const qint64 knownSize = -1)
        : QWidget(parent)
    {
        setObjectName("fileProgressRow");
        setFixedHeight(74);

        auto* root = new QVBoxLayout(this);
        root->setContentsMargins(12, 4, 12, 4);
        root->setSpacing(8);

        auto* top = new QWidget(this);
        auto* topL = new QHBoxLayout(top);
        topL->setContentsMargins(0, 0, 0, 0);
        topL->setSpacing(10);

        QFileInfo fi(filePath);

        auto* icon = new QLabel(top);
        icon->setObjectName("fileIcon");
        icon->setFixedSize(18, 18);
        icon->setPixmap(QIcon(iconForPath(filePath)).pixmap(18, 18));

        name_ = new QLabel(fi.fileName(), top);
        name_->setObjectName("fileName");
        name_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred); // Use Ignored to allow shrinking
        name_->setWordWrap(false);
        name_->setTextFormat(Qt::PlainText);

        // Store full filename for eliding
        fullName_ = fi.fileName();
        name_->setToolTip(fullName_); // Show full name on hover


        removeBtn_ = new QToolButton(top);
        removeBtn_->setObjectName("removeBtn");
        removeBtn_->setCursor(Qt::PointingHandCursor);
        removeBtn_->setAutoRaise(true);
        removeBtn_->setIcon(QIcon(":/icons/close.svg"));
        removeBtn_->setIconSize(QSize(18, 18));
        removeBtn_->setVisible(showRemove);
        removeBtn_->setFixedSize(28, 28);

        topL->addWidget(icon);
        topL->addWidget(name_);
        topL->addWidget(removeBtn_);

        bar_ = new QProgressBar(this);
        bar_->setObjectName("fileProgress");
        bar_->setRange(0, 100);
        bar_->setValue(0);
        bar_->setTextVisible(false);
        bar_->setFixedHeight(7);

        size_ = new QLabel(this);
        size_->setObjectName("fileSize");

        // Use known size if provided, otherwise try to get from file info
        if (knownSize >= 0) {
            size_->setText(humanSize(knownSize));
        } else if (fi.exists() && fi.isFile()) {
            size_->setText(humanSize(fi.size()));
        } else {
            size_->setText("Unknown size");
        }
        size_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        root->addWidget(top);
        root->addWidget(size_);
        root->addWidget(bar_);

        // Initial text eliding
        QTimer::singleShot(0, this, [this]() { updateElidedText(); });
    }

    void setProgress(const int pct) const { bar_->setValue(std::clamp(pct, 0, 100)); }
    void setSize(const qint64 bytes) const {
        if (size_) {
            size_->setText(humanSize(bytes));
        }
    }
    [[nodiscard]] QToolButton* removeButton() const { return removeBtn_; }

protected:
    void resizeEvent(QResizeEvent* event) override {
        QWidget::resizeEvent(event);
        updateElidedText();
    }

private:
    void updateElidedText() const {
        if (!name_ || fullName_.isEmpty()) return;

        // Calculate available width: widget width minus margins, icon, button, and spacing
        const int availableWidth = width() - 12 - 12 - 18 - 28 - 10 - 10 - 20; // margins + icon + button + spacing + buffer

        QFontMetrics metrics(name_->font());
        QString elidedText = metrics.elidedText(fullName_, Qt::ElideMiddle, availableWidth);
        name_->setText(elidedText);
    }

    QLabel* name_ = nullptr;
    QLabel* size_ = nullptr;
    QToolButton* removeBtn_ = nullptr;
    QProgressBar* bar_ = nullptr;
    QString fullName_;
};

ServerWidget::ServerWidget(QWidget* parent) : QWidget(parent) {
    setObjectName("serverWidgetRoot");
    setAttribute(Qt::WA_StyledBackground, true);
    setAttribute(Qt::WA_StyledBackground, true);
    setAcceptDrops(true);

    // ================= Root layout =================
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(18, 18, 18, 18);
    rootLayout->setSpacing(14);

    // ================= Top bar (NO window buttons) =================
    auto* topBar = new QWidget(this);
    topBar->setObjectName("topBar");
    auto* topL = new QHBoxLayout(topBar);
    topL->setContentsMargins(0, 0, 0, 0);
    topL->setSpacing(10);

    backButton_ = new QPushButton(topBar);
    backButton_->setObjectName("backBtn");
    backButton_->setCursor(Qt::PointingHandCursor);
    backButton_->setIcon(QIcon(":/icons/back.svg"));
    backButton_->setIconSize(QSize(22, 22));
    backButton_->setText("");
    backButton_->setStyleSheet(
        "QPushButton#backBtn { background: transparent; border: none; }"
        "QPushButton#backBtn:hover { background: #3d4552; }"
    );

    // Replace the statusWrap/onlineLabel_ section in ServerWidget constructor:

    auto* statusWrap = new QWidget(topBar);
    statusWrap->setObjectName("statusWrap");
    auto* statusL = new QHBoxLayout(statusWrap);
    statusL->setContentsMargins(12, 2, 12, 10 );
    statusL->setSpacing(8);

    // Icon
    auto* statusIcon = new QLabel(statusWrap);
    statusIcon->setObjectName("statusIcon");
    statusIcon->setFixedSize(32, 32);
    statusIcon->setScaledContents(true);
    statusIcon->setStyleSheet("padding-top: 6px;");

    // Text
    onlineLabel_ = new QLabel("", statusWrap);
    onlineLabel_->setObjectName("statusText");
    onlineLabel_->setStyleSheet(
        "font-size: 25px;"
        "color: " + onlineLabel_->palette().color(QPalette::WindowText).name() + ";"
    );


    // Add to layout
    statusL->addWidget(statusIcon);
    statusL->addWidget(onlineLabel_);

    // Style the wrapper
    statusWrap->setStyleSheet(
        "background: Transparent;"
    );

    // Center in window
    topL->addStretch(1);
    topL->addWidget(statusWrap, 0, Qt::AlignCenter);
    topL->addStretch(1);

    rootLayout->addWidget(topBar);

    // ================= Main 2-column content =================

    auto* content = new QWidget(this);
    content->setObjectName("content");
    auto* contentL = new QHBoxLayout(content);
    contentL->setContentsMargins(0, 0, 0, 0);
    contentL->setSpacing(16);
    // ---------- Left card: QR + URL ----------
    auto* leftCard = makeCard(content, "leftCard");
    leftCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    leftCard->setMinimumHeight(680); // Increased significantly for better aesthetics
    auto* left = new QVBoxLayout(leftCard);
    left->setContentsMargins(18, 18, 18, 18);
    left->setSpacing(14);


    // QR code box
    auto* qrBox = new QFrame(leftCard);
    qrBox->setObjectName("qrBox");
    qrBox->setMinimumHeight(380); // Increased for larger QR
    auto* qrBoxL = new QVBoxLayout(qrBox);
    qrBoxL->setContentsMargins(0, 0, 0, 0);
    qrBoxL->setAlignment(Qt::AlignCenter);

    qrLabel_ = new QLabel(qrBox);
    qrLabel_->setObjectName("qrCode");
    qrLabel_->setAlignment(Qt::AlignCenter);
    qrLabel_->setText("QR");
    qrLabel_->setMinimumSize(300, 300); // Larger QR code
    qrBoxL->addWidget(qrLabel_);

    left->addStretch(1);
    // Add QR box to card
    left->addWidget(qrBox);

    // Divider between QR and URL
    auto* divider = new QWidget(leftCard);
    divider->setObjectName("cardDivider");
    divider->setFixedHeight(1);
    left->addWidget(divider);

    // URL row
    auto* urlRow = new QWidget(leftCard);
    urlRow->setObjectName("urlRow");
    auto* urlL = new QHBoxLayout(urlRow);
    urlL->setContentsMargins(0, 0, 0, 0);
    urlL->setSpacing(10);

    urlLabel_ = new QLabel(urlRow);
    urlLabel_->setObjectName("urlText");
    urlLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    urlLabel_->setText("http://0.0.0.0:0000");
    urlLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    copyUrlButton_ = new QPushButton(urlRow);
    copyUrlButton_->setObjectName("copyBtn");
    copyUrlButton_->setCursor(Qt::PointingHandCursor);
    copyUrlButton_->setIcon(QIcon(":/icons/link.svg"));
    copyUrlButton_->setIconSize(QSize(18, 18));
    copyUrlButton_->setFixedSize(36, 30);
    copyUrlButton_->setText("");

    urlL->addWidget(urlLabel_, 1);
    urlL->addWidget(copyUrlButton_, 0);

    // Add URL row and hint
    left->addWidget(urlRow);

    auto* hint = new QLabel("Scan the QR or open the link in your browser to connect", leftCard);
    hint->setObjectName("hintText");
    hint->setWordWrap(true);
    left->addWidget(hint);

    left->addStretch(1);
    // ---------- Right column: Send Files + Received Files ----------
    auto* rightCol = new QWidget(content);
    rightCol->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightCol->setObjectName("rightCol");
    auto* right = new QVBoxLayout(rightCol);
    right->setContentsMargins(0, 0, 0, 0);
    right->setSpacing(16);

    // Send Files card
    auto* sendCard = makeCard(rightCol, "sendCard");
    sendCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sendCard->setMinimumHeight(330); // Increased significantly
    auto* send = new QVBoxLayout(sendCard);
    send->setContentsMargins(18, 18, 18, 18);
    send->setSpacing(4);

    send->addWidget(makeSectionTitle("Send Files", sendCard));

    dropZone_ = new QFrame(sendCard);
    auto* dropZone = dropZone_;
    //dropZone->setMinimumHeight(240);
    dropZone->setProperty("dropTarget", true);
    dropZone->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto* dzL = new QVBoxLayout(dropZone);
    dzL->setAlignment(Qt::AlignCenter);

    dropHintLabel_ = new QLabel("Drag and Drop Files", dropZone);
    dropHintLabel_->setObjectName("dropText");
    dropHintLabel_->setAlignment(Qt::AlignCenter);
    dzL->addWidget(dropHintLabel_);

    auto* actionRow = new QWidget(sendCard);
    actionRow->setObjectName("sendActions");
    auto* act = new QHBoxLayout(actionRow);
    act->setContentsMargins(0, 0, 0, 0);
    act->setSpacing(10);

    selectFilesButton_ = new QPushButton(actionRow);
    selectFilesButton_->setObjectName("browseBtn");
    selectFilesButton_->setCursor(Qt::PointingHandCursor);
    selectFilesButton_->setIcon(QIcon(":/icons/folder.svg"));
    selectFilesButton_->setIconSize(QSize(20, 20));
    selectFilesButton_->setFixedSize(44, 34);
    selectFilesButton_->setText("");

    sendButton_ = new QPushButton(actionRow);
    sendButton_->setObjectName("sendBtn");
    sendButton_->setCursor(Qt::PointingHandCursor);
    sendButton_->setIcon(QIcon(":/icons/send.svg"));
    sendButton_->setIconSize(QSize(20, 20));
    sendButton_->setFixedSize(44, 34);
    sendButton_->setText("");
    sendButton_->setEnabled(false);

    act->addWidget(selectFilesButton_);
    act->addStretch(1);
    act->addWidget(sendButton_);

    outgoingList_ = new QWidget(sendCard);
    outgoingList_->setObjectName("outgoingList");
    outgoingList_->setVisible(false);

    outgoingListLayout_ = new QVBoxLayout(outgoingList_);
    outgoingListLayout_->setContentsMargins(0, 0, 0, 0);
    outgoingListLayout_->setSpacing(10);
    outgoingListLayout_->addStretch(1);

    outgoingScroll_ = new QScrollArea(sendCard);
    outgoingScroll_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    outgoingScroll_->setObjectName("outgoingScroll");
    outgoingScroll_->setWidgetResizable(true);
    outgoingScroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    outgoingScroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    outgoingScroll_->setFrameShape(QFrame::NoFrame);
    outgoingScroll_->setWidget(outgoingList_);
    outgoingScroll_->setVisible(false);

    send->addWidget(dropZone, 1);
    send->addWidget(outgoingScroll_, 1);
    send->addWidget(actionRow, 0);

    // Received Files card
    auto* recvCard = makeCard(rightCol, "recvCard");
    recvCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    recvCard->setMinimumHeight(330); // Increased significantly
    auto* recv = new QVBoxLayout(recvCard);
    recv->setContentsMargins(18, 18, 18, 18);
    recv->setSpacing(12);

    recv->addWidget(makeSectionTitle("Received Files", recvCard));

    incomingList_ = new QWidget(recvCard);
    incomingList_->setObjectName("incomingList");

    incomingListLayout_ = new QVBoxLayout(incomingList_);
    incomingListLayout_->setContentsMargins(0, 0, 0, 0);
    incomingListLayout_->setSpacing(10);
    incomingListLayout_->addStretch(1);

    right->addWidget(sendCard, 1);
    right->setStretch(0, 1);

    right->addWidget(recvCard, 1);
    right->setStretch(1, 1);

    auto* incomingScroll = new QScrollArea(recvCard);
    incomingScroll->setObjectName("incomingScroll");
    incomingScroll->setWidgetResizable(true);
    incomingScroll->setFrameShape(QFrame::NoFrame);
    incomingScroll->setWidget(incomingList_);

    recv->addWidget(incomingScroll);

    contentL->addWidget(leftCard, 1);
    contentL->addWidget(rightCol, 1);

    rootLayout->addWidget(content, 1);

    // ================= Signals =================
    connect(backButton_, &QPushButton::clicked, this, [this]() { emit backRequested(); });

    connect(copyUrlButton_, &QPushButton::clicked, this, [this]() {
        QApplication::clipboard()->setText(urlLabel_->text());
    });

    connect(selectFilesButton_, &QPushButton::clicked, this, [this]() {
        if (const QStringList files = QFileDialog::getOpenFileNames(this, "Select Files");
            !files.isEmpty())
            setSelectedFiles(files);
    });

    connect(sendButton_, &QPushButton::clicked, this, [this]() {
        Logger::getInstance().info("Send button clicked, selectedFiles count: " + std::to_string(selectedFiles_.size()));
        if (!selectedFiles_.isEmpty()) {
            Logger::getInstance().info("Emitting sendFilesRequested with " + std::to_string(selectedFiles_.size()) + " files");
            emit sendFilesRequested(selectedFiles_);
        } else {
            Logger::getInstance().warning("Send button clicked but selectedFiles_ is empty!");
        }
    });
}

void ServerWidget::setServerUrl(const QString& url) {
    serverUrl_ = url;
    urlLabel_->setText(url);

    if (const QImage qrImg = generateQrCodeImage(url); !qrImg.isNull()) {
        constexpr int target = 260;
        qrLabel_->setPixmap(QPixmap::fromImage(qrImg).scaled(target, target, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        qrLabel_->setText("Failed to generate QR code");
    }
}

void ServerWidget::setSelectedFiles(const QStringList& files) {
    Logger::getInstance().debug("setSelectedFiles called with " + std::to_string(files.size()) + " files");
    for (const QString& f : files) {
        if (!selectedFiles_.contains(f))
            selectedFiles_.append(f);
    }
    Logger::getInstance().debug("Total selected files: " + std::to_string(selectedFiles_.size()));
    outgoingList_->setVisible(!selectedFiles_.isEmpty());
    dropHintLabel_->setVisible(selectedFiles_.isEmpty());
    outgoingScroll_->setVisible(!selectedFiles_.isEmpty());
    sendButton_->setEnabled(!selectedFiles_.isEmpty());
    Logger::getInstance().debug("sendButton_ enabled: " + std::string(sendButton_->isEnabled() ? "true" : "false"));
    dropZone_->setVisible(selectedFiles_.isEmpty());

    // remove bottom stretch if present (so we can append above it)
    if (outgoingListLayout_->count() > 0) {
        if (QLayoutItem* last = outgoingListLayout_->itemAt(outgoingListLayout_->count() - 1); last && last->spacerItem()) {
            outgoingListLayout_->takeAt(outgoingListLayout_->count() - 1);
            delete last;
        }
    }

    // add rows only for files that don't already have a row
    for (const QString& path : selectedFiles_) {
        if (outgoingRows_.contains(path))
            continue;

        auto* row = new FileCard(path, /*showRemove*/ true, outgoingList_);

        outgoingRows_.insert(path, row);
        outgoingListLayout_->addWidget(row);

        connect(row->removeButton(), &QToolButton::clicked, this, [this, path, row]() {
            outgoingRows_.remove(path);
            selectedFiles_.removeAll(path);
            row->deleteLater();

            const bool empty = selectedFiles_.isEmpty();
            outgoingScroll_->setVisible(!empty);
            outgoingList_->setVisible(!empty);
            dropZone_->setVisible(empty);
            dropHintLabel_->setVisible(empty);
            sendButton_->setEnabled(!empty);
        });
    }

    // add the stretch back (push rows to top)
    outgoingListLayout_->addStretch(1);
}

void ServerWidget::setOutgoingProgress(const QString& filePath, const int percent) const {
    if (const auto* row = outgoingRows_.value(filePath, nullptr)) {
        row->setProgress(percent);
    }
}

void ServerWidget::addReceivedFile(const QString& fileIdOrName) {
    if (incomingRows_.contains(fileIdOrName)) return;

    // remove stretch
    if (incomingListLayout_->count() > 0) {
        if (QLayoutItem* last = incomingListLayout_->itemAt(incomingListLayout_->count() - 1);
            last && last->spacerItem()) {
            incomingListLayout_->takeAt(incomingListLayout_->count() - 1);
            delete last;
        }
    }

    // Use known size if available
    const qint64 knownSize = incomingFileSizes_.value(fileIdOrName, -1);
    auto* row = new FileCard(fileIdOrName, /*showRemove*/ true, incomingList_, knownSize);
    incomingRows_.insert(fileIdOrName, row);

    connect(row->removeButton(), &QToolButton::clicked, this, [this, fileIdOrName, row]() {
        incomingRows_.remove(fileIdOrName);
        incomingFileSizes_.remove(fileIdOrName);
        row->deleteLater();
    });

    incomingListLayout_->addWidget(row);
    incomingListLayout_->addStretch(1);
}

void ServerWidget::setReceivedFile(const QString& filename, const quint64 fileSize) {
    // Store the file size
    incomingFileSizes_.insert(filename, fileSize);

    // Add the file to UI if not already present, or update the size if it exists
    if (!incomingRows_.contains(filename)) {
        addReceivedFile(filename);
    } else {
        // Update size of existing card (in case it was created without size info)
        if (auto* row = incomingRows_.value(filename, nullptr)) {
            row->setSize(fileSize);
        }
    }
}

void ServerWidget::setReceivedProgress(const QString& fileIdOrName, const int percent) {
    // Only update progress if the file card already exists (created by setReceivedFile)
    if (const auto* row = incomingRows_.value(fileIdOrName, nullptr)) {
        row->setProgress(percent);
    }
}
    void ServerWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData() && event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

    void ServerWidget::dragMoveEvent(QDragMoveEvent* event) {
    if (event->mimeData() && event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

    void ServerWidget::dropEvent(QDropEvent* event) {
    if (!event->mimeData() || !event->mimeData()->hasUrls())
        return;

    QStringList files;
    const auto urls = event->mimeData()->urls();
    for (const QUrl& url : urls) {
        if (url.isLocalFile()) {
            const QString path = url.toLocalFile();
            QFileInfo fi(path);
            if (fi.exists() && fi.isFile())
                files << path;
        }
    }

    if (!files.isEmpty()) {
        setSelectedFiles(files);
        outgoingList_->setVisible(true);
    }

    event->acceptProposedAction();
}


} // namespace blade
