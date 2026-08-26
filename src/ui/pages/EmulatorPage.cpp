#include "EmulatorPage.h"

#include <QGridLayout>
#include <QHideEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QResizeEvent>
#include <QVBoxLayout>

#include "audio/AudioManager.h"
#include "emulator/ConsoleProfile.h"
#include "emulator/CoreRegistry.h"
#include "emulator/EmulatorManager.h"
#include "emulator/LibretroBackend/LibretroAudio.h"
#include "input/InputManager.h"
#include "utils/FrameTimingProfiler.h"

#include <algorithm>
#include <functional>
#include <utility>
#include <iostream>

namespace
{
constexpr int kPageMargin = 16;
constexpr int kTitleFontSize = 24;
constexpr int kFrameMs = 16;
constexpr int kVolumeStepPercent = 5;

int emulatorIntervalMs(int speedMultiplier)
{
    return std::max(1, kFrameMs / std::max(1, speedMultiplier));
}

void setButtonFromQtKey(int qtKey, bool pressed)
{
    switch (qtKey)
    {
        case Qt::Key_Z:
            InputManager::setButton(InputManager::Button::A, pressed);
            break;

        case Qt::Key_D:
            InputManager::setButton(InputManager::Button::B, pressed);
            break;

        case Qt::Key_S:
            InputManager::setButton(InputManager::Button::X, pressed);
            std::cout << "X button pressed: " << pressed << std::endl;
            break;

        case Qt::Key_Q:
            InputManager::setButton(InputManager::Button::Y, pressed);
            std::cout << "Y button pressed: " << pressed << std::endl;
            break;

        case Qt::Key_Control:
            InputManager::setButton(InputManager::Button::Start, pressed);
            break;

        case Qt::Key_Shift:
            InputManager::setButton(InputManager::Button::Select, pressed);
            break;

        case Qt::Key_Up:
            InputManager::setButton(InputManager::Button::Up, pressed);
            break;

        case Qt::Key_Down:
            InputManager::setButton(InputManager::Button::Down, pressed);
            break;

        case Qt::Key_Left:
            InputManager::setButton(InputManager::Button::Left, pressed);
            break;

        case Qt::Key_Right:
            InputManager::setButton(InputManager::Button::Right, pressed);
            break;

        case Qt::Key_L:
            InputManager::setButton(InputManager::Button::L, pressed);
            break;

        case Qt::Key_M:
            InputManager::setButton(InputManager::Button::R, pressed);
            break;

        case Qt::Key_P:
        case Qt::Key_F1:
            InputManager::setButton(InputManager::Button::Menu, pressed);
            break;

        default:
            break;
    }
}

const char *kEmulatorPageStyle = R"(
    EmulatorPage {
        background-color: rgb(248, 0, 0)
    }

    QLabel {
        color: white;
        background: transparent;
    }
)";
}

