#pragma once

#include <optional>

#include <QString>

#include "../library/ConsoleType.h"

struct CoreDescriptor
{
    ConsoleType console = ConsoleType::Unknown;
    QString name;
    QString libraryPath;
};

class CoreRegistry
{
public:
    static std::optional<CoreDescriptor> descriptor(ConsoleType console);
    static QString libraryPath(ConsoleType console);
};