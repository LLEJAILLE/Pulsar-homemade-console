#pragma once

#include <chrono>

namespace FrameTimingProfiler
{
enum class Stage
{
    Input,
    RetroRun,
    Framebuffer,
    Conversion,
    Rendering,
    Audio,
    Count
};

class ScopedTimer
{
    public:
        explicit ScopedTimer(Stage stage);
        ~ScopedTimer();

        ScopedTimer(const ScopedTimer&) = delete;
        ScopedTimer& operator=(const ScopedTimer&) = delete;

    private:
        Stage m_stage;
        std::chrono::steady_clock::time_point m_start;
};

void beginFrame(unsigned expectedPaintEvents);
void notePaintCompleted();

}