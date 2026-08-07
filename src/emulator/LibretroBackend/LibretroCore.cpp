#include "LibretroCore.h"
#include "LibretroVideo.h"
#include "LibretroInput.h"
#include "LibretroTouch.h"
#include "LibretroAudio.h"
#include "utils/FrameTimingProfiler.h"

#include <QFile>
#include <QFileInfo>
#include <iostream>

namespace
{
    void videoRefreshCallback(const void* data, unsigned width, unsigned height, size_t pitch)
    {
        LibretroVideo::videoRefresh( data, width, height, pitch);
    }

    void audioSampleCallback(int16_t left, int16_t right)
    {
        int16_t sample[2] = { left, right };
        LibretroAudio::pushSamples(sample, 1);
    }

    size_t audioSampleBatchCallback(const int16_t* data, size_t frames)
    {
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

        if (device == RETRO_DEVICE_JOYPAD)
        return LibretroInput::state(id);
        
        if (device == RETRO_DEVICE_POINTER)
            return LibretroTouch::state(device, id);

        return 0;
    }
}

bool LibretroCore::load(const QString& libraryPath)
{
    if (m_library.isLoaded())
        m_library.unload();

    m_library.setFileName(libraryPath);

    if (!m_library.load()) {
        return false;
    }

    if (!resolveSymbols()) {
        return false;
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