#include "EmulatorManager.h"
#include "CoreRegistry.h"

#include <QFile>
#include <iostream>

EmulatorManager& EmulatorManager::instance()
{
    static EmulatorManager instance;
    return instance;
}

bool EmulatorManager::initialize(ConsoleType console)
{
    const std::optional<CoreDescriptor> core = CoreRegistry::descriptor(console);

    if (!core.has_value())
    {
        return false;
    }

    m_profile = ConsoleProfile::forConsole(console);

    std::cout << core->name.toStdString() << " " << core->libraryPath.toStdString() << std::endl;

    if (!m_core.load(core->libraryPath))
        return false;

    return m_core.initialize();
}

bool EmulatorManager::loadRom(const QString& romPath)
{
    // Load the ROM into the emulator core here
    return m_core.loadGame(romPath);
}

void EmulatorManager::runFrame()
{
    // Run a single frame of emulation here
    m_core.runFrame();
}

void EmulatorManager::stop()
{
    m_core.saveGame(m_core.m_savePath);
    m_core.unload();
}

QImage EmulatorManager::topScreen() const
{
    // Return the current frame of the top screen here
    return QImage();
}

QImage EmulatorManager::bottomScreen() const
{
    // Return the current frame of the bottom screen here
    return QImage();
}

const ConsoleProfile& EmulatorManager::profile() const
{
    return m_profile;
}