#pragma once

#include <filesystem>
#include <string>

#include "ConsoleType.h"

class Game
{
public:
    std::string title;
    std::filesystem::path romPath;
    ConsoleType console = ConsoleType::Unknown;
};