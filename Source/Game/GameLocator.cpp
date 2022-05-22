#include "GameLocator.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <filesystem>

static const std::string DUKE_NUKEM_3D_GROUP_FILE_NAME("DUKE3D.GRP");

GameLocator::GameLocator() { }

GameLocator::~GameLocator() { }

bool GameLocator::locateGames() {
	std::vector<std::string> gameSearchPaths(getGameSearchPaths());

	m_gamePaths.clear();

	for(std::vector<std::string>::const_iterator i = gameSearchPaths.cbegin(); i != gameSearchPaths.cend(); ++i) {
		if(!std::filesystem::is_directory(std::filesystem::path(*i))) {
			continue;
		}

		std::string groupFilePath(Utilities::joinPaths(*i, DUKE_NUKEM_3D_GROUP_FILE_NAME));

		if(!std::filesystem::is_regular_file(std::filesystem::path(groupFilePath))) {
			continue;
		}

		m_gamePaths.emplace_back(*i);
	}

	return !m_gamePaths.empty();
}

size_t GameLocator::numberOfGamePaths() const {
	return m_gamePaths.size();
}

bool GameLocator::hasGamePath(const std::string & gamePath) const {
	return indexOfGamePath(gamePath) != std::numeric_limits<size_t>::max();
}

size_t GameLocator::indexOfGamePath(const std::string & gamePath) const {
	if(gamePath.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_gamePaths.size(); i++) {
		if(Utilities::areStringsEqual(m_gamePaths[i], gamePath)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

const std::string * GameLocator::getGamePath(size_t index) const {
	if(index >= m_gamePaths.size()) {
		return nullptr;
	}

	return &m_gamePaths[index];
}

const std::vector<std::string> & GameLocator::getGamePaths() {
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

void GameLocator::clearGamePaths() {
	m_gamePaths.clear();
}
