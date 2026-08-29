#include "ConsoleWindow.h"

#include <QVBoxLayout>
#include <QTimer>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>

#include <QString>
#include <filesystem>

#include "pages/BottomHomePage.h"
#include "pages/BottomSettingsPage.h"
#include "pages/EmulatorPage.h"
#include "emulator/EmulatorManager.h"
#include "screens/BottomScreen.h"
#include "screens/TopScreen.h"

#include "audio/AudioManager.h"
#include "library/LibraryManager.h"
#include "utils/Paths.h"

ConsoleWindow::ConsoleWindow(const std::vector<Game> &games, QWidget *parent) : QWidget(parent) , m_games(games) , m_topScreen(new TopScreen(this)) , m_bottomScreen(new BottomScreen(this))
{
    setStyleSheet(QStringLiteral("background-color: #000000;"));

    // Detect number of screens and setup dual-screen mode if available
    auto screens = QGuiApplication::screens();
    if (screens.size() >= 2) {
        m_isDualScreenMode = true;
        setupDualScreenMode(screens);
    } else {
        setupSingleScreenMode();
    }

    m_topScreen->loadPage(m_currentPage);
    m_bottomScreen->loadPage(m_currentPage, m_games);
    
    // Set focus to bottom screen for keyboard input (menu navigation)
    m_bottomScreen->setFocus();

    if (m_currentPage == Page::SplashScreen) {
        AudioManager::instance().playSplashScreen();
        QTimer::singleShot(4000, this, [this]() {
            m_currentPage = Page::Home;

            m_topScreen->loadPage(m_currentPage);
            m_bottomScreen->loadPage(m_currentPage, m_games);
            bindHomeSignals();
        });
    }
}

ConsoleWindow::~ConsoleWindow()
{
    if (m_separateBottomWindow) {
        m_separateBottomWindow->deleteLater();
    }
    if (m_separateEmulatorGameWindow) {
        m_separateEmulatorGameWindow->deleteLater();
    }
}

void ConsoleWindow::setupDualScreenMode(const QList<QScreen *> &screens)
{
    // Create separate window for BottomScreen on second display
    m_separateBottomWindow = new QWidget(nullptr);
    m_separateBottomWindow->setStyleSheet(QStringLiteral("background-color: #000000;"));
    
    auto *bottomLayout = new QVBoxLayout(m_separateBottomWindow);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(0);
    bottomLayout->addWidget(m_bottomScreen);
    
    // Setup TopScreen on first display (this window)
    auto *topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(0);
    topLayout->addWidget(m_topScreen);
    
    // Position and show windows on their respective screens
    // TopScreen on first screen
    const QRect screen1Geometry = screens[0]->geometry();
    this->setGeometry(screen1Geometry);
    this->showFullScreen();
    
    // BottomScreen on second screen
    const QRect screen2Geometry = screens[1]->geometry();
    m_separateBottomWindow->setGeometry(screen2Geometry);
    m_separateBottomWindow->showFullScreen();
}

void ConsoleWindow::setupSingleScreenMode()
{
    // Traditional vertical layout for single screen
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_topScreen, 1);
    layout->addWidget(m_bottomScreen, 1);
    
    // Show fullscreen on startup
    this->showFullScreen();
}

