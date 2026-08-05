#pragma once

#include <QWidget>
#include <QString>
#include <QTimer>

#include "library/Game.h"

#include "ui/widgets/EmulatorScreenWidget.h"

class BackgroundWidget;
class QLabel;
class QResizeEvent;
class QHideEvent;
class QVBoxLayout;
class QKeyEvent;

class EmulatorPage : public QWidget
{
    Q_OBJECT

public:
    explicit EmulatorPage(const Game &game, QWidget *parent = nullptr);
    ~EmulatorPage() override;

    const Game& currentGame() const { return m_game; }

protected:
    void hideEvent(QHideEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

signals:
    void backToHome();

private:
    Game m_game;
    BackgroundWidget *m_background = nullptr;
    QWidget *m_overlay = nullptr;
    QLabel *m_gameTitle = nullptr;
    QLabel *m_statusLabel = nullptr;

    QTimer* m_emulatorTimer = nullptr;

    EmulatorScreenWidget* m_topScreen = nullptr;
    EmulatorScreenWidget* m_bottomScreen = nullptr;
};
