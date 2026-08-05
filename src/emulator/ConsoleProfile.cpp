#include "ConsoleProfile.h"

ConsoleProfile ConsoleProfile::forConsole(ConsoleType console)
{
    ConsoleProfile profile;
    profile.m_console = console;

    switch (console)
    {
        case ConsoleType::NDS:
            profile.m_screenCount = 2;
            profile.m_hasTouch = true;
            profile.m_touchScreenIndex = 1;
            break;

        case ConsoleType::GBA:
            profile.m_screenCount = 1;
            profile.m_hasTouch = false;
            profile.m_touchScreenIndex = -1;
            break;

        default:
            profile.m_screenCount = 1;
            profile.m_hasTouch = false;
            profile.m_touchScreenIndex = -1;
            break;
    }

    return profile;
}

ConsoleType ConsoleProfile::console() const
{
    return m_console;
}

int ConsoleProfile::screenCount() const
{
    return m_screenCount;
}

bool ConsoleProfile::hasTouch() const
{
    return m_hasTouch;
}

int ConsoleProfile::touchScreenIndex() const
{
    return m_touchScreenIndex;
}