#include "LibretroCore.h"
#include "LibretroVideo.h"
#include "LibretroInput.h"
#include "LibretroTouch.h"
#include "LibretroAudio.h"
#include "utils/FrameTimingProfiler.h"

#include <QFile>
#include <QFileInfo>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>

namespace
{
struct CallbackStats
{
    std::atomic<std::uint64_t> calls{0};
    std::atomic<std::uint64_t> totalNs{0};
};

bool profileEnabled()
{
    static const bool enabled = []() {
        const char* env = std::getenv("PULSAR_LIBRETRO_PROFILE");
        return env && std::strcmp(env, "0") != 0;
    }();
    return enabled;
}

void printStatsIfNeeded(const char* name, CallbackStats& stats)
{
    if (!profileEnabled()) {
        return;
    }

    const auto calls = stats.calls.load(std::memory_order_relaxed);
    if (calls == 0 || (calls % 256u) != 0u) {
        return;
    }

    const auto totalNs = stats.totalNs.load(std::memory_order_relaxed);
    const double avgUs = static_cast<double>(totalNs) / 1000.0 / static_cast<double>(calls);
    std::cout << "[libretro profile] " << name << " avg=" << avgUs << " us over " << calls << " calls" << std::endl;
}

CallbackStats& videoStats()
{
    static CallbackStats stats;
    return stats;
}

CallbackStats& audioStats()
{
    static CallbackStats stats;
    return stats;
}

CallbackStats& inputStats()
{
    static CallbackStats stats;
    return stats;
}

CallbackStats& runStats()
{
    static CallbackStats stats;
    return stats;
}

void videoRefreshCallback(const void* data, unsigned width, unsigned height, size_t pitch)
{
    if (profileEnabled()) {
        const auto start = std::chrono::steady_clock::now();
        LibretroVideo::videoRefresh(data, width, height, pitch);
        const auto end = std::chrono::steady_clock::now();
        auto& stats = videoStats();
        stats.calls.fetch_add(1, std::memory_order_relaxed);
        stats.totalNs.fetch_add(static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()), std::memory_order_relaxed);
        printStatsIfNeeded("video_refresh", stats);
        return;
    }

    LibretroVideo::videoRefresh(data, width, height, pitch);
}

void audioSampleCallback(int16_t left, int16_t right)
{
    int16_t sample[2] = { left, right };
    if (profileEnabled()) {
        const auto start = std::chrono::steady_clock::now();
        LibretroAudio::pushSamples(sample, 1);
        const auto end = std::chrono::steady_clock::now();
        auto& stats = audioStats();
        stats.calls.fetch_add(1, std::memory_order_relaxed);
        stats.totalNs.fetch_add(static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()), std::memory_order_relaxed);
        printStatsIfNeeded("audio_sample", stats);
        return;
    }

    LibretroAudio::pushSamples(sample, 1);
}

size_t audioSampleBatchCallback(const int16_t* data, size_t frames)
{
    if (profileEnabled()) {
        const auto start = std::chrono::steady_clock::now();
        const auto result = LibretroAudio::pushSamples(data, frames);
        const auto end = std::chrono::steady_clock::now();
        auto& stats = audioStats();
        stats.calls.fetch_add(1, std::memory_order_relaxed);
        stats.totalNs.fetch_add(static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()), std::memory_order_relaxed);
        printStatsIfNeeded("audio_batch", stats);
        return result;
    }

    return LibretroAudio::pushSamples(data, frames);
}

void inputPollCallback()
{
}

int16_t inputStateCallback(unsigned port, unsigned device, unsigned index, unsigned id)
{
    Q_UNUSED(port)
    Q_UNUSED(device)
    Q_UNUSED(index)

    if (profileEnabled()) {
        const auto start = std::chrono::steady_clock::now();
        int16_t result = 0;
        if (device == RETRO_DEVICE_JOYPAD)
            result = LibretroInput::state(id);
        else if (device == RETRO_DEVICE_POINTER)
            result = LibretroTouch::state(device, id);

        const auto end = std::chrono::steady_clock::now();
        auto& stats = inputStats();
        stats.calls.fetch_add(1, std::memory_order_relaxed);
        stats.totalNs.fetch_add(static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()), std::memory_order_relaxed);
        printStatsIfNeeded("input_state", stats);
        return result;
    }

    if (device == RETRO_DEVICE_JOYPAD)
        return LibretroInput::state(id);

    if (device == RETRO_DEVICE_POINTER)
        return LibretroTouch::state(device, id);

    return 0;
}

int readIntFromEnv(const char* name, int fallback, int minimum, int maximum)
{
    const char* rawValue = std::getenv(name);
    if (!rawValue) {
        return fallback;
    }

    char* end = nullptr;
    const long parsed = std::strtol(rawValue, &end, 10);
    if (end == rawValue || *end != '\0') {
        return fallback;
    }

    return std::clamp(static_cast<int>(parsed), minimum, maximum);
}

