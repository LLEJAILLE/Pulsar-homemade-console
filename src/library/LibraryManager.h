#pragma once

#include <filesystem>
#include <vector>

#include "Game.h"

class LibraryManager
{
public:
    void scan(const std::filesystem::path &libraryPath);

    const std::vector<Game> &games() const;

private:
    std::vector<Game> m_games;
};