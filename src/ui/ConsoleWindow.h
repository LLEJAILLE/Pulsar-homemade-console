#pragma once

#include <vector>

#include <QWidget>
#include <QList>

#include "library/Game.h"
#include "ui/pages/pages.hpp"

class TopScreen;
class BottomScreen;
class EmulatorPage;
class QScreen;

class ConsoleWindow : public QWidget
{
public:
    explicit ConsoleWindow(const std::vector<Game> &games, QWidget *parent = nullptr);
    ~ConsoleWindow();

private:
    std::vector<Game> m_games;
    TopScreen *m_topScreen;
    BottomScreen *m_bottomScreen;
    EmulatorPage *m_emulatorPage = nullptr;
    QWidget *m_separateBottomWindow = nullptr;
    bool m_isDualScreenMode = false;

    Page m_currentPage = Page::SplashScreen;
    Game *m_currentGame = nullptr;

    void setupDualScreenMode(const QList<QScreen *> &screens);
    void setupSingleScreenMode();
    void launchGame(const Game &game);
    void openSettings();
    void backToHome();
    void reloadLibrary();
    void bindHomeSignals();
    void bindSettingsSignals();
};
