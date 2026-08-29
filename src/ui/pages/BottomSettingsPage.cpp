#include "BottomSettingsPage.h"

#include "../../utils/Paths.h"

#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QSet>
#include <QStorageInfo>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

namespace
{
constexpr int kPageMargin = 8;  // Reduced for vertical 800x480 display
constexpr int kPageSpacing = 4;  // Reduced spacing

const char *kSettingsPageStyle = R"(
    BottomSettingsPage {
        background-color: rgb(248, 0, 0)
    }

    QLabel {
        color: white;
        background: transparent;
    }

    QListWidget {
        background: rgba(0, 0, 0, 60);
        color: white;
        border: 1px solid rgba(255, 255, 255, 60);
        border-radius: 10px;
        padding: 6px;
        font-size: 16px;
    }

    QListWidget::item {
        padding: 6px;
        border-radius: 6px;
    }

    QListWidget::item:selected {
        background: rgba(255, 255, 255, 50);
    }
)";

bool isRomExtension(const QString &suffix)
{
    const QString lowerSuffix = suffix.toLower();
    return lowerSuffix == QStringLiteral("nds") || lowerSuffix == QStringLiteral("gba");
}
}

BottomSettingsPage::BottomSettingsPage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("BottomSettingsPage"));
    setStyleSheet(QString::fromLatin1(kSettingsPageStyle));
    setFocusPolicy(Qt::StrongFocus);

    m_background = new BackgroundWidget(this);
    m_overlay = new QWidget(this);
    m_overlay->setAttribute(Qt::WA_TranslucentBackground);

    auto *layout = new QVBoxLayout(m_overlay);
    layout->setContentsMargins(kPageMargin, kPageMargin, kPageMargin, kPageMargin);
    layout->setSpacing(kPageSpacing);

    m_pathLabel = new QLabel(QStringLiteral("Volumes"), m_overlay);
    m_pathLabel->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: 700;"));

    m_statusLabel = new QLabel(QStringLiteral("Choose a disk or USB volume"), m_overlay);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setStyleSheet(QStringLiteral("font-size: 15px;"));

    m_entriesList = new QListWidget(m_overlay);
    m_entriesList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_entriesList->setFocusPolicy(Qt::NoFocus);

    m_popupContainer = new QWidget(this);
    m_popupContainer->hide();
    m_popupContainer->setAttribute(Qt::WA_StyledBackground, true);

    auto *popupLayout = new QHBoxLayout(m_popupContainer);
    popupLayout->setContentsMargins(12, 8, 12, 8);
    popupLayout->setSpacing(0);

    m_popupLabel = new QLabel(m_popupContainer);
    m_popupLabel->setWordWrap(true);
    m_popupLabel->setAlignment(Qt::AlignCenter);
    popupLayout->addWidget(m_popupLabel);

    layout->addWidget(m_pathLabel);
    layout->addWidget(m_statusLabel);
    layout->addWidget(m_entriesList, 1);

    loadVolumes();
}

void BottomSettingsPage::resizeEvent(QResizeEvent *event)
{
    if (m_background != nullptr) {
        m_background->setGeometry(rect());
    }

    if (m_overlay != nullptr) {
        m_overlay->setGeometry(rect());
    }

    if (m_popupContainer != nullptr) {
        const int popupWidth = std::min(500, std::max(260, width() - 40));
        const int popupHeight = 64;
        const int popupX = (width() - popupWidth) / 2;
        const int popupY = height() - popupHeight - 20;
        m_popupContainer->setGeometry(popupX, popupY, popupWidth, popupHeight);
    }

    QWidget::resizeEvent(event);
}

void BottomSettingsPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    QTimer::singleShot(0, this, [this]() {
        setFocus(Qt::OtherFocusReason);
    });
}