int defaultDesmumeThreadCount()
{
#if defined(__linux__) && defined(__aarch64__)
    const int aarch64Default = 4;
#else
    const int aarch64Default = 2;
#endif

    const unsigned hw = std::thread::hardware_concurrency();
    if (hw == 0u) {
        return aarch64Default;
    }

    const int suggested = std::clamp(static_cast<int>(hw > 1u ? hw - 1u : 1u), 1, 8);
    return std::max(aarch64Default, suggested);
}

QByteArray readStringFromEnv(const char* name)
{
    const char* rawValue = std::getenv(name);
    return rawValue ? QByteArray(rawValue) : QByteArray();
}
}

bool LibretroCore::load(const QString& libraryPath)
{
    LibretroEnvironment::resetCoreOptions();

    if (m_library.isLoaded())
        m_library.unload();

    m_library.setFileName(libraryPath);

    if (!m_library.load()) {
        return false;
    }

    if (!resolveSymbols()) {
        return false;
    }

    const QString libraryName = QFileInfo(libraryPath).fileName();
    m_isDesmume2015 = libraryName.compare(
        QStringLiteral("desmume2015_libretro.dll"), Qt::CaseInsensitive) == 0
        || libraryName.compare(
            QStringLiteral("desmume_libretro.so"), Qt::CaseInsensitive) == 0;

    if (m_isDesmume2015) {
        const QByteArray cpuModeEnv = readStringFromEnv("PULSAR_DESMUME_CPU_MODE");
        const QString cpuMode = cpuModeEnv.isEmpty() ? QStringLiteral("jit") : QString::fromUtf8(cpuModeEnv);
        const int jitBlockSize = readIntFromEnv("PULSAR_DESMUME_JIT_BLOCK_SIZE", 20, 1, 128);
        const int numCores = readIntFromEnv("PULSAR_DESMUME_NUM_CORES", defaultDesmumeThreadCount(), 1, 8);

        LibretroEnvironment::setCoreOption(QStringLiteral("desmume_cpu_mode"), cpuMode);
        LibretroEnvironment::setCoreOption(QStringLiteral("desmume_jit_block_size"), QString::number(jitBlockSize));
        LibretroEnvironment::setCoreOption(QStringLiteral("desmume_num_cores"), QString::number(numCores));
        LibretroEnvironment::setCoreOption(QStringLiteral("desmume_advanced_timing"), QStringLiteral("disabled"));
        LibretroEnvironment::setCoreOption(QStringLiteral("desmume_frameskip"), QStringLiteral("0"));
        LibretroEnvironment::setCoreOption(QStringLiteral("desmume_internal_resolution"), QStringLiteral("256x192"));
        LibretroEnvironment::setCoreOption(QStringLiteral("desmume_gfx_edgemark"), QStringLiteral("disabled"));
        LibretroEnvironment::setCoreOption(QStringLiteral("desmume_gfx_linehack"), QStringLiteral("disabled"));

        std::cout << "[desmume config] cpu_mode=" << cpuMode.toStdString()
                  << " jit_block_size=" << jitBlockSize
                  << " num_cores=" << numCores << std::endl;
    }

    std::cout << "================================" << std::endl;
    std::cout << "Core chargé.";
    std::cout << "================================" << std::endl;

    return true;
}

bool LibretroCore::initialize()
{
    if (!retro_init)
        return false;

    if (!resolveCallbacks()) {
        return false;
    }

    retro_init();

    retro_system_info info{};
    retro_get_system_info(&info);

    return true;
}

void LibretroCore::unload()
{
    if (!m_savePath.isEmpty()) {
        saveGame(m_savePath);
    }

    if (retro_unload_game)
        retro_unload_game();

    if (retro_deinit)
        retro_deinit();

    if (m_library.isLoaded())
        m_library.unload();
}

