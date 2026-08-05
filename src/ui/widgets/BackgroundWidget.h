#pragma once

#include <QWidget>
#include <QTimer>
#include <QVector>

class QGraphicsBlurEffect;

struct Bubble
{
    QPointF position;
    float radius;
    float alpha;
    float speed;
    float phase;
};

class BackgroundWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BackgroundWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void updateAnimation();

    QVector<Bubble> m_bubbles;

    QTimer* m_timer = nullptr;
    QGraphicsBlurEffect* m_blur = nullptr;

    float m_time = 0.f;
};