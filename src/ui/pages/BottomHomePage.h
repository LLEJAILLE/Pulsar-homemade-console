#pragma once
#include "library/Game.h"
#include "../widgets/BackgroundWidget.h"
#include "../widgets/BatteryWidget.h"
#include "../../audio/AudioManager.h"

#include <vector>
#include <QString>
#include <QWidget>
#include <QPropertyAnimation>

class GameTileWidget;
class QLabel;
class QHBoxLayout;
class QKeyEvent;
class QResizeEvent;
class QScrollArea;
class QShowEvent;
class QTimer;
class BatteryWidget;

class BottomHomePage : public QWidget
{
    Q_OBJECT

public:
    explicit BottomHomePage(const std::vector<Game> &games, QWidget *parent = nullptr);

signals:
    void selectedGameChanged(const QString &gameTitle);
    void launchGame(const Game &game);
    void openSettingsRequested();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void updateTileSizes();
    void updateSelection();
    void centerCurrentTile();
    void setCurrentIndex(int newIndex);

    void updateCurrentTime();
    void updateBatteryLevel();

    std::string currentTime() const;
    
    std::string currentTimeString = currentTime();
    int m_batteryLevel = 100;
    

    QScrollArea *m_scrollArea = nullptr;
    QWidget *m_container = nullptr;
    QHBoxLayout *m_gamesLayout = nullptr;
    std::vector<GameTileWidget *> m_tiles;
    std::vector<QString> m_gameTitles;
    std::vector<Game> m_games;
    int m_currentIndex = -1;
    int m_settingsIndex = -1;

    BackgroundWidget* m_background = nullptr;
    QWidget* m_overlay = nullptr;
    QLabel* m_hourLabel = nullptr;
    BatteryWidget* m_batteryWidget = nullptr;
    QTimer* m_timeTimer = nullptr;

    QPropertyAnimation* m_scrollAnimation = nullptr;
    AudioManager &audioManager = AudioManager::instance();
    
};