EmulatorPage::EmulatorPage(const Game &game, QWidget *parent) : QWidget(parent) , m_game(game)
{
    setObjectName(QStringLiteral("EmulatorPage"));
    setStyleSheet(QString::fromLatin1(kEmulatorPageStyle));

    const ConsoleProfile profile = ConsoleProfile::forConsole(m_game.console);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_topScreen = new EmulatorScreenWidget(profile, 0, this);
    m_bottomScreen = new EmulatorScreenWidget(profile, profile.screenCount() > 1 ? 1 : -1, this);

    layout->addWidget(m_topScreen);
    layout->addWidget(m_bottomScreen);

    const unsigned expectedPaintEvents = 2u;

    EmulatorManager &emulator = EmulatorManager::instance();
    emulator.initialize(m_game.console);
    emulator.loadRom(QString::fromUtf8(m_game.romPath.u8string().c_str()));

    m_emulatorTimer = new QTimer(this);
    m_emulatorTimer->setTimerType(Qt::PreciseTimer);
    connect(m_emulatorTimer, &QTimer::timeout, this, [this, &emulator, expectedPaintEvents]()
    {
        FrameTimingProfiler::beginFrame(expectedPaintEvents);

        FrameTimingProfiler::ScopedTimer inputTimer(FrameTimingProfiler::Stage::Input);
        const bool menuPressed = InputManager::button(InputManager::Button::Menu);
        if (menuPressed && !m_menuButtonLatch) {
            m_menuButtonLatch = true;
            toggleSettingsOverlay();
        } else if (!menuPressed) {
            m_menuButtonLatch = false;
        }

        auto handleOverlayButtonEdge = [](bool pressed, bool &latch, const std::function<void()> &onPressed)
        {
            if (pressed && !latch) {
                latch = true;
                onPressed();
            } else if (!pressed) {
                latch = false;
            }
        };

        if (m_overlaySettingsOpen) {
            handleOverlayButtonEdge(InputManager::button(InputManager::Button::Up), m_upButtonLatch, [this]() {
                m_selectedSettingIndex = (m_selectedSettingIndex - 1 + static_cast<int>(m_settingRows.size()))
                    % static_cast<int>(m_settingRows.size());
                updateSelectionUi();
            });

            handleOverlayButtonEdge(InputManager::button(InputManager::Button::Down), m_downButtonLatch, [this]() {
                m_selectedSettingIndex = (m_selectedSettingIndex + 1) % static_cast<int>(m_settingRows.size());
                updateSelectionUi();
            });

            handleOverlayButtonEdge(InputManager::button(InputManager::Button::Left), m_leftButtonLatch, [this]() {
                adjustSelectedSetting(-1);
            });

            handleOverlayButtonEdge(InputManager::button(InputManager::Button::Right), m_rightButtonLatch, [this]() {
                adjustSelectedSetting(1);
            });

            handleOverlayButtonEdge(InputManager::button(InputManager::Button::A), m_aButtonLatch, [this]() {
                activateSelectedSetting();
            });

            handleOverlayButtonEdge(InputManager::button(InputManager::Button::B), m_bButtonLatch, [this]() {
                toggleSettingsOverlay();
            });
        }

        QElapsedTimer frameTimer;
        frameTimer.start();

        emulator.runFrame();

        m_topScreen->update();
        m_bottomScreen->update();

        m_lastFrameMs = static_cast<double>(frameTimer.nsecsElapsed()) / 1000000.0;
        ++m_framesSinceLastFpsUpdate;
        updateTelemetryUi();
    });

    createOverlays();
    applySettings();

    m_fpsElapsed.start();
    m_perfElapsed.start();
    m_lastCpuClock = std::clock();

    m_emulatorTimer->start(emulatorIntervalMs(m_speedMultiplier));

    setFocusPolicy(Qt::StrongFocus);
}

EmulatorPage::~EmulatorPage()
{
    if (m_emulatorTimer) {
        m_emulatorTimer->stop();
    }

    LibretroAudio::shutdown();
}

void EmulatorPage::hideEvent(QHideEvent *event)
{
    if (m_emulatorTimer) {
        m_emulatorTimer->stop();
    }

    LibretroAudio::shutdown();

    QWidget::hideEvent(event);
}

void EmulatorPage::resizeEvent(QResizeEvent *event)
{
    if (m_overlay) {
        m_overlay->setGeometry(rect());
    }

    if (m_fpsOverlayLabel) {
        m_fpsOverlayLabel->move(width() - m_fpsOverlayLabel->width() - 12, 10);
    }

    if (m_perfOverlayLabel) {
        m_perfOverlayLabel->move(width() - m_perfOverlayLabel->width() - 12, 42);
    }

    QWidget::resizeEvent(event);
}

