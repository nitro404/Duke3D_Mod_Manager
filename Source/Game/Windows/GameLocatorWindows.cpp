#include "GameLocatorWindows.h"

#include "Platform/Windows/WindowsUtilities.h"
#include "Utilities/FileUtilities.h"

GameLocatorWindows::GameLocatorWindows()
    : GameLocator() { }

GameLocatorWindows::~GameLocatorWindows() { }

std::vector<std::string> GameLocatorWindows::getGameSearchPaths() {
    std::vector<std::string> searchPaths;

    // Duke Nukem 3D: 20th Anniversary World Tour (Steam)
    std::optional<std::string> optionalSteamWorldTourDirectory(WindowsUtilities::getRegistryEntry(R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Steam App 434050)", "InstallLocation"));

    if(optionalSteamWorldTourDirectory.has_value()) {
        searchPaths.emplace_back(optionalSteamWorldTourDirectory.value());
    }

    // Duke Nukem 3D: Megaton Edition (Steam)
    std::optional<std::string> optionalSteamMegatonDirectory(WindowsUtilities::getRegistryEntry(R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Steam App 225140)", "InstallLocation"));

    if(optionalSteamMegatonDirectory.has_value()) {
        searchPaths.emplace_back(Utilities::joinPaths(optionalSteamMegatonDirectory.value(), "gameroot"));
    }

    // 3D Realms Anthology / Kill-A-Ton Collection 2015 (Steam)
    std::optional<std::string> optionalSteam3DRealmsAnthologyKillATonCollectionDirectory(WindowsUtilities::getRegistryEntry(R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Steam App 359850)", "InstallLocation"));

    if(optionalSteam3DRealmsAnthologyKillATonCollectionDirectory.has_value()) {
        searchPaths.emplace_back(Utilities::joinPaths(optionalSteam3DRealmsAnthologyKillATonCollectionDirectory.value(), "Duke Nukem 3D"));
    }

    // Duke Nukem 3D: Atomic Edition (Good Old Games)
    std::optional<std::string> optionalGoodOldGamesAtomicEditionDirectoryA(WindowsUtilities::getRegistryEntry(R"(SOFTWARE\GOG.com\Games\1207658730)", "path"));

    if(optionalGoodOldGamesAtomicEditionDirectoryA.has_value()) {
        searchPaths.emplace_back(optionalGoodOldGamesAtomicEditionDirectoryA.value());
    }

    std::optional<std::string> optionalGoodOldGamesAtomicEditionDirectoryB(WindowsUtilities::getRegistryEntry(R"(SOFTWARE\GOG.com\GOGDUKE3D)", "PATH"));

    if(optionalGoodOldGamesAtomicEditionDirectoryB.has_value()) {
        searchPaths.emplace_back(optionalGoodOldGamesAtomicEditionDirectoryB.value());
    }

    // Duke Nukem 3D: Atomic Edition (ZOOM Platform)
    std::optional<std::string> optionalZoomAtomicEditionDirectory(WindowsUtilities::getRegistryEntry(R"(SOFTWARE\ZOOM PLATFORM\Duke Nukem 3D - Atomic Edition)", "InstallPath"));

    if(optionalZoomAtomicEditionDirectory.has_value()) {
        searchPaths.emplace_back(optionalZoomAtomicEditionDirectory.value());
    }

    // Duke Nukem 3D (3D Realms Anthology)
    std::optional<std::string> optionalDukeNukem3DDirectory(WindowsUtilities::getRegistryEntry(R"(SOFTWARE\3DRealms\Duke Nukem 3D)", ""));

    if(optionalDukeNukem3DDirectory.has_value()) {
        searchPaths.emplace_back(Utilities::joinPaths(optionalDukeNukem3DDirectory.value(), "Duke Nukem 3D"));
    }

    // 3D Realms Anthology
    std::optional<std::string> optional3DRealsmAnthologyDirectory(WindowsUtilities::getRegistryEntry(R"(SOFTWARE\3DRealms\Anthology)", ""));

    if(optional3DRealsmAnthologyDirectory.has_value()) {
        searchPaths.emplace_back(Utilities::joinPaths(optional3DRealsmAnthologyDirectory.value(), "Duke Nukem 3D"));
    }

    return searchPaths;
}
