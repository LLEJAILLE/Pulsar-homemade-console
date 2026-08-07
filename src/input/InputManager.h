#pragma once

#include <array>
#include <atomic>
#include <cstddef>

class InputManager
{
public:
    enum class Button : std::size_t
    {
        A = 0,
        B,
        X,
        Y,
        Start,
        Select,
        Up,
        Down,
        Left,
        Right,
        L,
        R,
        Menu,
    };

    static constexpr std::size_t kButtonCount = static_cast<std::size_t>(Button::Menu) + 1;

    static void setButton(Button button, bool pressed);
    static bool button(Button button);
    static void clearButtons();

    static void pressTouch(int x, int y);
    static void moveTouch(int x, int y);
    static void releaseTouch();

    static bool touchPressed();
    static int touchX();
    static int touchY();

private:
    static std::size_t indexOf(Button button);

    static std::array<std::atomic<bool>, kButtonCount> s_buttons;
    static std::atomic<bool> s_touchPressed;
    static std::atomic<int> s_touchX;
    static std::atomic<int> s_touchY;
};