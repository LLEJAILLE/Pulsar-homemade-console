#include "GameInstallerPage.h"

#include "../../utils/Paths.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStorageInfo>
#include <QVBoxLayout>

#include <algorithm>
#include <filesystem>

namespace
{
bool isSupportedRom(const QString &path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    return suffix == QStringLiteral("gba") || suffix == QStringLiteral("nds");
}
}

GameInstallerPage::GameInstallerPage(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("GameInstallerPage"));
    setFocusPolicy(Qt::StrongFocus);

    m_background = new BackgroundWidget(this);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);

    auto *titleLabel = new QLabel(QStringLiteral("Installer un jeu"), this);
    titleLabel->setStyleSheet(QStringLiteral("color: white; font-size: 26px; font-weight: 700;"));
    layout->addWidget(titleLabel);

    m_pathLabel = new QLabel(this);
    m_pathLabel->setStyleSheet(QStringLiteral("color: rgba(255, 255, 255, 190); font-size: 16px;"));
    layout->addWidget(m_pathLabel);

    m_entriesWidget = new QListWidget(this);
    m_entriesWidget->setFocusPolicy(Qt::NoFocus);
    m_entriesWidget->setStyleSheet(QStringLiteral(
        "QListWidget { color: white; background: rgba(0, 0, 0, 45); border: none; font-size: 18px; }"
        "QListWidget::item { padding: 8px; }"
        "QListWidget::item:selected { background: rgba(255, 255, 255, 55); font-weight: 700; }"));
    layout->addWidget(m_entriesWidget, 1);

    m_statusLabel = new QLabel(QStringLiteral("A: ouvrir ou installer   B: retour"), this);
    m_statusLabel->setStyleSheet(QStringLiteral("color: white; font-size: 14px;"));
    layout->addWidget(m_statusLabel);

    refreshEntries();
}

void GameInstallerPage::refreshEntries()
{
    m_entries.clear();
    m_entriesWidget->clear();

    if (m_currentPath.isEmpty()) {
        m_pathLabel->setText(QStringLiteral("Disques detectes"));
        for (const QFileInfo &drive : QDir::drives()) {
            const QStorageInfo storage(drive.absoluteFilePath());
            QString driveName = storage.displayName().trimmed();
            if (driveName.isEmpty()) {
                driveName = QStringLiteral("Disque local");
            }

            m_entries.push_back({ drive.absoluteFilePath(), true, false });
            m_entriesWidget->addItem(QStringLiteral("[Disque] %1 (%2)")
                .arg(driveName, drive.absoluteFilePath()));
        }
    } else {
        m_pathLabel->setText(m_currentPath);
        const QDir directory(m_currentPath);
        const bool consoleFolder = isConsoleFolder(m_currentPath);
        const QFileInfoList entries = directory.entryInfoList(
            QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot,
            QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);

        for (const QFileInfo &entry : entries) {
            if (entry.isDir()) {
                m_entries.push_back({ entry.absoluteFilePath(), true, false });
                m_entriesWidget->addItem(QStringLiteral("[Dossier] ") + entry.fileName());
            } else if (consoleFolder && isSupportedRom(entry.absoluteFilePath())) {
                m_entries.push_back({ entry.absoluteFilePath(), false, true });
                m_entriesWidget->addItem(QStringLiteral("[Jeu] ") + entry.fileName());
            }
        }
    }

    if (!m_entries.empty()) {
        m_entriesWidget->setCurrentRow(0);
    }
}

bool GameInstallerPage::isConsoleFolder(const QString &path) const
{
    const QString folderName = QFileInfo(path).fileName().toLower();
    return folderName == QStringLiteral("gba") || folderName == QStringLiteral("nds");
}

void GameInstallerPage::enterSelectedEntry()
{
    const int row = m_entriesWidget->currentRow();
    if (row < 0 || row >= static_cast<int>(m_entries.size())) {
        return;
    }

    const Entry &entry = m_entries[static_cast<std::size_t>(row)];
    if (entry.directory) {
        m_currentPath = entry.path;
        refreshEntries();
    } else if (entry.installable) {
        installSelectedGame();
    }
}

