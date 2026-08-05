#pragma once

#include <QWidget>

class BatteryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BatteryWidget(QWidget *parent = nullptr);

    void setBatteryLevel(int level);
    int batteryLevel() const { return m_level; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_level = 100;
};
