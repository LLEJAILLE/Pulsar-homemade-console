#pragma once

#include <vector>

#include <QWidget>

#include "library/Game.h"
#include "ui/pages/pages.hpp"

class TopScreen;
class BottomScreen;
class EmulatorPage;

class ConsoleWindow : public QWidget
{
public:
    explicit ConsoleWindow(const std::vector<Game> &games, QWidget *parent = nullptr);

private:
    std::vector<Game> m_games;
    TopScreen *m_topScreen;
    BottomScreen *m_bottomScreen;
    EmulatorPage *m_emulatorPage = nullptr;

    Page m_currentPage = Page::SplashScreen;
    Game *m_currentGame = nullptr;

    void launchGame(const Game &game);
    void backToHome();
};
