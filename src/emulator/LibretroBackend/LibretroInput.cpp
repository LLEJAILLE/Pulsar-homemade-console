#include "LibretroInput.h"

#include "../../input/InputManager.h"

#include <array>

namespace
{
struct ButtonMapEntry
{
    unsigned libretroId;
    InputManager::Button button;
};

constexpr std::array<ButtonMapEntry, 12> kButtonMap = {{
    { RETRO_DEVICE_ID_JOYPAD_A, InputManager::Button::A },
    { RETRO_DEVICE_ID_JOYPAD_B, InputManager::Button::B },
    { RETRO_DEVICE_ID_JOYPAD_X, InputManager::Button::X },
    { RETRO_DEVICE_ID_JOYPAD_Y, InputManager::Button::Y },
    { RETRO_DEVICE_ID_JOYPAD_START, InputManager::Button::Start },
    { RETRO_DEVICE_ID_JOYPAD_SELECT, InputManager::Button::Select },
    { RETRO_DEVICE_ID_JOYPAD_UP, InputManager::Button::Up },
    { RETRO_DEVICE_ID_JOYPAD_DOWN, InputManager::Button::Down },
    { RETRO_DEVICE_ID_JOYPAD_LEFT, InputManager::Button::Left },
    { RETRO_DEVICE_ID_JOYPAD_RIGHT, InputManager::Button::Right },
    { RETRO_DEVICE_ID_JOYPAD_L, InputManager::Button::L },
    { RETRO_DEVICE_ID_JOYPAD_R, InputManager::Button::R },
}};
}

int16_t LibretroInput::state(unsigned id)
{
    for (const auto& entry : kButtonMap) {
        if (entry.libretroId == id) {
            return InputManager::button(entry.button) ? 1 : 0;
        }
    }

    return 0;
}