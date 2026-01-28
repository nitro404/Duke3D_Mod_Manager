#include "GameLocator.h"

#include "GameVersion.h"
#include "Game/File/Group/GRP/GroupGRP.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

#include <filesystem>

GameLocator::GameLocator() { }

GameLocator::~GameLocator() { }

bool GameLocator::locateGames() {
	std::vector<std::pair<std::string, std::string>> gameSearchPaths(getGameSearchPaths());

	m_gamePaths.clear();

	spdlog::info("Locating Duke Nukem 3D game installs...");

	for(std::vector<std::pair<std::string, std::string>>::const_iterator i = gameSearchPaths.cbegin(); i != gameSearchPaths.cend(); ++i) {
		if(!std::filesystem::is_directory(std::filesystem::path(i->second))) {
			continue;
		}

		std::string groupFilePath(Utilities::joinPaths(i->second, GroupGRP::DUKE_NUKEM_3D_GROUP_FILE_NAME));

		if(!std::filesystem::is_regular_file(std::filesystem::path(groupFilePath))) {
			continue;
		}

		m_gamePaths.emplace_back(*i);

		const std::string * gameName = &i->first;

		const GameVersion * defaultGameVersion = GameVersion::getDefaultGameVersionWithID(i->first);

		if(defaultGameVersion != nullptr) {
			gameName = &defaultGameVersion->getLongName();
		}

		spdlog::info("Located '{}' game install at: '{}'.", *gameName, i->second);
	}

	spdlog::info("Located {} Duke Nukem 3D game install{}.", m_gamePaths.size(), m_gamePaths.size() == 1 ? "" : "s");

	return !m_gamePaths.empty();
}

size_t GameLocator::numberOfGamePaths() const {
	return m_gamePaths.size();
}

bool GameLocator::hasGamePath(const std::string & gamePath) const {
	return indexOfGamePath(gamePath) != std::numeric_limits<size_t>::max();
}

bool GameLocator::hasGamePathWithID(const std::string & gameVersionID) const {
	return indexOfGamePathWithID(gameVersionID) != std::numeric_limits<size_t>::max();
}

size_t GameLocator::indexOfGamePath(const std::string & gamePath) const {
	if(gamePath.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_gamePaths.size(); i++) {
		if(Utilities::areStringsEqual(m_gamePaths[i].second, gamePath)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t GameLocator::indexOfGamePathWithID(const std::string & gameVersionID) const {
	if(gameVersionID.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_gamePaths.size(); i++) {
		if(Utilities::areStringsEqual(m_gamePaths[i].first, gameVersionID)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

const std::string * GameLocator::getGamePath(size_t index) const {
	if(index >= m_gamePaths.size()) {
		return nullptr;
	}

	return &m_gamePaths[index].second;
}

const std::string * GameLocator::getGamePathWithID(const std::string & gameVersionID) const {
	size_t gamePathIndex = indexOfGamePathWithID(gameVersionID);

	if(gamePathIndex >= m_gamePaths.size()) {
		return nullptr;
	}

	return &m_gamePaths[gamePathIndex].second;
}

const std::vector<std::pair<std::string, std::string>> & GameLocator::getGamePaths() const {
	return m_gamePaths;
}

bool GameLocator::removeGamePath(size_t index) {
	if(index >= m_gamePaths.size()) {
		return false;
	}

	m_gamePaths.erase(m_gamePaths.begin() + index);

	return true;
}

bool GameLocator::removeGamePath(const std::string & gamePath) {
	return removeGamePath(indexOfGamePath(gamePath));
}

bool GameLocator::removeGamePathWithID(const std::string & gameVersionID) {
	return removeGamePath(indexOfGamePathWithID(gameVersionID));
}

void GameLocator::clearGamePaths() {
	m_gamePaths.clear();
}
