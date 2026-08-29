#include "EmulatorConfig.h"

void EmulatorConfig::applyOptimizationProfile(OptimizationProfile profile)
{
    QMap<QString, QString> config;
    
    switch (profile) {
        case OptimizationProfile::HighPerformance:
            config = getHighPerformanceConfig();
            break;
        case OptimizationProfile::Balanced:
            config = getBalancedConfig();
            break;
        case OptimizationProfile::HighQuality:
            config = getHighQualityConfig();
            break;
    }
    
    // TODO: Implement proper libretro variable application
    // For now, variables are stored but not actively applied to the core
    // This requires proper integration with RETRO_ENVIRONMENT_SET_VARIABLES
}

void EmulatorConfig::applyRaspberryPiOptimization()
{
    applyOptimizationProfile(OptimizationProfile::HighPerformance);
}

QMap<QString, QString> EmulatorConfig::getHighPerformanceConfig()
{
    QMap<QString, QString> config;
    
    // ===== CPU Optimization =====
    config["melonds_jit_enable"] = "enabled";           // Activer JIT (~3x speedup)
    config["melonds_jit_block_size"] = "8";             // Taille bloc petite pour économiser RAM
    config["melonds_jit_branch_optimisations"] = "enabled";
    config["melonds_jit_literal_optimisations"] = "enabled";
    config["melonds_jit_fast_memory"] = "disabled";     // Désactiver sur Raspberry Pi
    
    // ===== Rendering Optimization =====
    config["melonds_render_mode"] = "software";         // Software rendering au lieu d'OpenGL
    config["melonds_threaded_renderer"] = "enabled";    // Activer multi-thread (+20-30% perf sur RPi)
    
    // ===== Audio Optimization =====
    config["melonds_audio_interpolation"] = "disabled"; // Pas d'interpolation audio
    config["melonds_audio_bitdepth"] = "10-bit";        // Réduire la profondeur
    
    // ===== Display Optimization =====
    config["melonds_show_cursor"] = "disabled";         // Pas de curseur
    config["melonds_secondary_screen_filtering"] = "nearest";  // Pas de filtrage
    config["melonds_hybrid_screen_filtering"] = "nearest";
    
    // ===== System Resources =====
    config["melonds_dsi_sd_card_enable"] = "disabled";         // Pas de carte SD virtuelle
    config["melonds_homebrew_sd_card_enable"] = "disabled";
    config["melonds_battery_update_interval"] = "60";          // Moins de mises à jour
    
    // ===== Disable OSD Overlays =====
    config["melonds_show_mic_state"] = "disabled";
    config["melonds_show_camera_state"] = "disabled";
    config["melonds_show_lid_state"] = "disabled";
    config["melonds_show_sensor_reading"] = "disabled";
    config["melonds_warn_unsupported_features"] = "disabled";
    config["melonds_warn_bios_problems"] = "disabled";
    
    return config;
}

QMap<QString, QString> EmulatorConfig::getBalancedConfig()
{
    QMap<QString, QString> config;
    
    // ===== CPU Configuration =====
    config["melonds_jit_enable"] = "enabled";
    config["melonds_jit_block_size"] = "16";
    config["melonds_jit_branch_optimisations"] = "enabled";
    config["melonds_jit_literal_optimisations"] = "enabled";
    config["melonds_jit_fast_memory"] = "enabled";
    
    // ===== Rendering =====
    config["melonds_render_mode"] = "software";
    config["melonds_threaded_renderer"] = "enabled";
    
    // ===== Audio =====
    config["melonds_audio_interpolation"] = "linear";
    config["melonds_audio_bitdepth"] = "automatic";
    
    // ===== Display =====
    config["melonds_show_cursor"] = "timeout";
    config["melonds_cursor_timeout"] = "3";
    config["melonds_secondary_screen_filtering"] = "linear";
    config["melonds_hybrid_screen_filtering"] = "linear";
    
    // ===== System =====
    config["melonds_dsi_sd_card_enable"] = "disabled";
    config["melonds_homebrew_sd_card_enable"] = "disabled";
    config["melonds_battery_update_interval"] = "30";
    
    // ===== OSD =====
    config["melonds_show_mic_state"] = "disabled";
    config["melonds_show_camera_state"] = "disabled";
    
    return config;
}

QMap<QString, QString> EmulatorConfig::getHighQualityConfig()
{
    QMap<QString, QString> config;
    
    // ===== CPU Configuration =====
    config["melonds_jit_enable"] = "enabled";
    config["melonds_jit_block_size"] = "32";
    config["melonds_jit_branch_optimisations"] = "enabled";
    config["melonds_jit_literal_optimisations"] = "enabled";
    config["melonds_jit_fast_memory"] = "enabled";
    
    // ===== Rendering =====
    config["melonds_render_mode"] = "opengl";
    config["melonds_opengl_resolution"] = "4";
    config["melonds_opengl_better_polygons"] = "enabled";
    config["melonds_threaded_renderer"] = "enabled";
    
    // ===== Audio =====
    config["melonds_audio_interpolation"] = "cubic";
    config["melonds_audio_bitdepth"] = "automatic";
    
    // ===== Display =====
    config["melonds_show_cursor"] = "timeout";
    config["melonds_cursor_timeout"] = "5";
    config["melonds_secondary_screen_filtering"] = "linear";
    config["melonds_hybrid_screen_filtering"] = "linear";
    
    // ===== System =====
    config["melonds_dsi_sd_card_enable"] = "enabled";
    config["melonds_homebrew_sd_card_enable"] = "enabled";
    config["melonds_battery_update_interval"] = "10";
    
    return config;
}