bool LibretroCore::resolveSymbols()
{
    retro_init =
        reinterpret_cast<retro_init_t>(
            m_library.resolve("retro_init"));

    retro_deinit =
        reinterpret_cast<retro_deinit_t>(
            m_library.resolve("retro_deinit"));

    retro_get_system_info =
        reinterpret_cast<retro_get_system_info_t>(
            m_library.resolve("retro_get_system_info"));

    retro_api_version =
        reinterpret_cast<retro_api_version_t>(
            m_library.resolve("retro_api_version"));

    retro_set_environment =
        reinterpret_cast<retro_set_environment_t>(
            m_library.resolve("retro_set_environment"));

    retro_set_video_refresh =
        reinterpret_cast<retro_set_video_refresh_t>(
            m_library.resolve("retro_set_video_refresh"));

    retro_set_audio_sample =
        reinterpret_cast<retro_set_audio_sample_t>(
            m_library.resolve("retro_set_audio_sample"));

    retro_set_audio_sample_batch =
        reinterpret_cast<retro_set_audio_sample_batch_t>(
            m_library.resolve("retro_set_audio_sample_batch"));

    retro_set_input_poll =
        reinterpret_cast<retro_set_input_poll_t>(
            m_library.resolve("retro_set_input_poll"));

    retro_set_input_state =
        reinterpret_cast<retro_set_input_state_t>(
            m_library.resolve("retro_set_input_state"));

    retro_load_game =
        reinterpret_cast<retro_load_game_t>(
            m_library.resolve("retro_load_game"));

    retro_unload_game =
        reinterpret_cast<retro_unload_game_t>(
            m_library.resolve("retro_unload_game"));

    retro_get_system_av_info =
        reinterpret_cast<retro_get_system_av_info_t>(
            m_library.resolve("retro_get_system_av_info"));

    retro_run =
        reinterpret_cast<retro_run_t>(
            m_library.resolve("retro_run"));
    
    retro_get_memory_data =
    reinterpret_cast<retro_get_memory_data_t>(
        m_library.resolve("retro_get_memory_data"));

    retro_get_memory_size =
        reinterpret_cast<retro_get_memory_size_t>(
            m_library.resolve("retro_get_memory_size"));

    retro_set_controller_port_device =
        reinterpret_cast<retro_set_controller_port_device_t>(
            m_library.resolve("retro_set_controller_port_device"));
    
    return retro_init &&
            retro_deinit &&
            retro_get_system_info &&
            retro_api_version &&
            retro_set_environment &&
            retro_set_video_refresh &&
            retro_set_audio_sample &&
            retro_set_audio_sample_batch &&
            retro_set_input_poll &&
            retro_set_input_state &&
            retro_load_game &&
            retro_unload_game &&
            retro_get_system_av_info &&
            retro_run &&
            retro_get_memory_data &&
            retro_get_memory_size &&
            retro_set_controller_port_device;
}

bool LibretroCore::resolveCallbacks()
{
    retro_set_environment(LibretroEnvironment::callback);
    retro_set_video_refresh(videoRefreshCallback);
    retro_set_audio_sample(audioSampleCallback);
    retro_set_audio_sample_batch(audioSampleBatchCallback);
    retro_set_input_poll(inputPollCallback);
    retro_set_input_state(inputStateCallback);

    return true;
}


bool LibretroCore::loadGame(const QString& romPath)
{
    if (!retro_load_game)
        return false;

    QFile file(romPath);

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    m_romData = file.readAll();
    QByteArray path = romPath.toLocal8Bit();

    retro_game_info game{};
    game.path = path.constData();
    game.data = m_romData.constData();
    game.size = m_romData.size();
    game.meta = nullptr;

    if (!retro_load_game(&game))
    {
        return false;
    }

    QFileInfo romInfo(romPath);
    m_savePath = romInfo.absolutePath() + "/" + romInfo.completeBaseName() + ".sav";

    if (QFile::exists(m_savePath)) {
        loadSave(m_savePath);
    }

    retro_system_av_info avInfo{};
    retro_get_system_av_info(&avInfo);

    LibretroAudio::initialize(static_cast<unsigned>(avInfo.timing.sample_rate));

    std::cout << "ROM chargée." << std::endl;
    std::cout << "FPS :" << avInfo.timing.fps << std::endl;
    std::cout << "Resolution :"
              << avInfo.geometry.base_width
              << "x"
              << avInfo.geometry.base_height;

    std::cout << "================================" << std::endl;

    return true;
}

void LibretroCore::runFrame()
{
    if (retro_run) {
        FrameTimingProfiler::ScopedTimer timer(FrameTimingProfiler::Stage::RetroRun);

        if (profileEnabled()) {
            const auto start = std::chrono::steady_clock::now();
            retro_run();
            const auto end = std::chrono::steady_clock::now();
            auto& stats = runStats();
            stats.calls.fetch_add(1, std::memory_order_relaxed);
            stats.totalNs.fetch_add(static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()), std::memory_order_relaxed);
            printStatsIfNeeded("retro_run", stats);
            return;
        }

        retro_run();
    }
}

bool LibretroCore::saveGame(const QString& savepath)
{
    void* memory = retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);
    size_t size = retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);

    if (!memory || size == 0)
        return false;

    QFile file(savepath);

    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(reinterpret_cast<const char*>(memory), size);

    return true;
}

bool LibretroCore::loadSave(const QString& path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    void* memory = retro_get_memory_data(RETRO_MEMORY_SAVE_RAM);
    size_t size = retro_get_memory_size(RETRO_MEMORY_SAVE_RAM);

    if (!memory || size == 0)
        return false;

    QByteArray data = file.readAll();

    memcpy(memory, data.constData(), std::min(size, static_cast<size_t>(data.size())));

    return true;
}