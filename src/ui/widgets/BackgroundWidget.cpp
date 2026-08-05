#include "BackgroundWidget.h"

#include <QGraphicsBlurEffect>
#include <QPainter>
#include <QRandomGenerator>


#include <cmath>

BackgroundWidget::BackgroundWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);

    m_blur = new QGraphicsBlurEffect(this);
    m_blur->setBlurRadius(18);
    m_blur->setBlurHints(QGraphicsBlurEffect::QualityHint);
    setGraphicsEffect(m_blur);

    m_timer = new QTimer(this);

    connect(
        m_timer,
        &QTimer::timeout,
        this,
        &BackgroundWidget::updateAnimation);

    m_timer->start(16);

    for (int i = 0; i < 20; ++i)
    {
        Bubble b;

        b.position = QPointF(
            QRandomGenerator::global()->bounded(2000),
            QRandomGenerator::global()->bounded(1200));

        b.radius = QRandomGenerator::global()->bounded(30, 120);

        b.alpha = QRandomGenerator::global()->bounded(10, 35);

        b.speed = QRandomGenerator::global()->bounded(5, 20) / 100.f;

        b.phase = QRandomGenerator::global()->generateDouble() * 6.28;

        m_bubbles.push_back(b);
    }
}

void BackgroundWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(12, 12, 12));

    for (const Bubble &bubble : m_bubbles)
    {
        float offset =
            std::sin(
                m_time * bubble.speed +
                bubble.phase) * 15.f;

        QPointF pos(
            bubble.position.x(),
            bubble.position.y() + offset);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, bubble.alpha));

        painter.drawEllipse(
            pos,
            bubble.radius,
            bubble.radius);
    }
}

void BackgroundWidget::updateAnimation()
{
    m_time += 0.016f;

    for (Bubble &bubble : m_bubbles)
    {
        bubble.position.setY(
            bubble.position.y() - bubble.speed);

        if (bubble.position.y() + bubble.radius < 0)
        {
            bubble.position.setY(height() + bubble.radius);

            bubble.position.setX(
                QRandomGenerator::global()->bounded(width()));
        }
    }

    update();
}