#include "LoginWidget.h"


namespace blade {

LoginWidget::LoginWidget(QWidget* parent) : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 60, 50, 60);
    mainLayout->setSpacing(30);

    mainLayout->addStretch(1);

    auto* logoLabel = new QLabel(this);
    logoLabel->setPixmap(QPixmap(":/blade.png").scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setObjectName("logoLabel");
    mainLayout->addWidget(logoLabel, 0, Qt::AlignCenter);

    auto* titleLabel = new QLabel("BLADE", this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    auto* subtitleLabel = new QLabel("Bi-Directional LAN Asset Distribution Engine", this);
    subtitleLabel->setObjectName("subtitleLabel");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitleLabel);

    mainLayout->addSpacing(20);

    authModeCombo_ = new QComboBox(this);
    authModeCombo_->addItem("Start Without Authentication");
    authModeCombo_->addItem("Start With Authentication");
    mainLayout->addWidget(authModeCombo_, 0, Qt::AlignCenter);

    downloadPath_ = settings_.value("downloadPath", QDir::homePath()).toString();

    auto* pathRow = new QWidget(this);
    auto* pathLayout = new QHBoxLayout(pathRow);
    pathLayout->setContentsMargins(0, 0, 0, 0);

    downloadPathLabel_ = new QLabel(QString("Download location: %1").arg(downloadPath_), this);
    selectPathButton_ = new QPushButton("Change Location", this);

    pathLayout->addWidget(downloadPathLabel_, 1);
    pathLayout->addWidget(selectPathButton_);
    mainLayout->addWidget(pathRow, 0, Qt::AlignCenter);

    auto* formWidget = new QWidget(this);
    formWidget->setObjectName("formContainer");
    auto* formLayout = new QVBoxLayout(formWidget);
    formLayout->setSpacing(20);
    formLayout->setContentsMargins(40, 40, 40, 40);

    auto* usernameLabel = new QLabel("Username:", this);
    usernameLabel->setObjectName("fieldLabel");
    formLayout->addWidget(usernameLabel);

    usernameEdit_ = new QLineEdit(this);
    usernameEdit_->setObjectName("inputField");
    usernameEdit_->setPlaceholderText("Enter username");
    usernameEdit_->setMinimumHeight(48);
    formLayout->addWidget(usernameEdit_);

    auto* passwordLabel = new QLabel("Password:", this);
    passwordLabel->setObjectName("fieldLabel");
    formLayout->addWidget(passwordLabel);

    passwordEdit_ = new QLineEdit(this);
    passwordEdit_->setObjectName("inputField");
    passwordEdit_->setPlaceholderText("Enter password");
    passwordEdit_->setEchoMode(QLineEdit::Password);
    passwordEdit_->setMinimumHeight(48);
    formLayout->addWidget(passwordEdit_);

    mainLayout->addWidget(formWidget);
    mainLayout->addSpacing(10);

    auto* buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(15);

    startButton_ = new QPushButton("Start", this);
    startButton_->setObjectName("primaryButton");
    startButton_->setMinimumHeight(56);
    startButton_->setCursor(Qt::PointingHandCursor);
    mainLayout->addWidget(startButton_, 0, Qt::AlignCenter);

    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch(1);

    connect(authModeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](const int index) {
        const bool authEnabled = (index == 1);
        usernameEdit_->setEnabled(authEnabled);
        passwordEdit_->setEnabled(authEnabled);

        startButton_->setText("Start");
        startButton_->setVisible(true);
    });

    connect(selectPathButton_, &QPushButton::clicked, this, [this]() {
        if (const QString dir = QFileDialog::getExistingDirectory(this, "Select Download Folder", downloadPath_); !dir.isEmpty()) {
            downloadPath_ = dir;
            downloadPathLabel_->setText(QString("Download location: %1").arg(downloadPath_));
            settings_.setValue("downloadPath", downloadPath_);
            emit downloadPathChanged(downloadPath_);
        }
    });

    connect(startButton_, &QPushButton::clicked, this, [this]() {
        if (authModeCombo_->currentIndex() == 1) {
            emit startWithAuth(usernameEdit_->text(), passwordEdit_->text());
        } else {
            emit startNoAuth();
        }
    });
}

} // namespace blade