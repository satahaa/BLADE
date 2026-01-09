#include "LoginWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>

namespace blade {

LoginWidget::LoginWidget(QWidget* parent) : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 60, 50, 60);
    mainLayout->setSpacing(30);

    // Add stretch at top
    mainLayout->addStretch(1);

    // Logo
    auto* logoLabel = new QLabel(this);
    logoLabel->setPixmap(QPixmap(":/blade.ico").scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setObjectName("logoLabel");
    mainLayout->addWidget(logoLabel, 0, Qt::AlignCenter);

    // Title
    auto* titleLabel = new QLabel("BLADE", this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Subtitle
    auto* subtitleLabel = new QLabel("Local Network File Transfer", this);
    subtitleLabel->setObjectName("subtitleLabel");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitleLabel);

    mainLayout->addSpacing(20);

    // Form container
    auto* formWidget = new QWidget(this);
    formWidget->setObjectName("formContainer");
    auto* formLayout = new QVBoxLayout(formWidget);
    formLayout->setSpacing(20);
    formLayout->setContentsMargins(40, 40, 40, 40);

    // Username
    auto* usernameLabel = new QLabel("Username (Optional)", this);
    usernameLabel->setObjectName("fieldLabel");
    formLayout->addWidget(usernameLabel);

    usernameEdit_ = new QLineEdit(this);
    usernameEdit_->setObjectName("inputField");
    usernameEdit_->setPlaceholderText("Enter username");
    usernameEdit_->setMinimumHeight(48);
    formLayout->addWidget(usernameEdit_);

    // Password
    auto* passwordLabel = new QLabel("Password (Optional)", this);
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

    // Buttons
    auto* buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(15);

    startAuthButton_ = new QPushButton("Start with Authentication", this);
    startAuthButton_->setObjectName("primaryButton");
    startAuthButton_->setMinimumHeight(56);
    startAuthButton_->setCursor(Qt::PointingHandCursor);
    buttonLayout->addWidget(startAuthButton_);

    startNoAuthButton_ = new QPushButton("Start without Authentication", this);
    startNoAuthButton_->setObjectName("secondaryButton");
    startNoAuthButton_->setMinimumHeight(56);
    startNoAuthButton_->setCursor(Qt::PointingHandCursor);
    buttonLayout->addWidget(startNoAuthButton_);

    mainLayout->addLayout(buttonLayout);

    // Add stretch at bottom
    mainLayout->addStretch(1);

    // Connect signals
    connect(startAuthButton_, &QPushButton::clicked, this, [this]() {
        emit startWithAuth(usernameEdit_->text(), passwordEdit_->text());
    });

    connect(startNoAuthButton_, &QPushButton::clicked, this, [this]() {
        emit startNoAuth();
    });
}

} // namespace blade