void ConsoleWindow::launchGame(const Game &game)
{
    m_currentPage = Page::Emulator;
    m_currentGame = const_cast<Game *>(&game);

    if (m_emulatorPage) {
        m_emulatorPage->hide();
        m_emulatorPage->deleteLater();
        m_emulatorPage = nullptr;
    }
    
    if (m_separateEmulatorGameWindow) {
        m_separateEmulatorGameWindow->hide();
        m_separateEmulatorGameWindow->deleteLater();
        m_separateEmulatorGameWindow = nullptr;
    }

    // Create EmulatorPage WITHOUT parent so fullscreen works correctly
    m_emulatorPage = new EmulatorPage(game, nullptr);
    connect(m_emulatorPage, &EmulatorPage::backToHome, this, &ConsoleWindow::backToHome);

    m_topScreen->hide();
    m_bottomScreen->hide();
    if (m_separateBottomWindow) {
        m_separateBottomWindow->hide();
    }
    
    // In dual-screen mode, separate the game screens across two displays
    if (m_isDualScreenMode) {
        auto screens = QGuiApplication::screens();
        if (screens.size() >= 2) {
            // Show top screen of game on first screen
            const QRect screen1Geometry = screens[0]->geometry();
            m_emulatorPage->setGeometry(screen1Geometry);
            m_emulatorPage->showFullScreen();
            
            // Get the bottom screen widget and move it to second display
            QWidget *gameBottomScreen = m_emulatorPage->getBottomScreenWidget();
            if (gameBottomScreen) {
                // Create a separate window for the bottom screen on the second display
                m_separateEmulatorGameWindow = new QWidget(nullptr);
                m_separateEmulatorGameWindow->setStyleSheet(QStringLiteral("background-color: #000000;"));
                
                auto *bottomLayout = new QVBoxLayout(m_separateEmulatorGameWindow);
                bottomLayout->setContentsMargins(0, 0, 0, 0);
                bottomLayout->setSpacing(0);
                bottomLayout->addWidget(gameBottomScreen);
                
                // Show on second screen
                const QRect screen2Geometry = screens[1]->geometry();
                m_separateEmulatorGameWindow->setGeometry(screen2Geometry);
                m_separateEmulatorGameWindow->showFullScreen();
            }
        } else {
            m_emulatorPage->showFullScreen();
        }
    } else {
        // Single screen mode: show game in fullscreen
        m_emulatorPage->showFullScreen();
    }
    
    m_emulatorPage->setFocus();
}

void ConsoleWindow::backToHome()
{
    m_currentPage = Page::Home;
    m_currentGame = nullptr;

    if (m_emulatorPage) {
        m_emulatorPage->hide();
        EmulatorManager::instance().stop();
        m_emulatorPage->deleteLater();
        m_emulatorPage = nullptr;
    }
    
    if (m_separateEmulatorGameWindow) {
        m_separateEmulatorGameWindow->hide();
        m_separateEmulatorGameWindow->deleteLater();
        m_separateEmulatorGameWindow = nullptr;
    }

    m_topScreen->show();
    m_bottomScreen->show();
    if (m_separateBottomWindow) {
        m_separateBottomWindow->show();
    }
    m_topScreen->loadPage(m_currentPage);
    m_bottomScreen->loadPage(m_currentPage, m_games);
    bindHomeSignals();
    m_bottomScreen->setFocus();
}

void ConsoleWindow::openSettings()
{
    m_currentPage = Page::Settings;
    m_topScreen->loadPage(m_currentPage);
    m_bottomScreen->loadPage(m_currentPage, m_games);
    m_topScreen->setSettingsStatus(QStringLiteral("Select a disk or USB folder"));
    bindSettingsSignals();
    if (auto *settingsPage = m_bottomScreen->findChild<BottomSettingsPage *>()) {
        QTimer::singleShot(0, settingsPage, [settingsPage]() {
            settingsPage->setFocus(Qt::OtherFocusReason);
        });
    } else {
        m_bottomScreen->setFocus();
    }
}

void ConsoleWindow::reloadLibrary()
{
    LibraryManager libraryManager;
    const QByteArray libraryPathUtf8 = Paths::library().toUtf8();
    libraryManager.scan(std::filesystem::u8path(libraryPathUtf8.constData()));
    m_games = libraryManager.games();
}

void ConsoleWindow::bindHomeSignals()
{
    if (auto *bottomHomePage = m_bottomScreen->findChild<BottomHomePage *>()) {
        connect(bottomHomePage, &BottomHomePage::selectedGameChanged, m_topScreen, &TopScreen::setGameTitle);
        connect(bottomHomePage, &BottomHomePage::launchGame, this, &ConsoleWindow::launchGame);
        connect(bottomHomePage, &BottomHomePage::openSettingsRequested, this, &ConsoleWindow::openSettings);

        if (!m_games.empty()) {
            m_topScreen->setGameTitle(QString::fromStdString(m_games.front().title));
        } else {
            m_topScreen->setGameTitle(QStringLiteral("Settings"));
        }
    }
}

void ConsoleWindow::bindSettingsSignals()
{
    if (auto *settingsPage = m_bottomScreen->findChild<BottomSettingsPage *>()) {
        connect(settingsPage, &BottomSettingsPage::backRequested, this, &ConsoleWindow::backToHome);
        connect(settingsPage, &BottomSettingsPage::statusChanged, m_topScreen, &TopScreen::setSettingsStatus);
        connect(settingsPage, &BottomSettingsPage::libraryChanged, this, [this]() {
            reloadLibrary();
        });
    }
}
