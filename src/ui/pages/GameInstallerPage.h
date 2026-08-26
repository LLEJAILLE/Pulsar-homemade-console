#pragma once

#include "../widgets/BackgroundWidget.h"

#include <QWidget>

#include <filesystem>
#include <vector>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QKeyEvent;
class QResizeEvent;

class GameInstallerPage : public QWidget
{
    Q_OBJECT

public:
    explicit GameInstallerPage(QWidget *parent = nullptr);

signals:
    void backToSettings();
    void gameInstalled();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    struct Entry {
        QString path;
        bool directory = false;
        bool installable = false;
    };

    void refreshEntries();
    void enterSelectedEntry();
    void goBack();
    void installSelectedGame();
    bool isConsoleFolder(const QString &path) const;
    QString findCover(const QString &romPath) const;
    void setStatus(const QString &message);

    BackgroundWidget *m_background = nullptr;
    QLabel *m_pathLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QListWidget *m_entriesWidget = nullptr;
    std::vector<Entry> m_entries;
    QString m_currentPath;
};