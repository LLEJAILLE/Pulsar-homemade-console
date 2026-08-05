#include "LibraryManager.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace {
    ConsoleType consoleTypeFromPath(const std::filesystem::path &rootPath, const std::filesystem::path &filePath) {
        const auto relativePath = std::filesystem::relative(filePath.parent_path(), rootPath);

        for (const auto &part : relativePath) {
            const std::string folderName = part.string();

            if (folderName == "nds") {
                return ConsoleType::NDS;
            }

            if (folderName == "gba") {
                return ConsoleType::GBA;
            }
        }

        return ConsoleType::Unknown;
    }

    bool hasSupportedExtension(const std::filesystem::path &filePath) {
        const std::string extension = filePath.extension().string();

        if (extension.empty()) {
            return false;
        }

        std::string normalizedExtension;
        normalizedExtension.reserve(extension.size());

        for (const char character : extension) {
            normalizedExtension.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
        }

        return normalizedExtension == ".nds" || normalizedExtension == ".gba";
    }
}

void LibraryManager::scan(const std::filesystem::path &libraryPath) {
    m_games.clear();

    if (!std::filesystem::exists(libraryPath)) {
        return;
    }

    for (const auto &entry : std::filesystem::recursive_directory_iterator(libraryPath)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const std::filesystem::path filePath = entry.path();

        if (!hasSupportedExtension(filePath)) {
            continue;
        }

        Game game;
        game.title = filePath.stem().string();
        game.romPath = filePath;
        game.console = consoleTypeFromPath(libraryPath, filePath);

        m_games.push_back(std::move(game));
    }
}

const std::vector<Game> &LibraryManager::games() const
{
    return m_games;
}