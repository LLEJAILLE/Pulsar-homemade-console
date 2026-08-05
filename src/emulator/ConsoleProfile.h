#pragma once

#include "../library/ConsoleType.h"

class ConsoleProfile
{
public:
    static ConsoleProfile forConsole(ConsoleType console);

    ConsoleType console() const;
    int screenCount() const;
    bool hasTouch() const;
    int touchScreenIndex() const;

private:
    ConsoleType m_console = ConsoleType::Unknown;
    int m_screenCount = 1;
    bool m_hasTouch = false;
    int m_touchScreenIndex = -1;
};