void EmulatorPage::keyPressEvent(QKeyEvent *event)
{
    const int key = event->key();

    if (key == Qt::Key_P || key == Qt::Key_F1) {
        InputManager::setButton(InputManager::Button::Menu, true);
        event->accept();
        return;
    }

    if (m_overlaySettingsOpen) {
        switch (key)
        {
            case Qt::Key_Up:
                m_selectedSettingIndex = (m_selectedSettingIndex - 1 + static_cast<int>(m_settingRows.size()))
                    % static_cast<int>(m_settingRows.size());
                updateSelectionUi();
                break;

            case Qt::Key_Down:
                m_selectedSettingIndex = (m_selectedSettingIndex + 1) % static_cast<int>(m_settingRows.size());
                updateSelectionUi();
                break;

            case Qt::Key_Left:
                adjustSelectedSetting(-1);
                break;

            case Qt::Key_Right:
                adjustSelectedSetting(1);
                break;

            case Qt::Key_A:
            case Qt::Key_Space:
            case Qt::Key_Return:
            case Qt::Key_Enter:
                activateSelectedSetting();
                break;

            case Qt::Key_S:
            case Qt::Key_Escape:
                toggleSettingsOverlay();
                break;

            default:
                break;
        }

        event->accept();
        return;
    }

    setButtonFromQtKey(key, true);

    if (key == Qt::Key_Escape) {
        emit backToHome();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

void EmulatorPage::keyReleaseEvent(QKeyEvent *event)
{
    const int key = event->key();
    if (key == Qt::Key_P || key == Qt::Key_F1) {
        InputManager::setButton(InputManager::Button::Menu, false);
    }

    if (m_overlaySettingsOpen) {
        event->accept();
        return;
    }

    setButtonFromQtKey(key, false);
    QWidget::keyReleaseEvent(event);
}

bool EmulatorPage::toggleSettingsOverlay()
{
    m_overlaySettingsOpen = !m_overlaySettingsOpen;

    if (m_overlaySettingsOpen) {
        m_overlay->setGeometry(rect());
        m_overlay->show();
        m_overlay->raise();

        InputManager::clearButtons();
        InputManager::releaseTouch();
    } else {
        m_overlay->hide();
        InputManager::clearButtons();
    }

    applySettings();
    setFocus(Qt::OtherFocusReason);

    return m_overlaySettingsOpen;
}

void EmulatorPage::createOverlays()
{
    m_overlay = new QWidget(this);
    m_overlay->setStyleSheet(QStringLiteral("background-color: rgba(0, 0, 0, 170);"));

    auto *overlayLayout = new QVBoxLayout(m_overlay);
    overlayLayout->setContentsMargins(kPageMargin * 2, kPageMargin * 2, kPageMargin * 2, kPageMargin * 2);
    overlayLayout->setSpacing(14);

    auto *titleLabel = new QLabel(QStringLiteral("Parametres"), m_overlay);
    titleLabel->setStyleSheet(QStringLiteral("color: white; font-size: 24px; font-weight: 700;"));

    auto *coreLabel = new QLabel(m_overlay);
    const std::optional<CoreDescriptor> core = CoreRegistry::descriptor(m_game.console);
    coreLabel->setText(core.has_value()
        ? QStringLiteral("Core actif : %1").arg(core->name)
        : QStringLiteral("Core actif : inconnu"));
    coreLabel->setStyleSheet(QStringLiteral("color: rgba(255, 255, 255, 210); font-size: 16px;"));

    m_settingsPanel = new QWidget(m_overlay);
    m_settingsPanel->setStyleSheet(QStringLiteral("background-color: rgba(255, 255, 255, 20); border: 1px solid rgba(255, 255, 255, 70); border-radius: 10px;"));
    m_settingsGrid = new QGridLayout(m_settingsPanel);
    m_settingsGrid->setContentsMargins(14, 12, 14, 12);
    m_settingsGrid->setHorizontalSpacing(24);
    m_settingsGrid->setVerticalSpacing(10);

    const std::vector<std::pair<SettingItem, QString>> rows = {
        { SettingItem::Volume, QStringLiteral("Volume") },
        { SettingItem::Speed, QStringLiteral("Vitesse") },
        { SettingItem::ShowFps, QStringLiteral("Afficher les FPS") },
        { SettingItem::ShowPerformance, QStringLiteral("Afficher les performances") },
    };

    int rowIndex = 0;
    for (const auto &row : rows) {
        auto *nameLabel = new QLabel(row.second, m_settingsPanel);
        auto *valueLabel = new QLabel(m_settingsPanel);
        valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        nameLabel->setMinimumWidth(280);
        valueLabel->setMinimumWidth(120);

        m_settingsGrid->addWidget(nameLabel, rowIndex, 0);
        m_settingsGrid->addWidget(valueLabel, rowIndex, 1);

        m_settingRows.push_back({ row.first, nameLabel, valueLabel });
        ++rowIndex;
    }

    m_settingsGrid->setColumnStretch(0, 3);
    m_settingsGrid->setColumnStretch(1, 1);

    auto *hintLabel = new QLabel(
        QStringLiteral("Haut/Bas: selection  Gauche/Droite: modifier  A: valider  B/Echap: fermer"),
        m_overlay);
    hintLabel->setStyleSheet(QStringLiteral("color: rgba(255, 255, 255, 210); font-size: 14px;"));

    overlayLayout->addWidget(titleLabel);
    overlayLayout->addWidget(coreLabel);
    overlayLayout->addWidget(m_settingsPanel);
    overlayLayout->addWidget(hintLabel);
    overlayLayout->addStretch();

    m_overlay->hide();

    m_fpsOverlayLabel = new QLabel(this);
    m_fpsOverlayLabel->setStyleSheet(QStringLiteral("background-color: rgba(0, 0, 0, 160); color: white; padding: 4px 8px; border-radius: 6px;"));
    m_fpsOverlayLabel->setText(QStringLiteral("FPS: 0"));
    m_fpsOverlayLabel->adjustSize();
    m_fpsOverlayLabel->hide();

    m_perfOverlayLabel = new QLabel(this);
    m_perfOverlayLabel->setStyleSheet(QStringLiteral("background-color: rgba(0, 0, 0, 160); color: white; padding: 4px 8px; border-radius: 6px;"));
    m_perfOverlayLabel->setText(QStringLiteral("CPU: 0%  GPU: 0%"));
    m_perfOverlayLabel->adjustSize();
    m_perfOverlayLabel->hide();

    updateSelectionUi();
    updateSettingsUi();
}

void EmulatorPage::applySettings()
{
    const float volume = static_cast<float>(m_volumePercent) / 100.0f;
    LibretroAudio::setVolume(volume);
    AudioManager::instance().setVolume(volume);
    setEmulationSpeedMultiplier(m_speedMultiplier);

    if (m_fpsOverlayLabel) {
        m_fpsOverlayLabel->setVisible(m_showFps);
    }

    if (m_perfOverlayLabel) {
        m_perfOverlayLabel->setVisible(m_showPerformance);
    }

    updateSettingsUi();
    updateTelemetryUi();
}

void EmulatorPage::updateSettingsUi()
{
    for (const SettingRow &row : m_settingRows) {
        if (!row.valueLabel) {
            continue;
        }

        switch (row.item)
        {
            case SettingItem::Volume:
                row.valueLabel->setText(m_volumePercent == 0
                    ? QStringLiteral("Mute")
                    : QStringLiteral("%1%").arg(m_volumePercent));
                break;

            case SettingItem::Speed:
                row.valueLabel->setText(QStringLiteral("x%1").arg(m_speedMultiplier));
                break;

            case SettingItem::ShowFps:
                row.valueLabel->setText(yesNoText(m_showFps));
                break;

            case SettingItem::ShowPerformance:
                row.valueLabel->setText(yesNoText(m_showPerformance));
                break;
        }
    }
}

void EmulatorPage::updateSelectionUi()
{
    for (int i = 0; i < static_cast<int>(m_settingRows.size()); ++i) {
        const bool selected = i == m_selectedSettingIndex;
        const QString selectedStyle = QStringLiteral("background-color: rgba(255, 255, 255, 230); color: black; border-radius: 4px; padding: 4px 8px;");
        const QString normalStyle = QStringLiteral("background-color: transparent; color: white; padding: 4px 8px;");

        if (m_settingRows[static_cast<std::size_t>(i)].nameLabel) {
            m_settingRows[static_cast<std::size_t>(i)].nameLabel->setStyleSheet(selected ? selectedStyle : normalStyle);
        }

        if (m_settingRows[static_cast<std::size_t>(i)].valueLabel) {
            m_settingRows[static_cast<std::size_t>(i)].valueLabel->setStyleSheet(selected ? selectedStyle : normalStyle);
        }
    }
}

void EmulatorPage::updateTelemetryUi()
{
    if (m_fpsElapsed.elapsed() >= 250) {
        const qint64 elapsedMs = std::max<qint64>(1, m_fpsElapsed.elapsed());
        m_currentFps = static_cast<double>(m_framesSinceLastFpsUpdate) * 1000.0 / static_cast<double>(elapsedMs);
        m_framesSinceLastFpsUpdate = 0;
        m_fpsElapsed.restart();
    }

    if (m_perfElapsed.elapsed() >= 500) {
        const std::clock_t nowClock = std::clock();
        const qint64 elapsedMs = std::max<qint64>(1, m_perfElapsed.elapsed());
        const double elapsedSec = static_cast<double>(elapsedMs) / 1000.0;

        const double deltaCpuSec = static_cast<double>(nowClock - m_lastCpuClock) / static_cast<double>(CLOCKS_PER_SEC);
        m_cpuUsagePercent = std::clamp((deltaCpuSec / elapsedSec) * 100.0, 0.0, 100.0);

        const double frameBudgetMs = static_cast<double>(emulatorIntervalMs(m_speedMultiplier));
        m_gpuUsagePercent = std::clamp((m_lastFrameMs / std::max(1.0, frameBudgetMs)) * 100.0, 0.0, 100.0);

        m_lastCpuClock = nowClock;
        m_perfElapsed.restart();
    }

    if (m_fpsOverlayLabel && m_showFps) {
        m_fpsOverlayLabel->setText(QStringLiteral("FPS: %1").arg(qRound(m_currentFps)));
        m_fpsOverlayLabel->adjustSize();
        m_fpsOverlayLabel->move(width() - m_fpsOverlayLabel->width() - 12, 10);
    }

    if (m_perfOverlayLabel && m_showPerformance) {
        m_perfOverlayLabel->setText(
            QStringLiteral("CPU: %1%  GPU: %2%")
                .arg(qRound(m_cpuUsagePercent))
                .arg(qRound(m_gpuUsagePercent)));
        m_perfOverlayLabel->adjustSize();
        m_perfOverlayLabel->move(width() - m_perfOverlayLabel->width() - 12, 42);
    }
}

void EmulatorPage::adjustSelectedSetting(int direction)
{
    if (m_settingRows.empty() || direction == 0) {
        return;
    }

    const SettingItem item = m_settingRows[static_cast<std::size_t>(m_selectedSettingIndex)].item;

    switch (item)
    {
        case SettingItem::Volume:
            m_volumePercent = std::clamp(m_volumePercent + (direction * kVolumeStepPercent), 0, 100);
            break;

        case SettingItem::Speed:
            m_speedMultiplier = std::clamp(m_speedMultiplier + direction, 1, 4);
            break;

        case SettingItem::ShowFps:
            m_showFps = !m_showFps;
            break;

        case SettingItem::ShowPerformance:
            m_showPerformance = !m_showPerformance;
            break;
    }

    applySettings();
}

void EmulatorPage::activateSelectedSetting()
{
    if (m_settingRows.empty()) {
        return;
    }

    const SettingItem item = m_settingRows[static_cast<std::size_t>(m_selectedSettingIndex)].item;

    switch (item)
    {
        case SettingItem::ShowFps:
            m_showFps = !m_showFps;
            break;

        case SettingItem::ShowPerformance:
            m_showPerformance = !m_showPerformance;
            break;

        case SettingItem::Volume:
        case SettingItem::Speed:
            break;
    }

    applySettings();
}

void EmulatorPage::setEmulationSpeedMultiplier(int speedMultiplier)
{
    m_speedMultiplier = std::clamp(speedMultiplier, 1, 4);

    if (!m_emulatorTimer) {
        return;
    }

    const int interval = emulatorIntervalMs(m_speedMultiplier);
    m_emulatorTimer->setInterval(interval);
}

QString EmulatorPage::yesNoText(bool value)
{
    return value ? QStringLiteral("Oui") : QStringLiteral("Non");
}