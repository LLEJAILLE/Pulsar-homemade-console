#pragma once

#include <QImage>
#include <QString>

#include "ConsoleProfile.h"
#include "../library/ConsoleType.h"
#include "LibretroBackend/LibretroCore.h"

class EmulatorManager
{
    public:
        static EmulatorManager& instance();

        bool initialize(ConsoleType console);
        bool loadRom(const QString&);

        void runFrame();
        void stop();

        QImage topScreen() const;
        QImage bottomScreen() const;
        const ConsoleProfile& profile() const;

    private:
        EmulatorManager() = default;

        ConsoleProfile m_profile;
        LibretroCore m_core;
};