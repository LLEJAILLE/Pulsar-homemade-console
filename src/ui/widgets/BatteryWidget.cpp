#include "BatteryWidget.h"

#include <QPainter>
#include <QFont>
#include <algorithm>

BatteryWidget::BatteryWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(100, 20);
}

void BatteryWidget::setBatteryLevel(int level)
{
    m_level = std::clamp(level, 0, 100);
    update();
}

void BatteryWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw battery outline
    const int batteryWidth = 28;
    const int batteryHeight = 14;
    const int batteryX = 2;
    const int batteryY = (height() - batteryHeight) / 2;

    // Battery body
    painter.setPen(QPen(QColor(255, 255, 255), 1.5));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(batteryX, batteryY, batteryWidth, batteryHeight);

    // Battery terminal
    painter.drawRect(batteryX + batteryWidth, batteryY + 4, 2, 6);

    // Battery fill
    const int fillWidth = (batteryWidth - 2) * m_level / 100;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255));
    painter.drawRect(batteryX + 2, batteryY + 2, fillWidth, batteryHeight - 4);

    // Draw percentage text
    painter.setPen(QColor(255, 255, 255));
    QFont font;
    font.setPointSize(10);
    font.setWeight(QFont::Bold);
    painter.setFont(font);

    painter.drawText(
        batteryX + batteryWidth + 12,
        0,
        width() - batteryWidth - 14,
        height(),
        Qt::AlignVCenter | Qt::AlignLeft,
        QString::number(m_level) + "%");
}
