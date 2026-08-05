#pragma once
#include <QLibrary>
#include <QString>
#include <QByteArray>

#include "Libretro.h"
#include "LibretroEnvironment.h"

class LibretroCore
{
    public:
        bool load(const QString& libraryPath);
        bool initialize();
        
        bool loadGame(const QString&);
        void runFrame();
        
        void unload();

        bool saveGame(const QString& savePath);
        bool loadSave(const QString& savePath);
        
        QString m_savePath;
        
    private:
        QLibrary m_library;
        QByteArray m_romData;

        using retro_init_t = void (*)();
        using retro_deinit_t = void (*)();
        using retro_get_system_info_t = void (*)(retro_system_info*);
        using retro_api_version_t = unsigned (*)();

        retro_init_t retro_init = nullptr;
        retro_deinit_t retro_deinit = nullptr;
        retro_get_system_info_t retro_get_system_info = nullptr;
        retro_api_version_t retro_api_version = nullptr;


        using retro_set_environment_t = void (*)(retro_environment_t);
        using retro_set_video_refresh_t = void (*)(retro_video_refresh_t);
        using retro_set_audio_sample_t = void (*)(retro_audio_sample_t);
        using retro_set_audio_sample_batch_t = void (*)(retro_audio_sample_batch_t);
        using retro_set_input_poll_t = void (*)(retro_input_poll_t);
        using retro_set_input_state_t = void (*)(retro_input_state_t);

        retro_set_environment_t retro_set_environment = nullptr;
        retro_set_video_refresh_t retro_set_video_refresh = nullptr;
        retro_set_audio_sample_t retro_set_audio_sample = nullptr;
        retro_set_audio_sample_batch_t retro_set_audio_sample_batch = nullptr;
        retro_set_input_poll_t retro_set_input_poll = nullptr;
        retro_set_input_state_t retro_set_input_state = nullptr;


        using retro_load_game_t = bool (*)(const retro_game_info*);
        using retro_unload_game_t = void (*)();
        using retro_get_system_av_info_t = void (*)(retro_system_av_info*);
        using retro_run_t = void (*)();

        retro_load_game_t retro_load_game = nullptr;
        retro_unload_game_t retro_unload_game = nullptr;
        retro_get_system_av_info_t retro_get_system_av_info = nullptr;
        retro_run_t retro_run = nullptr;


        using retro_get_memory_data_t = void* (*)(unsigned);
        using retro_get_memory_size_t = size_t (*)(unsigned);

        retro_get_memory_data_t retro_get_memory_data = nullptr;
        retro_get_memory_size_t retro_get_memory_size = nullptr;

        using retro_set_controller_port_device_t = void (*)(unsigned port, unsigned device);

        retro_set_controller_port_device_t retro_set_controller_port_device = nullptr;


    private:
        bool resolveSymbols();
        bool resolveCallbacks();
};