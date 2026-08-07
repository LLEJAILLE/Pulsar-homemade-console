#include "FrameTimingProfiler.h"

#include <array>
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <mutex>

namespace FrameTimingProfiler
{
namespace
{
using Clock = std::chrono::steady_clock;
using Nanoseconds = std::chrono::nanoseconds;

struct FrameState
{
    bool active = false;
    unsigned expectedPaintEvents = 0;
    unsigned completedPaintEvents = 0;
    Clock::time_point frameStart{};
    std::array<Nanoseconds, static_cast<size_t>(Stage::Count)> stageDurations{};
};

struct WindowStats
{
    std::uint64_t frameCount = 0;
    std::uint64_t totalFrameNs = 0;
    std::uint64_t syncNs = 0;
    std::array<std::uint64_t, static_cast<size_t>(Stage::Count)> stageNs{};
    Clock::time_point windowStart = Clock::now();
};

std::mutex g_mutex;
FrameState g_frameState;
WindowStats g_windowStats;

double toMilliseconds(std::uint64_t nanoseconds)
{
    return static_cast<double>(nanoseconds) / 1000000.0;
}

double averageMilliseconds(std::uint64_t totalNanoseconds, std::uint64_t frameCount)
{
    if (frameCount == 0) {
        return 0.0;
    }

    return static_cast<double>(totalNanoseconds) / static_cast<double>(frameCount) / 1000000.0;
}

void printWindowStatsLocked(Clock::time_point now)
{
    const auto elapsed = std::chrono::duration_cast<Nanoseconds>(now - g_windowStats.windowStart);
    if (elapsed < std::chrono::seconds(1) || g_windowStats.frameCount == 0) {
        return;
    }

    const double elapsedSeconds = static_cast<double>(elapsed.count()) / 1000000000.0;
    const double frames = static_cast<double>(g_windowStats.frameCount);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "FPS : " << (frames / elapsedSeconds) << '\n';
    std::cout << "input : " << averageMilliseconds(g_windowStats.stageNs[static_cast<size_t>(Stage::Input)], g_windowStats.frameCount) << " ms\n";
    std::cout << "retro_run : " << averageMilliseconds(g_windowStats.stageNs[static_cast<size_t>(Stage::RetroRun)], g_windowStats.frameCount) << " ms\n";
    std::cout << "framebuffer : " << averageMilliseconds(g_windowStats.stageNs[static_cast<size_t>(Stage::Framebuffer)], g_windowStats.frameCount) << " ms\n";
    std::cout << "conversion : " << averageMilliseconds(g_windowStats.stageNs[static_cast<size_t>(Stage::Conversion)], g_windowStats.frameCount) << " ms\n";
    std::cout << "rendering : " << averageMilliseconds(g_windowStats.stageNs[static_cast<size_t>(Stage::Rendering)], g_windowStats.frameCount) << " ms\n";
    std::cout << "audio : " << averageMilliseconds(g_windowStats.stageNs[static_cast<size_t>(Stage::Audio)], g_windowStats.frameCount) << " ms\n";
    std::cout << "sync : " << averageMilliseconds(g_windowStats.syncNs, g_windowStats.frameCount) << " ms\n";
    std::cout << "total frame : " << averageMilliseconds(g_windowStats.totalFrameNs, g_windowStats.frameCount) << " ms\n";
    std::cout << std::flush;

    g_windowStats = WindowStats{};
    g_windowStats.windowStart = now;
}

void recordLocked(Stage stage, Nanoseconds duration)
{
    g_frameState.stageDurations[static_cast<size_t>(stage)] += duration;
    g_windowStats.stageNs[static_cast<size_t>(stage)] += static_cast<std::uint64_t>(duration.count());
}

void finishFrameLocked(Clock::time_point now)
{
    if (!g_frameState.active) {
        return;
    }

    const auto frameDuration = std::chrono::duration_cast<Nanoseconds>(now - g_frameState.frameStart);
    const auto activeDuration = std::accumulate(
        g_frameState.stageDurations.begin(),
        g_frameState.stageDurations.end(),
        Nanoseconds::zero());
    const auto syncDuration = frameDuration > activeDuration ? frameDuration - activeDuration : Nanoseconds::zero();

    g_windowStats.totalFrameNs += static_cast<std::uint64_t>(frameDuration.count());
    g_windowStats.syncNs += static_cast<std::uint64_t>(syncDuration.count());
    ++g_windowStats.frameCount;

    g_frameState = FrameState{};
    printWindowStatsLocked(now);
}
}

ScopedTimer::ScopedTimer(Stage stage)
    : m_stage(stage)
    , m_start(Clock::now())
{
}

ScopedTimer::~ScopedTimer()
{
    const auto end = Clock::now();
    const auto duration = std::chrono::duration_cast<Nanoseconds>(end - m_start);

    std::lock_guard<std::mutex> lock(g_mutex);
    recordLocked(m_stage, duration);
}

void beginFrame(unsigned expectedPaintEvents)
{
    const auto now = Clock::now();

    std::lock_guard<std::mutex> lock(g_mutex);

    if (g_frameState.active) {
        finishFrameLocked(now);
    }

    g_frameState = FrameState{};
    g_frameState.active = true;
    g_frameState.expectedPaintEvents = std::max(1u, expectedPaintEvents);
    g_frameState.frameStart = now;
}

void notePaintCompleted()
{
    const auto now = Clock::now();

    std::lock_guard<std::mutex> lock(g_mutex);

    if (!g_frameState.active) {
        return;
    }

    ++g_frameState.completedPaintEvents;
    if (g_frameState.completedPaintEvents >= g_frameState.expectedPaintEvents) {
        finishFrameLocked(now);
    }
}

}