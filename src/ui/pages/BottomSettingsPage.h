#pragma once

#include "../widgets/BackgroundWidget.h"

#include <QWidget>
#include <QVector>

class QLabel;
class QListWidget;
class QKeyEvent;
class QResizeEvent;
class QShowEvent;
class QTimer;

class BottomSettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit BottomSettingsPage(QWidget *parent = nullptr);

signals:
    void backRequested();
    void libraryChanged();
    void statusChanged(const QString &message);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    struct BrowserEntry {
        QString label;
        QString absolutePath;
        bool isDirectory = false;
        bool isVolume = false;
    };

    enum class InstallTarget {
        None,
        Nds,
        Gba,
    };

    enum class StatusKind {
        Info,
        Loading,
        Success,
        Error,
    };

    void loadVolumes();
    void openPath(const QString &path, bool pushHistory);
    void rebuildEntries();
    void setStatus(const QString &message, StatusKind kind = StatusKind::Info);
    void goToParent();
    void goBackInHistory();
    void goForwardInHistory();
    bool installSelectedRom();
    void showPopup(const QString &message, StatusKind kind, int autoHideMs = 0);
    void hidePopup();
    InstallTarget detectInstallTargetForPath(const QString &path) const;
    QString installTargetName(InstallTarget target) const;

    BackgroundWidget *m_background = nullptr;
    QWidget *m_overlay = nullptr;
    QLabel *m_pathLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QListWidget *m_entriesList = nullptr;
    QWidget *m_popupContainer = nullptr;
    QLabel *m_popupLabel = nullptr;

    QString m_currentPath;
    QVector<BrowserEntry> m_entries;

    QVector<QString> m_history;
    int m_historyIndex = -1;
    int m_popupVersion = 0;
};
