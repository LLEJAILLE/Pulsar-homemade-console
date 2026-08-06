#include "InputManager.h"

std::array<std::atomic<bool>, InputManager::kButtonCount> InputManager::s_buttons{};
std::atomic<bool> InputManager::s_touchPressed{false};
std::atomic<int> InputManager::s_touchX{0};
std::atomic<int> InputManager::s_touchY{0};

std::size_t InputManager::indexOf(Button button)
{
    return static_cast<std::size_t>(button);
}

void InputManager::setButton(Button button, bool pressed)
{
    s_buttons[indexOf(button)].store(pressed, std::memory_order_relaxed);
}

bool InputManager::button(Button button)
{
    return s_buttons[indexOf(button)].load(std::memory_order_relaxed);
}

void InputManager::clearButtons()
{
    for (auto &buttonState : s_buttons) {
        buttonState.store(false, std::memory_order_relaxed);
    }
}

void InputManager::pressTouch(int x, int y)
{
    s_touchX.store(x, std::memory_order_relaxed);
    s_touchY.store(y, std::memory_order_relaxed);
    s_touchPressed.store(true, std::memory_order_relaxed);
}

void InputManager::moveTouch(int x, int y)
{
    s_touchX.store(x, std::memory_order_relaxed);
    s_touchY.store(y, std::memory_order_relaxed);
}

void InputManager::releaseTouch()
{
    s_touchPressed.store(false, std::memory_order_relaxed);
}

bool InputManager::touchPressed()
{
    return s_touchPressed.load(std::memory_order_relaxed);
}

int InputManager::touchX()
{
    return s_touchX.load(std::memory_order_relaxed);
}

int InputManager::touchY()
{
    return s_touchY.load(std::memory_order_relaxed);
}