void GameInstallerPage::goBack()
{
    if (m_currentPath.isEmpty()) {
        emit backToSettings();
        return;
    }

    const QDir currentDirectory(m_currentPath);
    QDir parent = currentDirectory;
    if (!parent.cdUp()) {
        m_currentPath.clear();
    } else {
        m_currentPath = parent.absolutePath();
    }
    refreshEntries();
}

QString GameInstallerPage::findCover(const QString &romPath) const
{
    const QString stem = QFileInfo(romPath).completeBaseName();
    const QString driveRoot = QFileInfo(romPath).absoluteDir().rootPath();

    QStringList coverDirectories;
    coverDirectories << QDir(driveRoot).filePath(QStringLiteral("cover"));

    QDir sourceDirectory = QFileInfo(romPath).absoluteDir();
    for (int level = 0; level < 3; ++level) {
        coverDirectories << sourceDirectory.filePath(QStringLiteral("cover"));
        if (!sourceDirectory.cdUp()) {
            break;
        }
    }

    for (const QString &coverDirectoryPath : coverDirectories) {
        const QDir coverDirectory(coverDirectoryPath);
        const QFileInfoList covers = coverDirectory.entryInfoList(
            QStringList() << (stem + QStringLiteral(".png")),
            QDir::Files | QDir::Readable,
            QDir::Name);
        if (!covers.isEmpty()) {
            return covers.front().absoluteFilePath();
        }

        const QFileInfoList files = coverDirectory.entryInfoList(
            QDir::Files | QDir::Readable,
            QDir::Name);
        for (const QFileInfo &cover : files) {
            if (cover.suffix().compare(QStringLiteral("png"), Qt::CaseInsensitive) == 0 &&
                cover.completeBaseName().compare(stem, Qt::CaseInsensitive) == 0) {
                return cover.absoluteFilePath();
            }
        }
    }
    return {};
}

void GameInstallerPage::installSelectedGame()
{
    const int row = m_entriesWidget->currentRow();
    if (row < 0 || row >= static_cast<int>(m_entries.size()) || !m_entries[static_cast<std::size_t>(row)].installable) {
        return;
    }

    const QString sourcePath = m_entries[static_cast<std::size_t>(row)].path;
    const QString console = QFileInfo(m_currentPath).fileName().toLower();
    const QString destinationDirectory = QDir(Paths::library()).filePath(console);
    QDir().mkpath(destinationDirectory);
    const QString destinationPath = QDir(destinationDirectory).filePath(QFileInfo(sourcePath).fileName());

    if (QFileInfo(sourcePath).canonicalFilePath() == QFileInfo(destinationPath).canonicalFilePath()) {
        setStatus(QStringLiteral("Ce jeu est deja installe"));
        return;
    }

    setStatus(QStringLiteral("Installation en cours..."));
    QApplication::processEvents();

    QFile::remove(destinationPath);
    if (!QFile::copy(sourcePath, destinationPath)) {
        setStatus(QStringLiteral("Installation impossible"));
        return;
    }

    const QString coverPath = findCover(sourcePath);
    if (!coverPath.isEmpty()) {
        const QString iconPath = QDir(Paths::icons()).filePath(QFileInfo(coverPath).fileName());
        QFile::remove(iconPath);
        if (QFile::copy(coverPath, iconPath)) {
            setStatus(QStringLiteral("Jeu installe avec sa cover"));
        } else {
            setStatus(QStringLiteral("Jeu installe, cover non copiee"));
        }
    } else {
        setStatus(QStringLiteral("Jeu installe"));
    }

    emit gameInstalled();
}

void GameInstallerPage::setStatus(const QString &message)
{
    m_statusLabel->setText(message);
}

void GameInstallerPage::resizeEvent(QResizeEvent *event)
{
    m_background->setGeometry(rect());
    QWidget::resizeEvent(event);
}

void GameInstallerPage::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        m_entriesWidget->setCurrentRow(std::max(0, m_entriesWidget->currentRow() - 1));
        break;
    case Qt::Key_Down:
        m_entriesWidget->setCurrentRow(std::min(m_entriesWidget->count() - 1, m_entriesWidget->currentRow() + 1));
        break;
    case Qt::Key_A:
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        enterSelectedEntry();
        break;
    case Qt::Key_B:
    case Qt::Key_Escape:
        goBack();
        break;
    default:
        QWidget::keyPressEvent(event);
        return;
    }
    event->accept();
}