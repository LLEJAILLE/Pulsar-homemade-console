#pragma once

#include <QWidget>
#include <QString>
#include <QTimer>
#include <QElapsedTimer>
#include <vector>
#include <ctime>

#include "library/Game.h"

#include "ui/widgets/EmulatorScreenWidget.h"

class BackgroundWidget;
class QLabel;
class QResizeEvent;
class QHideEvent;
class QVBoxLayout;
class QKeyEvent;
class QGridLayout;

class EmulatorPage : public QWidget
{
    Q_OBJECT

public:
    explicit EmulatorPage(const Game &game, QWidget *parent = nullptr);
    ~EmulatorPage() override;

    const Game& currentGame() const { return m_game; }

    bool toggleSettingsOverlay();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

signals:
    void backToHome();

private:
    enum class SettingItem
    {
        Volume = 0,
        Speed,
        ShowFps,
        ShowPerformance,
    };

    struct SettingRow
    {
        SettingItem item;
        QLabel *nameLabel = nullptr;
        QLabel *valueLabel = nullptr;
    };

    void createOverlays();
    void applySettings();
    void updateSettingsUi();
    void updateSelectionUi();
    void updateTelemetryUi();
    void adjustSelectedSetting(int direction);
    void activateSelectedSetting();
    void setEmulationSpeedMultiplier(int speedMultiplier);
    static QString yesNoText(bool value);

    bool m_overlaySettingsOpen = false;
    bool m_menuButtonLatch = false;
    bool m_upButtonLatch = false;
    bool m_downButtonLatch = false;
    bool m_leftButtonLatch = false;
    bool m_rightButtonLatch = false;
    bool m_aButtonLatch = false;
    bool m_bButtonLatch = false;
    int m_selectedSettingIndex = 0;
    int m_volumePercent = 75;
    int m_speedMultiplier = 1;
    bool m_showFps = false;
    bool m_showPerformance = false;

    QElapsedTimer m_fpsElapsed;
    int m_framesSinceLastFpsUpdate = 0;
    double m_currentFps = 0.0;
    QElapsedTimer m_perfElapsed;
    std::clock_t m_lastCpuClock = 0;
    double m_cpuUsagePercent = 0.0;
    double m_gpuUsagePercent = 0.0;
    double m_lastFrameMs = 0.0;

    QWidget *m_settingsPanel = nullptr;
    QGridLayout *m_settingsGrid = nullptr;
    std::vector<SettingRow> m_settingRows;
    QLabel *m_fpsOverlayLabel = nullptr;
    QLabel *m_perfOverlayLabel = nullptr;

    Game m_game;
    BackgroundWidget *m_background = nullptr;
    QWidget *m_overlay = nullptr;
    QLabel *m_gameTitle = nullptr;
    QLabel *m_statusLabel = nullptr;

    QTimer* m_emulatorTimer = nullptr;

    EmulatorScreenWidget* m_topScreen = nullptr;
    EmulatorScreenWidget* m_bottomScreen = nullptr;
};
