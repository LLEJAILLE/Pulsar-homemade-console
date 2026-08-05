#include "EmulatorPage.h"

#include <QLabel>
#include <QHideEvent>
#include <QTimer>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QKeyEvent>

#include "emulator/EmulatorManager.h"
#include "emulator/ConsoleProfile.h"
#include "emulator/LibretroBackend/LibretroAudio.h"
#include "input/InputManager.h"

#include <iostream>

namespace
{
constexpr int kPageMargin = 16;
constexpr int kTitleFontSize = 24;

void setButtonFromQtKey(int qtKey, bool pressed)
{
    switch (qtKey)
    {
        case Qt::Key_A:
            InputManager::setButton(InputManager::Button::A, pressed);
            break;

        case Qt::Key_S:
            InputManager::setButton(InputManager::Button::B, pressed);
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

EmulatorPage::EmulatorPage(const Game &game, QWidget *parent) : QWidget(parent), m_game(game)
{
    setObjectName(QStringLiteral("EmulatorPage"));
    setStyleSheet(QString::fromLatin1(kEmulatorPageStyle));

    const ConsoleProfile profile = ConsoleProfile::forConsole(m_game.console);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    m_topScreen = new EmulatorScreenWidget(profile, 0, this);
    m_bottomScreen = new EmulatorScreenWidget(profile, profile.screenCount() > 1 ? 1 : -1, this);

    layout->addWidget(m_topScreen);
    layout->addWidget(m_bottomScreen);

    EmulatorManager &emulator = EmulatorManager::instance();
    emulator.initialize(m_game.console);
    emulator.loadRom(QString::fromStdString(m_game.romPath.string()));

    m_emulatorTimer = new QTimer(this);
    m_emulatorTimer->setTimerType(Qt::PreciseTimer);
    connect(m_emulatorTimer, &QTimer::timeout, this, [this, &emulator]()
    {
        emulator.runFrame();

        m_topScreen->update();
        m_bottomScreen->update();
    });
            
    m_emulatorTimer->start(16);

    setFocusPolicy(Qt::StrongFocus);
}

EmulatorPage::~EmulatorPage()
{
    if (m_emulatorTimer)
        m_emulatorTimer->stop();

    LibretroAudio::shutdown();
}

void EmulatorPage::hideEvent(QHideEvent *event)
{
    if (m_emulatorTimer)
        m_emulatorTimer->stop();

    LibretroAudio::shutdown();

    QWidget::hideEvent(event);
}


void EmulatorPage::keyPressEvent(QKeyEvent *event)
{
    setButtonFromQtKey(event->key(), true);

    // Escape the game
    if (event->key() == Qt::Key_Escape) {
        emit backToHome();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

void EmulatorPage::keyReleaseEvent(QKeyEvent *event)
{
    setButtonFromQtKey(event->key(), false);

    QWidget::keyReleaseEvent(event);
}