void BottomSettingsPage::keyPressEvent(QKeyEvent *event)
{
    if (m_entriesList == nullptr) {
        event->ignore();
        return;
    }

    switch (event->key()) {
    case Qt::Key_Down: {
        const int nextRow = std::min(m_entriesList->currentRow() + 1, m_entriesList->count() - 1);
        m_entriesList->setCurrentRow(std::max(0, nextRow));
        event->accept();
        return;
    }

    case Qt::Key_Up: {
        const int previousRow = std::max(0, m_entriesList->currentRow() - 1);
        m_entriesList->setCurrentRow(previousRow);
        event->accept();
        return;
    }

    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_A:
    case Qt::Key_Space: {
        const int row = m_entriesList->currentRow();
        if (row < 0 || row >= m_entries.size()) {
            event->accept();
            return;
        }

        const BrowserEntry &entry = m_entries[row];
        if (entry.isDirectory || entry.isVolume) {
            openPath(entry.absolutePath, true);
        } else {
            installSelectedRom();
        }
        event->accept();
        return;
    }

    case Qt::Key_Left:
    case Qt::Key_Backspace:
    case Qt::Key_B:
        goToParent();
        event->accept();
        return;

    case Qt::Key_Q:
        goBackInHistory();
        event->accept();
        return;

    case Qt::Key_E:
        goForwardInHistory();
        event->accept();
        return;

    case Qt::Key_R:
        if (m_currentPath.isEmpty()) {
            loadVolumes();
        } else {
            rebuildEntries();
        }
        event->accept();
        return;

    case Qt::Key_Escape:
        emit backRequested();
        event->accept();
        return;

    default:
        QWidget::keyPressEvent(event);
        return;
    }
}

void BottomSettingsPage::loadVolumes()
{
    m_currentPath.clear();
    m_entries.clear();
    m_entriesList->clear();

    QSet<QString> seenRoots;

    const QFileInfoList roots = QDir::drives();
    for (const QFileInfo &rootInfo : roots) {
        const QString rootPath = QDir::cleanPath(rootInfo.absoluteFilePath());
        if (rootPath.isEmpty() || seenRoots.contains(rootPath)) {
            continue;
        }

        BrowserEntry entry;
        entry.label = QStringLiteral("[DISK] ") + rootPath;
        entry.absolutePath = rootPath;
        entry.isDirectory = true;
        entry.isVolume = true;

        m_entries.push_back(entry);
        seenRoots.insert(rootPath);
    }

    const QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();
    for (const QStorageInfo &volume : volumes) {
        if (!volume.isValid() || !volume.isReady()) {
            continue;
        }

        const QString rootPath = QDir::cleanPath(volume.rootPath());
        if (rootPath.isEmpty() || seenRoots.contains(rootPath)) {
            continue;
        }

        QString label = volume.displayName().trimmed();
        if (label.isEmpty()) {
            label = rootPath;
        }

        BrowserEntry entry;
        entry.label = QStringLiteral("[VOLUME] ") + label + QStringLiteral(" -> ") + rootPath;
        entry.absolutePath = rootPath;
        entry.isDirectory = true;
        entry.isVolume = true;

        m_entries.push_back(entry);
        seenRoots.insert(rootPath);
    }

    std::sort(m_entries.begin(), m_entries.end(), [](const BrowserEntry &left, const BrowserEntry &right) {
        return left.absolutePath < right.absolutePath;
    });

    for (const BrowserEntry &entry : std::as_const(m_entries)) {
        m_entriesList->addItem(entry.label);
    }

    if (m_entriesList->count() > 0) {
        m_entriesList->setCurrentRow(0);
    }

    m_history.clear();
    m_historyIndex = -1;

    m_pathLabel->setText(QStringLiteral("Volumes"));
    setStatus(QStringLiteral("Select a disk or USB volume"), StatusKind::Info);
}

