#include "CoreRegistry.h"

#include "utils/Paths.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>

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
    const QString coresDirectory = Paths::cores();
    const QStringList coreDirectories = {
        QStringLiteral("desmume_libretro-linux-aarch64"),
        QStringLiteral("desmume_libretro-linux-arm64"),
        QStringLiteral("desmume_libretro-linux-x86_64")
    };

    QString selectedCore;
    for (const QString& directory : coreDirectories) {
        const QDir coreDirectory(QDir(coresDirectory).filePath(directory));
        const QString currentCore = coreDirectory.filePath(QStringLiteral("desmume_libretro.so"));
        const QString legacyCore = coreDirectory.filePath(QStringLiteral("desmume2015_libretro.so"));

        if (QFileInfo::exists(currentCore)) {
            selectedCore = currentCore;
            break;
        }

        if (QFileInfo::exists(legacyCore)) {
            selectedCore = legacyCore;
            break;
        }
    }

    return {
        ConsoleType::NDS,
        QStringLiteral("DeSmuME 2015"),
        selectedCore
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