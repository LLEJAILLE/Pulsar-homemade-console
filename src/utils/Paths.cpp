#include "Paths.h"

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

QString Paths::join(const QString& base, const QString& relative)
{
    return QDir::cleanPath(QDir(base).filePath(relative));
}

QString Paths::projectRoot()
{
    static const QString root = QDir::cleanPath(
        QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("..")));

    return root;
}

QString Paths::assets()
{
    return join(projectRoot(), QStringLiteral("assets"));
}

QString Paths::audio()
{
    return join(assets(), QStringLiteral("audio"));
}

QString Paths::backgrounds()
{
    return join(assets(), QStringLiteral("pulsar"));
}

QString Paths::icons()
{
    return join(assets(), QStringLiteral("icons"));
}

QString Paths::img()
{
    return join(assets(), QStringLiteral("img"));
}

QString Paths::cores()
{
    return join(projectRoot(), QStringLiteral("src/core"));
}

QString Paths::bios()
{
    return join(system(), QStringLiteral("bios"));
}

QString Paths::system()
{
    return join(projectRoot(), QStringLiteral("system"));
}

QString Paths::library()
{
    return join(projectRoot(), QStringLiteral("src/library"));
}

QString Paths::roms()
{
    return join(library(), QStringLiteral("roms"));
}

QString Paths::saves()
{
    return join(library(), QStringLiteral("saves"));
}

QString Paths::config()
{
    return join(projectRoot(), QStringLiteral("config"));
}

QString Paths::userDataRoot()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString Paths::userConfig()
{
    return join(userDataRoot(), QStringLiteral("config"));
}

QString Paths::userSaves()
{
    return join(userDataRoot(), QStringLiteral("saves"));
}

QString Paths::cache()
{
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
}
