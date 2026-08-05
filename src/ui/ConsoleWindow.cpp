#include "ConsoleWindow.h"

#include <QVBoxLayout>
#include <QTimer>

#include <QString>

#include "pages/BottomHomePage.h"
#include "pages/EmulatorPage.h"
#include "emulator/EmulatorManager.h"
#include "screens/BottomScreen.h"
#include "screens/TopScreen.h"

#include "audio/AudioManager.h"

ConsoleWindow::ConsoleWindow(const std::vector<Game> &games, QWidget *parent) : QWidget(parent) , m_games(games) , m_topScreen(new TopScreen(this)) , m_bottomScreen(new BottomScreen(this))
{
    setStyleSheet(QStringLiteral("background-color: #000000;"));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_topScreen, 1);
    layout->addWidget(m_bottomScreen, 1);

    m_topScreen->loadPage(m_currentPage);
    m_bottomScreen->loadPage(m_currentPage, m_games);

    if (m_currentPage == Page::SplashScreen) {
        AudioManager::instance().playSplashScreen();
        QTimer::singleShot(4000, this, [this]() {
            m_currentPage = Page::Home;

            m_topScreen->loadPage(m_currentPage);
            m_bottomScreen->loadPage(m_currentPage, m_games);

            if (auto *bottomHomePage = m_bottomScreen->findChild<BottomHomePage *>()) {
                connect(bottomHomePage, &BottomHomePage::selectedGameChanged, m_topScreen, &TopScreen::setGameTitle);
                connect(bottomHomePage, &BottomHomePage::launchGame, this, &ConsoleWindow::launchGame);

                if (!m_games.empty())
                {
                    m_topScreen->setGameTitle(QString::fromStdString(m_games.front().title));
                }
            }
        });
    }
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

    m_emulatorPage = new EmulatorPage(game, this);
    connect(m_emulatorPage, &EmulatorPage::backToHome, this, &ConsoleWindow::backToHome);

    m_topScreen->hide();
    m_bottomScreen->hide();
    m_emulatorPage->setGeometry(rect());
    m_emulatorPage->show();
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

    m_topScreen->show();
    m_bottomScreen->show();
    m_bottomScreen->setFocus();
}
