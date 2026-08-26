#include "CoreRegistry.h"

#include "utils/Paths.h"

#include <QDir>

namespace
{
CoreDescriptor makeDesmumeDescriptor()
{
#ifdef Q_OS_WIN
    return {
        ConsoleType::NDS,
        QStringLiteral("DeSmuME"),
        QDir(Paths::cores()).filePath(QStringLiteral("desmume_libretro-win32-x86_64/desmume2015_libretro.dll"))
    };
#else
    return {
        ConsoleType::NDS,
        QStringLiteral("DeSmuME 2015"),
        QDir(Paths::cores()).filePath(QStringLiteral("desmume_libretro-linux-x86_64/desmume_libretro.so"))
    };
#endif
}

CoreDescriptor makeMGbaDescriptor()
{
#ifdef Q_OS_WIN
    return {
        ConsoleType::GBA,
        QStringLiteral("mGBA"),
        QDir(Paths::cores()).filePath(QStringLiteral("mGBA_libretro-win32-x86_64/mgba_libretro.dll"))
    };
#else
    return {
        ConsoleType::GBA,
        QStringLiteral("mGBA"),
        QDir(Paths::cores()).filePath(QStringLiteral("mGBA_libretro-linux-x86_64/mgba_libretro.so"))
    };
#endif
}
}

std::optional<CoreDescriptor> CoreRegistry::descriptor(ConsoleType console)
{
    switch (console)
    {
        case ConsoleType::NDS:
            return makeDesmumeDescriptor();

        case ConsoleType::GBA:
            return makeMGbaDescriptor();

        default:
            return std::nullopt;
    }
}

QString CoreRegistry::libraryPath(ConsoleType console)
{
    const std::optional<CoreDescriptor> core = descriptor(console);

    if (!core.has_value())
        return {};

    return core->libraryPath;
}