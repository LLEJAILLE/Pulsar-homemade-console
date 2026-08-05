#pragma once
#include <QWidget>
#include <QMouseEvent>

#include "../../emulator/ConsoleProfile.h"

class EmulatorScreenWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit EmulatorScreenWidget(const ConsoleProfile& profile, int screenIndex, QWidget* parent = nullptr);

    protected:
        void paintEvent(QPaintEvent*) override;

        void mousePressEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;

    private:
        ConsoleProfile m_profile;
        int m_screenIndex;
        bool isActiveScreen() const;
};