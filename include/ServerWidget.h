#ifndef BLADE_SERVERWIDGET_H
#define BLADE_SERVERWIDGET_H

#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMap>
#include <QStringList>
#include <QFileDialog>
#include <QApplication>
#include <QMimeDatabase>
#include <QDragEnterEvent>
#include <QScrollArea>


namespace blade {

    // Forward declaration (defined in ServerWidget.cpp)
    class FileCard;

    class ServerWidget : public QWidget {
        Q_OBJECT

    public:
        explicit ServerWidget(QWidget* parent = nullptr);

        // Server / pairing
        void setServerUrl(const QString& url);

        // Outgoing (selected files)
        void setSelectedFiles(const QStringList& files);
        void setOutgoingProgress(const QString& filePath, int percent) const;

        // Incoming (received files)
        void addReceivedFile(const QString& fileIdOrName);
        void setReceivedProgress(const QString& fileIdOrName, int percent);

        signals:
            // User wants to stop server & go back
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

        // Outgoing list (optional, hidden)
        QFrame* dropZone_ = nullptr;
        QLabel* dropHintLabel_ = nullptr;
        QScrollArea* outgoingScroll_ = nullptr;
        QWidget* outgoingList_ = nullptr;
        QVBoxLayout* outgoingListLayout_ = nullptr;

        // Incoming list (scroll area content)
        QWidget* incomingList_ = nullptr;
        QVBoxLayout* incomingListLayout_ = nullptr;

        // State
        QString serverUrl_;
        QStringList selectedFiles_;

        // Progress rows
        QMap<QString, FileCard*> outgoingRows_;
        QMap<QString, FileCard*> incomingRows_;

    protected:
        void dragEnterEvent(QDragEnterEvent* event) override;
        void dragMoveEvent(QDragMoveEvent* event) override;
        void dropEvent(QDropEvent* event) override;
    };

} // namespace blade

#endif // BLADE_SERVERWIDGET_H
