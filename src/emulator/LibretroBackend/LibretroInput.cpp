#include "LibretroInput.h"

#include "../../input/InputManager.h"

namespace
{
bool mapLibretroButton(unsigned id, InputManager::Button &button)
{
    switch (id)
    {
        case RETRO_DEVICE_ID_JOYPAD_A:
            button = InputManager::Button::A;
            return true;

        case RETRO_DEVICE_ID_JOYPAD_B:
            button = InputManager::Button::B;
            return true;

        case RETRO_DEVICE_ID_JOYPAD_X:
            button = InputManager::Button::X;
            return true;

        case RETRO_DEVICE_ID_JOYPAD_Y:
            button = InputManager::Button::Y;
            return true;

        case RETRO_DEVICE_ID_JOYPAD_START:
            button = InputManager::Button::Start;
            return true;

        case RETRO_DEVICE_ID_JOYPAD_SELECT:
            button = InputManager::Button::Select;
            return true;

        case RETRO_DEVICE_ID_JOYPAD_UP:
            button = InputManager::Button::Up;
            return true;

        case RETRO_DEVICE_ID_JOYPAD_DOWN:
            button = InputManager::Button::Down;
            return true;

        case RETRO_DEVICE_ID_JOYPAD_LEFT:
            button = InputManager::Button::Left;
            return true;

        case RETRO_DEVICE_ID_JOYPAD_RIGHT:
            button = InputManager::Button::Right;
            return true;

        case RETRO_DEVICE_ID_JOYPAD_L:
            button = InputManager::Button::L;
            return true;

        case RETRO_DEVICE_ID_JOYPAD_R:
            button = InputManager::Button::R;
            return true;

        default:
            return false;
    }
}
}

int16_t LibretroInput::state(unsigned id)
{
    InputManager::Button button;
    if (!mapLibretroButton(id, button))
        return 0;

    return InputManager::button(button) ? 1 : 0;
}