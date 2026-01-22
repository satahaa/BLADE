#ifndef BLADE_SERVERWIDGET_H
#define BLADE_SERVERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMap>
#include <QStringList>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QFileDialog>
#include <QScrollArea>
#include <QProgressBar>
#include <QFrame>
#include <QFont>
#include <QImage>
#include <QPixmap>
#include <QMap>
#include <QFileInfo>
#include <QClipboard>
#include <QApplication>
#include <algorithm>

namespace blade {

    // Forward declaration (defined in ServerWidget.cpp)
    class FileProgressRow;

    class ServerWidget : public QWidget {
        Q_OBJECT

    public:
        explicit ServerWidget(QWidget* parent = nullptr);

        // Server / pairing
        void setServerUrl(const QString& url);

        // Outgoing (selected files)
        void setSelectedFiles(const QStringList& files);
        void setOutgoingProgress(const QString& filePath, int percent);

        // Incoming (received files)
        void addReceivedFile(const QString& fileIdOrName);
        void setReceivedProgress(const QString& fileIdOrName, int percent);

        signals:
            // User wants to stop server & go back to login
            void backRequested();

        // User clicked "Send" for selected files
        void sendFilesRequested(const QStringList& files);

    private:
        // Top bar
        QPushButton* backButton_ = nullptr;
        QLabel* onlineLabel_ = nullptr;

        // Pairing section
        QLabel* qrLabel_ = nullptr;
        QLabel* urlLabel_ = nullptr;
        QPushButton* copyUrlButton_ = nullptr;

        // Actions
        QPushButton* selectFilesButton_ = nullptr;
        QPushButton* sendButton_ = nullptr;

        // Outgoing list
        QWidget* outgoingList_ = nullptr;
        QVBoxLayout* outgoingListLayout_ = nullptr;

        // Incoming list
        QWidget* incomingList_ = nullptr;
        QVBoxLayout* incomingListLayout_ = nullptr;

        // State
        QString serverUrl_;
        QStringList selectedFiles_;

        // Progress rows
        QMap<QString, FileProgressRow*> outgoingRows_;
        QMap<QString, FileProgressRow*> incomingRows_;
    };

} // namespace blade

#endif // BLADE_SERVERWIDGET_H
