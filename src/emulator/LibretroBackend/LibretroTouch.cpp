#include "LibretroTouch.h"

#include "LibretroVideo.h"
#include "../../input/InputManager.h"

#include "Libretro.h"
#include <algorithm>
#include <cmath>

int16_t LibretroTouch::state(unsigned device, unsigned id)
{
    if (device != RETRO_DEVICE_POINTER)
        return 0;

    const QImage &frame = LibretroVideo::frame();
    const int contentWidth = frame.isNull() ? 256 : frame.width();
    const int contentHeight = frame.isNull() ? 384 : frame.height();

    const auto toLibretroCoordinate = [](int value, int size) -> int16_t
    {
        if (size <= 1)
            return 0;

        const double normalized = (static_cast<double>(value) / static_cast<double>(size - 1)) * 2.0 - 1.0;
        const double scaled = std::clamp(normalized * 32767.0, -32767.0, 32767.0);

        return static_cast<int16_t>(std::lround(scaled));
    };

    switch (id)
    {
        case RETRO_DEVICE_ID_POINTER_X:
            return toLibretroCoordinate(InputManager::touchX(), contentWidth);

        case RETRO_DEVICE_ID_POINTER_Y:
            return toLibretroCoordinate(InputManager::touchY(), contentHeight);

        case RETRO_DEVICE_ID_POINTER_PRESSED:
            return InputManager::touchPressed() ? 1 : 0;
    }

    return 0;
}