#include "LoginWidget.h"
#include <QToolButton>
#include <QVBoxLayout>

namespace blade {

static QPixmap svgPixmap(const QString& resPath, const QSize& size) {
    return QIcon(resPath).pixmap(size);
}

LoginWidget::LoginWidget(QWidget* parent) : QWidget(parent) {
    setObjectName("loginRoot");
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);
    // ===== Root layout =====
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(56, 64, 56, 64);
    mainLayout->setSpacing(22);

    mainLayout->addStretch(1);

    // ===== Brand header =====
    auto* logoLabel = new QLabel(this);
    logoLabel->setObjectName("logoLabel");
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setPixmap(QPixmap(":/blade.png")
                             .scaled(118, 118, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    mainLayout->addWidget(logoLabel, 0, Qt::AlignHCenter);

    auto* titleLabel = new QLabel("BLADE", this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    auto* subtitleLabel = new QLabel("Bi-Directional LAN Asset Distribution Engine", this);
    subtitleLabel->setObjectName("subtitleLabel");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitleLabel);

    mainLayout->addSpacing(16);

    // ===== Card container =====
    auto* card = new QWidget(this);
    card->setObjectName("loginCard");
    card->setFixedWidth(560); // slightly wider for bigger text/icon balance

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(30, 28, 30, 28);
    cardLayout->setSpacing(18);

    // ===== Header row: encrypted icon + "Secure Access" + lock toggle icon =====
    auto* headerRowW = new QWidget(card);
    headerRowW->setObjectName("headerRow");

    auto* headerRow = new QHBoxLayout(headerRowW);
    headerRow->setContentsMargins(0, 0, 0, 0);
    headerRow->setSpacing(12);

    auto* encryptedIcon = new QLabel(headerRowW);
    encryptedIcon->setObjectName("cardHeaderIcon");
    encryptedIcon->setPixmap(svgPixmap(":/icons/encrypted.svg", QSize(24, 24)));
    encryptedIcon->setFixedSize(28, 28);
    encryptedIcon->setAlignment(Qt::AlignCenter);

    auto* headerTitle = new QLabel("Authentication", headerRowW);
    headerTitle->setObjectName("cardHeaderTitle");
    headerTitle->setStyleSheet("font-size: 18px; font-weight: 700;"); // slight bump

    // Icon-only toggle (transparent background)
    authToggleSwitch_ = new QToolButton(headerRowW);
    authToggleSwitch_->setObjectName("authToggleIcon");
    authToggleSwitch_->setCheckable(true);
    authToggleSwitch_->setChecked(false); // OFF by default
    authToggleSwitch_->setCursor(Qt::PointingHandCursor);
    authToggleSwitch_->setAutoRaise(true);
    authToggleSwitch_->setIcon(QIcon(":/icons/no_lock.svg"));
    authToggleSwitch_->setIconSize(QSize(26, 26));
    authToggleSwitch_->setToolTip("Enable authentication");

    headerRow->addWidget(encryptedIcon, 0, Qt::AlignVCenter);
    headerRow->addWidget(headerTitle, 1, Qt::AlignVCenter);
    headerRow->addWidget(authToggleSwitch_, 0, Qt::AlignVCenter);

    cardLayout->addWidget(headerRowW);

    auto* divider = new QWidget(card);
    divider->setObjectName("cardDivider");
    divider->setFixedHeight(1);
    cardLayout->addWidget(divider);

    // ===== Password row (key icon + input + eye toggle) =====
    auto* passwordRow = new QWidget(card);
    passwordRow->setObjectName("passwordRow");

    auto* passwordRowLayout = new QHBoxLayout(passwordRow);
    passwordRowLayout->setContentsMargins(14, 10, 14, 10); // real padding
    passwordRowLayout->setSpacing(12);

    auto* pwIcon = new QLabel(passwordRow);
    pwIcon->setObjectName("inputLeadingIcon");
    pwIcon->setPixmap(svgPixmap(":/icons/key.svg", QSize(20, 20)));
    pwIcon->setFixedSize(30, 30);
    pwIcon->setAlignment(Qt::AlignCenter);

    passwordEdit_ = new QLineEdit(passwordRow);
    passwordEdit_->setObjectName("inputField");
    passwordEdit_->setPlaceholderText("Enter Password");
    passwordEdit_->setEchoMode(QLineEdit::Password);
    passwordEdit_->setEnabled(false);
    passwordEdit_->setMinimumHeight(46);
    passwordEdit_->setStyleSheet("font-size: 16px;"); // bump input text slightly

    passwordVisButton_ = new QToolButton(passwordRow);
    passwordVisButton_->setObjectName("revealButton");
    passwordVisButton_->setCursor(Qt::PointingHandCursor);
    passwordVisButton_->setAutoRaise(true);
    passwordVisButton_->setIconSize(QSize(24, 24));
    passwordVisButton_->setToolTip("Show password");
    passwordVisButton_->setCheckable(true);
    passwordVisButton_->setChecked(false); // false = hidden (Password mode)
    passwordVisButton_->setIcon(QIcon(":/icons/visibility_off.svg"));

    passwordRowLayout->addWidget(pwIcon, 0, Qt::AlignVCenter);
    passwordRowLayout->addWidget(passwordEdit_, 1, Qt::AlignVCenter);
    passwordRowLayout->addWidget(passwordVisButton_, 0, Qt::AlignVCenter);

    cardLayout->addWidget(passwordRow);

    // ===== Download label =====
    auto* dlLabel = new QLabel("Download Location:", card);
    dlLabel->setObjectName("sectionLabel");
    dlLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #9ca3af;");
    cardLayout->addWidget(dlLabel);

    // ===== Download row (folder icon + path + change) =====
    downloadPath_ = settings_.value("downloadPath", QDir::homePath()).toString();

    auto* downloadRow = new QWidget(card);
    downloadRow->setObjectName("downloadRow");

    auto* downloadRowLayout = new QHBoxLayout(downloadRow);
    downloadRowLayout->setContentsMargins(14, 10, 14, 10);
    downloadRowLayout->setSpacing(18);

    auto* folderIcon = new QLabel(downloadRow);
    folderIcon->setObjectName("inputLeadingIcon");
    folderIcon->setPixmap(svgPixmap(":/icons/folder.svg", QSize(20, 20)));
    folderIcon->setFixedSize(30, 30);
    folderIcon->setAlignment(Qt::AlignCenter);

    downloadPathLabel_ = new QLabel(downloadPath_, downloadRow);
    downloadPathLabel_->setObjectName("pathPill");
    downloadPathLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    downloadPathLabel_->setMinimumHeight(46);
    downloadPathLabel_->setStyleSheet("font-size: 16px;"); // path should be readable

    selectPathButton_ = new QPushButton("Change", downloadRow);
    selectPathButton_->setObjectName("secondaryButton");
    selectPathButton_->setCursor(Qt::PointingHandCursor);
    selectPathButton_->setMinimumHeight(44);
    selectPathButton_->setStyleSheet("font-size: 14px;"); // bump button text

    downloadRowLayout->addWidget(folderIcon, 0, Qt::AlignVCenter);
    downloadRowLayout->addWidget(downloadPathLabel_, 1, Qt::AlignVCenter);
    downloadRowLayout->addWidget(selectPathButton_, 0, Qt::AlignVCenter);

    cardLayout->addWidget(downloadRow);

    cardLayout->addSpacing(10);

    // ===== Primary action =====
    startButton_ = new QPushButton("", card);
    startButton_->setObjectName("primaryButton");
    startButton_->setCursor(Qt::PointingHandCursor);
    startButton_->setMinimumHeight(54);
    startButton_->setIcon(QIcon(":/icons/login.svg"));
    startButton_->setIconSize(QSize(22, 22));
    startButton_->setStyleSheet("font-size: 15px; font-weight: 800; letter-spacing: 2px;");
    startButton_->setToolTip("Start Server");

    cardLayout->addWidget(startButton_);

    mainLayout->addWidget(card, 0, Qt::AlignHCenter);
    mainLayout->addStretch(1);

    // ===== Behavior =====

    // Secure access toggle: lock <-> no_lock, enables password field
    connect(authToggleSwitch_, &QToolButton::toggled, this, [this](const bool enabled) {
        passwordEdit_->setEnabled(enabled);

        authToggleSwitch_->setIcon(QIcon(enabled ? ":/icons/lock.svg"
                                              : ":/icons/no_lock.svg"));
        authToggleSwitch_->setToolTip(enabled ? "Disable authentication"
                                            : "Enable authentication");

        if (!enabled) {
            passwordEdit_->clear();
            passwordEdit_->setEchoMode(QLineEdit::Password);
            passwordVisButton_->setIcon(QIcon(":/icons/visibility_off.svg"));
            passwordVisButton_->setToolTip("Show password");
        }
    });

    // Password visibility: visibility <-> visibility_off
    connect(passwordVisButton_, &QToolButton::toggled, this, [this](bool show) {
        // If auth is OFF, keep it hidden and keep icon as off
        if (!passwordEdit_->isEnabled()) {
            passwordVisButton_->blockSignals(true);
            passwordVisButton_->setChecked(false);
            passwordVisButton_->blockSignals(false);

            passwordEdit_->setEchoMode(QLineEdit::Password);
            passwordVisButton_->setIcon(QIcon(":/icons/visibility_off.svg"));
            passwordVisButton_->setToolTip("Show password");
            return;
        }

        passwordEdit_->setEchoMode(show ? QLineEdit::Normal : QLineEdit::Password);
        passwordVisButton_->setIcon(QIcon(show ? ":/icons/visibility.svg"
                                              : ":/icons/visibility_off.svg"));
        passwordVisButton_->setToolTip(show ? "Hide password" : "Show password");
    });

    // Download path selection
    connect(selectPathButton_, &QPushButton::clicked, this, [this]() {
        const QString dir = QFileDialog::getExistingDirectory(
            this, "Select Download Folder", downloadPath_);
        if (!dir.isEmpty()) {
            downloadPath_ = dir;
            downloadPathLabel_->setText(downloadPath_);
            settings_.setValue("downloadPath", downloadPath_);
            emit downloadPathChanged(downloadPath_);
        }
    });

    // Start
    connect(startButton_, &QPushButton::clicked, this, [this]() {
        if (authToggleSwitch_->isChecked()) {
            emit startWithAuth(passwordEdit_->text());
        } else {
            emit startNoAuth();
        }
    });
}

} // namespace blade