void BottomSettingsPage::openPath(const QString &path, bool pushHistory)
{
    const QString cleanedPath = QDir::cleanPath(path);
    if (cleanedPath.isEmpty()) {
        return;
    }

    QFileInfo info(cleanedPath);
    if (!info.exists() || !info.isDir()) {
        setStatus(QStringLiteral("Folder unavailable: ") + cleanedPath, StatusKind::Error);
        return;
    }

    m_currentPath = cleanedPath;

    if (pushHistory) {
        if (m_historyIndex + 1 < m_history.size()) {
            m_history.resize(m_historyIndex + 1);
        }
        m_history.push_back(m_currentPath);
        m_historyIndex = m_history.size() - 1;
    }

    rebuildEntries();
}

void BottomSettingsPage::rebuildEntries()
{
    m_entries.clear();
    m_entriesList->clear();

    if (m_currentPath.isEmpty()) {
        loadVolumes();
        return;
    }

    QDir currentDir(m_currentPath);
    if (!currentDir.exists()) {
        setStatus(QStringLiteral("Folder not found: ") + m_currentPath, StatusKind::Error);
        loadVolumes();
        return;
    }

    m_pathLabel->setText(m_currentPath);

    const QFileInfoList childDirs = currentDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);

    bool containsNamedConsoleDir = false;
    for (const QFileInfo &dirInfo : childDirs) {
        const QString lowerName = dirInfo.fileName().toLower();
        if (lowerName == QStringLiteral("nds") || lowerName == QStringLiteral("gba")) {
            containsNamedConsoleDir = true;
            break;
        }
    }

    for (const QFileInfo &dirInfo : childDirs) {
        const QString lowerName = dirInfo.fileName().toLower();

        // If an nds/gba folder exists in this level, only show those folder names.
        if (containsNamedConsoleDir && lowerName != QStringLiteral("nds") && lowerName != QStringLiteral("gba")) {
            continue;
        }

        BrowserEntry entry;
        entry.label = QStringLiteral("[DIR] ") + dirInfo.fileName();
        entry.absolutePath = dirInfo.absoluteFilePath();
        entry.isDirectory = true;

        m_entries.push_back(entry);
    }

    const QFileInfoList files = currentDir.entryInfoList(QDir::Files, QDir::Name | QDir::IgnoreCase);
    for (const QFileInfo &fileInfo : files) {
        if (!isRomExtension(fileInfo.suffix())) {
            continue;
        }

        BrowserEntry entry;
        entry.label = QStringLiteral("[ROM] ") + fileInfo.fileName();
        entry.absolutePath = fileInfo.absoluteFilePath();
        entry.isDirectory = false;

        m_entries.push_back(entry);
    }

    for (const BrowserEntry &entry : std::as_const(m_entries)) {
        m_entriesList->addItem(entry.label);
    }

    if (m_entriesList->count() > 0) {
        m_entriesList->setCurrentRow(0);
    }

    setStatus(QStringLiteral("Browse folders, then install .nds/.gba from nds or gba folder"), StatusKind::Info);
}

void BottomSettingsPage::setStatus(const QString &message, StatusKind kind)
{
    QString statusPrefix;
    QString statusColor = QStringLiteral("white");

    if (kind == StatusKind::Loading) {
        statusPrefix = QStringLiteral("[LOADING] ");
        statusColor = QStringLiteral("#ffe08a");
    } else if (kind == StatusKind::Success) {
        statusPrefix = QStringLiteral("[SUCCESS] ");
        statusColor = QStringLiteral("#b7ffb0");
    } else if (kind == StatusKind::Error) {
        statusPrefix = QStringLiteral("[ERROR] ");
        statusColor = QStringLiteral("#ffb8b8");
    } else {
        statusPrefix = QStringLiteral("[INFO] ");
    }

    const QString formattedMessage = statusPrefix + message;

    qInfo().noquote() << "[Settings]" << formattedMessage;

    if (m_statusLabel != nullptr) {
        m_statusLabel->setText(formattedMessage);
        m_statusLabel->setStyleSheet(QStringLiteral("font-size: 15px; color: %1;").arg(statusColor));
    }

    if (kind == StatusKind::Loading) {
        showPopup(QStringLiteral("Installation en cours..."), kind, 0);
    } else if (kind == StatusKind::Success || kind == StatusKind::Error) {
        showPopup(formattedMessage, kind, 2400);
    }

    emit statusChanged(formattedMessage);
}

