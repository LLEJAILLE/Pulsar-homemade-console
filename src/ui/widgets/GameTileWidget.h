#pragma once

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

#include "library/Game.h"

class GameTileWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int yOffset READ yOffset WRITE setYOffset)

public:
    explicit GameTileWidget(const Game& game, QWidget* parent = nullptr);

    void setTileSize(int size);
    void setSelected(bool selected);

    QSize sizeHint() const override;

    int yOffset() const { return m_yOffset; }
    void setYOffset(int value);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QPixmap loadCover(const QString& gameName);
    int m_yOffset = 0;
    QPropertyAnimation* m_animation = nullptr;

private:
    Game m_game;

    QPixmap m_cover;

    int m_tileSize = 0;
    bool m_selected = false;
    QGraphicsDropShadowEffect* m_shadow = nullptr;
};