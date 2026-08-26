#include "LibretroInput.h"

#include "../../input/InputManager.h"

#include <array>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>

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

bool profileInputTiming()
{
    static const bool enabled = []() {
        const char* env = std::getenv("PULSAR_LIBRETRO_PROFILE");
        return env && std::strcmp(env, "0") != 0;
    }();
    return enabled;
}
}

int16_t LibretroInput::state(unsigned id)
{
    const auto start = std::chrono::steady_clock::now();

    for (const auto& entry : kButtonMap) {
        if (entry.libretroId == id) {
            const int16_t value = InputManager::button(entry.button) ? 1 : 0;

            if (profileInputTiming()) {
                static std::atomic<std::uint64_t> calls{0};
                static std::atomic<std::uint64_t> totalNs{0};
                const auto end = std::chrono::steady_clock::now();
                const auto durationNs = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                calls.fetch_add(1, std::memory_order_relaxed);
                totalNs.fetch_add(static_cast<std::uint64_t>(durationNs), std::memory_order_relaxed);
                if ((calls.load(std::memory_order_relaxed) % 256u) == 0u) {
                    const double avgUs = static_cast<double>(totalNs.load(std::memory_order_relaxed)) / 1000.0 / static_cast<double>(calls.load(std::memory_order_relaxed));
                    std::cout << "[libretro input avg] " << avgUs << " us" << std::endl;
                }
            }

            return value;
        }
    }

    return 0;
}