void BottomSettingsPage::showPopup(const QString &message, StatusKind kind, int autoHideMs)
{
    if (m_popupContainer == nullptr || m_popupLabel == nullptr) {
        return;
    }

    QString backgroundColor = QStringLiteral("rgba(0, 0, 0, 190)");
    QString borderColor = QStringLiteral("rgba(255, 255, 255, 140)");
    QString textColor = QStringLiteral("white");

    if (kind == StatusKind::Success) {
        borderColor = QStringLiteral("rgba(120, 255, 120, 220)");
        textColor = QStringLiteral("#d8ffd8");
    } else if (kind == StatusKind::Error) {
        borderColor = QStringLiteral("rgba(255, 120, 120, 220)");
        textColor = QStringLiteral("#ffd0d0");
    } else if (kind == StatusKind::Loading) {
        borderColor = QStringLiteral("rgba(255, 220, 120, 220)");
        textColor = QStringLiteral("#ffe8b8");
    }

    m_popupContainer->setStyleSheet(
        QStringLiteral("background: %1; border: 2px solid %2; border-radius: 10px;")
            .arg(backgroundColor, borderColor));
    m_popupLabel->setStyleSheet(QStringLiteral("color: %1; font-size: 14px; font-weight: 700;").arg(textColor));
    m_popupLabel->setText(message);
    m_popupContainer->show();
    m_popupContainer->raise();

    ++m_popupVersion;
    const int popupVersion = m_popupVersion;

    if (autoHideMs > 0) {
        QTimer::singleShot(autoHideMs, this, [this, popupVersion]() {
            if (popupVersion == m_popupVersion) {
                hidePopup();
            }
        });
    }
}

void BottomSettingsPage::hidePopup()
{
    if (m_popupContainer != nullptr) {
        m_popupContainer->hide();
    }
}

void BottomSettingsPage::goToParent()
{
    if (m_currentPath.isEmpty()) {
        return;
    }

    QDir dir(m_currentPath);
    if (!dir.cdUp()) {
        loadVolumes();
        return;
    }

    openPath(dir.absolutePath(), true);
}

void BottomSettingsPage::goBackInHistory()
{
    if (m_historyIndex <= 0) {
        setStatus(QStringLiteral("No older location in history"), StatusKind::Info);
        return;
    }

    --m_historyIndex;
    m_currentPath = m_history[m_historyIndex];
    rebuildEntries();
}

void BottomSettingsPage::goForwardInHistory()
{
    if (m_historyIndex < 0 || m_historyIndex + 1 >= m_history.size()) {
        setStatus(QStringLiteral("No newer location in history"), StatusKind::Info);
        return;
    }

    ++m_historyIndex;
    m_currentPath = m_history[m_historyIndex];
    rebuildEntries();
}

BottomSettingsPage::InstallTarget BottomSettingsPage::detectInstallTargetForPath(const QString &path) const
{
    QDir current(path);

    while (true) {
        const QString lowerName = current.dirName().toLower();
        if (lowerName == QStringLiteral("nds")) {
            return InstallTarget::Nds;
        }
        if (lowerName == QStringLiteral("gba")) {
            return InstallTarget::Gba;
        }

        if (!current.cdUp()) {
            break;
        }
    }

    return InstallTarget::None;
}

QString BottomSettingsPage::installTargetName(InstallTarget target) const
{
    if (target == InstallTarget::Nds) {
        return QStringLiteral("nds");
    }

    if (target == InstallTarget::Gba) {
        return QStringLiteral("gba");
    }

    return QString();
}

