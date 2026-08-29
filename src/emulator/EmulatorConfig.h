#pragma once

#include <QString>
#include <QMap>

/**
 * EmulatorConfig - Gère les profils de configuration pour optimiser les performances
 * spécialement pour les systèmes à faibles ressources comme Raspberry Pi
 */
class EmulatorConfig
{
public:
    enum class OptimizationProfile {
        HighPerformance,  // Pour Raspberry Pi et systèmes faibles
        Balanced,         // Équilibre qualité/performance
        HighQuality       // Pour machines puissantes
    };

    /**
     * Applique un profil d'optimisation aux variables libretro
     * @param profile Le profil à appliquer
     */
    static void applyOptimizationProfile(OptimizationProfile profile);

    /**
     * Applique les paramètres d'optimisation pour Raspberry Pi
     */
    static void applyRaspberryPiOptimization();

private:
    static QMap<QString, QString> getHighPerformanceConfig();
    static QMap<QString, QString> getBalancedConfig();
    static QMap<QString, QString> getHighQualityConfig();
};