bool BottomSettingsPage::installSelectedRom()
{
    const int row = m_entriesList->currentRow();
    if (row < 0 || row >= m_entries.size()) {
        setStatus(QStringLiteral("Select a ROM file first"), StatusKind::Error);
        return false;
    }

    const BrowserEntry &entry = m_entries[row];
    if (entry.isDirectory) {
        setStatus(QStringLiteral("Select a ROM file, not a folder"), StatusKind::Error);
        return false;
    }

    setStatus(QStringLiteral("Installing ") + QFileInfo(entry.absolutePath).fileName() + QStringLiteral(" ..."), StatusKind::Loading);
    QCoreApplication::processEvents();

    const QFileInfo romInfo(entry.absolutePath);
    if (!romInfo.exists()) {
        setStatus(QStringLiteral("ROM file not found"), StatusKind::Error);
        return false;
    }

    const InstallTarget target = detectInstallTargetForPath(m_currentPath);
    if (target == InstallTarget::None) {
        setStatus(QStringLiteral("Open a folder named nds or gba before installing"), StatusKind::Error);
        return false;
    }

    const QString extension = romInfo.suffix().toLower();
    if ((target == InstallTarget::Nds && extension != QStringLiteral("nds")) ||
        (target == InstallTarget::Gba && extension != QStringLiteral("gba"))) {
        setStatus(QStringLiteral("ROM extension does not match current folder target"), StatusKind::Error);
        return false;
    }

    const QString destinationDirPath = QDir(Paths::library()).filePath(installTargetName(target));
    QDir destinationDir(destinationDirPath);
    if (!destinationDir.exists() && !QDir().mkpath(destinationDirPath)) {
        setStatus(QStringLiteral("Cannot create destination folder: ") + destinationDirPath, StatusKind::Error);
        return false;
    }

    const QString destinationRomPath = destinationDir.filePath(romInfo.fileName());
    if (QFile::exists(destinationRomPath) && !QFile::remove(destinationRomPath)) {
        setStatus(QStringLiteral("Cannot overwrite ROM: ") + destinationRomPath, StatusKind::Error);
        return false;
    }

    if (!QFile::copy(romInfo.absoluteFilePath(), destinationRomPath)) {
        setStatus(QStringLiteral("ROM copy failed"), StatusKind::Error);
        return false;
    }

    QString coverStatus = QStringLiteral("No matching cover found");
    const QString coverFileName = romInfo.completeBaseName() + QStringLiteral(".png");

    QString sourceCoverPath;
    const QString localCoverPath = QDir(QDir(m_currentPath).filePath(QStringLiteral("cover"))).filePath(coverFileName);
    if (QFile::exists(localCoverPath)) {
        sourceCoverPath = localCoverPath;
    } else {
        QDir parentDir(m_currentPath);
        if (parentDir.cdUp()) {
            const QString siblingCoverPath = QDir(parentDir.filePath(QStringLiteral("cover"))).filePath(coverFileName);
            if (QFile::exists(siblingCoverPath)) {
                sourceCoverPath = siblingCoverPath;
            }
        }
    }

    if (!sourceCoverPath.isEmpty()) {
        const QString iconsPath = Paths::icons();
        if (!QDir(iconsPath).exists()) {
            QDir().mkpath(iconsPath);
        }

        const QString destinationCoverPath = QDir(iconsPath).filePath(coverFileName);
        if (QFile::exists(destinationCoverPath)) {
            QFile::remove(destinationCoverPath);
        }

        if (QFile::copy(sourceCoverPath, destinationCoverPath)) {
            coverStatus = QStringLiteral("Cover installed: ") + coverFileName;
        } else {
            coverStatus = QStringLiteral("Cover found but copy failed");
        }
    }

    setStatus(QStringLiteral("Installed ") + romInfo.fileName() + QStringLiteral(" to ") + installTargetName(target) + QStringLiteral(". ") + coverStatus, StatusKind::Success);
    emit libraryChanged();
    return true;
}
