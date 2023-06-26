#include "GameFileFactoryRegistry.h"

#include "Animation/ANM/AnimationANM.h"
#include "Art/Art.h"
#include "Group/GRP/GroupGRP.h"
#include "Group/SSI/GroupSSI.h"
#include "Map/Map.h"
#include "Palette/ACT/PaletteACT.h"
#include "Palette/DAT/PaletteDAT.h"
#include "Palette/GPL/PaletteGPL.h"
#include "Palette/JASC/PaletteJASC.h"
#include "Palette/PAL/PalettePAL.h"
#include "Zip/Zip.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

GameFileFactoryRegistry::GameFileFactoryRegistry()
	: m_defaultFactoriesAssigned(false) {
	assignDefaultFactories();
}

GameFileFactoryRegistry::~GameFileFactoryRegistry() { }

bool GameFileFactoryRegistry::hasFactory(const std::string & fileNameOrExtension) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_gameFileFactories.find(GameFileFactoryRegistry::formatFileNameOrExtension(fileNameOrExtension)) != m_gameFileFactories.cend();
}

bool GameFileFactoryRegistry::setFactory(const std::string & fileNameOrExtension, std::function<std::unique_ptr<GameFile>()> createNewGameFileFunction, std::function<std::unique_ptr<GameFile>(const ByteBuffer & data)> readGameFileFunction, std::function<std::unique_ptr<GameFile>(const std::string & filePath)> loadGameFileFunction) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(fileNameOrExtension.empty() || createNewGameFileFunction == nullptr || readGameFileFunction == nullptr || loadGameFileFunction == nullptr) {
		return false;
	}

	std::string formattedFileNameOrExtension(formatFileNameOrExtension(fileNameOrExtension));

	if(formattedFileNameOrExtension.empty()) {
		return false;
	}

	m_gameFileFactories.emplace(formattedFileNameOrExtension, GameFileFactoryData({
		createNewGameFileFunction,
		readGameFileFunction,
		loadGameFileFunction
	}));

	return true;
}

size_t GameFileFactoryRegistry::setFactory(const std::vector<std::string> & fileNamesOrExtensions, std::function<std::unique_ptr<GameFile>()> createNewGameFileFunction, std::function<std::unique_ptr<GameFile>(const ByteBuffer & data)> readGameFileFunction, std::function<std::unique_ptr<GameFile>(const std::string & filePath)> loadGameFileFunction) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	size_t numberOfFactoriesSet = 0;

	for(const std::string & fileNameOrExtension : fileNamesOrExtensions) {
		if(setFactory(fileNameOrExtension, createNewGameFileFunction, readGameFileFunction, loadGameFileFunction)) {
			numberOfFactoriesSet++;
		}
	}

	return numberOfFactoriesSet;
}

bool GameFileFactoryRegistry::removeFactory(const std::string & fileNameOrExtension) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(fileNameOrExtension.empty()) {
		return false;
	}

	std::string formattedFileNameOrExtension(formatFileNameOrExtension(fileNameOrExtension));

	if(formattedFileNameOrExtension.empty()) {
		return false;
	}

	GameFileFactoryMap::const_iterator factoryDataIterator(m_gameFileFactories.find(formattedFileNameOrExtension));

	if(factoryDataIterator == m_gameFileFactories.cend()) {
		return false;
	}

	m_gameFileFactories.erase(factoryDataIterator);

	return true;
}

void GameFileFactoryRegistry::resetFactories() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_gameFileFactories.clear();
}

bool GameFileFactoryRegistry::areDefaultFactoriesAssigned() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_defaultFactoriesAssigned;
}

void GameFileFactoryRegistry::assignDefaultFactories() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	setFactory("act", []() {
		return std::make_unique<PaletteACT>();
	}, [](const ByteBuffer & data) {
		return PaletteACT::readFrom(data);
	}, [](const std::string & filePath) {
		return PaletteACT::loadFrom(filePath);
	});

	setFactory("anm", []() {
		return std::make_unique<AnimationANM>();
	}, [](const ByteBuffer & data) {
		return AnimationANM::readFrom(data);
	}, [](const std::string & filePath) {
		return AnimationANM::loadFrom(filePath);
	});

	setFactory("art", []() {
		return std::make_unique<Art>();
	}, [](const ByteBuffer & data) {
		return Art::readFrom(data);
	}, [](const std::string & filePath) {
		return Art::loadFrom(filePath);
	});

	setFactory("dat", []() {
		return std::make_unique<PaletteDAT>(PaletteDAT::Type::Palette);
	}, [](const ByteBuffer & data) {
		return PaletteDAT::readFrom(data);
	}, [](const std::string & filePath) {
		return PaletteDAT::loadFrom(filePath);
	});

	setFactory("gpl", []() {
		return std::make_unique<PaletteGPL>();
	}, [](const ByteBuffer & data) {
		return PaletteGPL::readFrom(data);
	}, [](const std::string & filePath) {
		return PaletteGPL::loadFrom(filePath);
	});

	setFactory("grp", []() {
		return std::make_unique<GroupGRP>();
	}, [](const ByteBuffer & data) {
		return GroupGRP::readFrom(data);
	}, [](const std::string & filePath) {
		return GroupGRP::loadFrom(filePath);
	});

	setFactory("jasc", []() {
		return std::make_unique<PaletteJASC>();
	}, [](const ByteBuffer & data) {
		return PaletteJASC::readFrom(data);
	}, [](const std::string & filePath) {
		return PaletteJASC::loadFrom(filePath);
	});

	setFactory("map", []() {
		return std::make_unique<Map>();
	}, [](const ByteBuffer & data) {
		return Map::readFrom(data);
	}, [](const std::string & filePath) {
		return Map::loadFrom(filePath);
	});

	setFactory("pal", []() {
		return std::make_unique<PalettePAL>();
	}, [](const ByteBuffer & data) {
		return PalettePAL::readFrom(data);
	}, [](const std::string & filePath) {
		return PalettePAL::loadFrom(filePath);
	});

	setFactory("ssi", []() {
		return std::make_unique<GroupSSI>();
	}, [](const ByteBuffer & data) {
		return GroupSSI::readFrom(data);
	}, [](const std::string & filePath) {
		return GroupSSI::loadFrom(filePath);
	});

	setFactory("zip", []() {
		return std::make_unique<Zip>();
	}, [](const ByteBuffer & data) {
		return Zip::readFrom(data);
	}, [](const std::string & filePath) {
		return Zip::loadFrom(filePath);
	});
}

std::unique_ptr<GameFile> GameFileFactoryRegistry::createNewGameFile(const std::string & filePathOrExtension) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	GameFileFactoryMap::const_iterator gameFileFactoryIterator(getGameFileFactoryForFilePath(filePathOrExtension));

	if(gameFileFactoryIterator == m_gameFileFactories.cend()) {
		return nullptr;
	}

	return gameFileFactoryIterator->second.createNewGameFileFunction();
}

std::unique_ptr<GameFile> GameFileFactoryRegistry::readGameFileFrom(const ByteBuffer & data, const std::string & filePathOrExtension) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	GameFileFactoryMap::const_iterator gameFileFactoryIterator(getGameFileFactoryForFilePath(filePathOrExtension));

	if(gameFileFactoryIterator == m_gameFileFactories.cend()) {
		return nullptr;
	}

	return gameFileFactoryIterator->second.readGameFileFunction(data);
}

std::unique_ptr<GameFile> GameFileFactoryRegistry::loadGameFileFrom(const std::string & filePath) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	GameFileFactoryMap::const_iterator gameFileFactoryIterator(getGameFileFactoryForFilePath(filePath));

	if(gameFileFactoryIterator == m_gameFileFactories.cend()) {
		return nullptr;
	}

	return gameFileFactoryIterator->second.loadGameFileFunction(filePath);
}

GameFileFactoryRegistry::GameFileFactoryMap::const_iterator GameFileFactoryRegistry::getGameFileFactoryForFilePath(const std::string & filePathOrExtension) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(filePathOrExtension.empty()) {
		return m_gameFileFactories.cend();
	}

	std::string formattedFileName(formatFileNameOrExtension(std::string(Utilities::getFileName(filePathOrExtension))));

	if(formattedFileName.empty()) {
		return m_gameFileFactories.cend();
	}

	GameFileFactoryMap::const_iterator gameFileFactoryIterator(m_gameFileFactories.find(formattedFileName));

	if(gameFileFactoryIterator != m_gameFileFactories.cend()) {
		return gameFileFactoryIterator;
	}

	std::string formattedFileExtension(formatFileNameOrExtension(std::string(Utilities::getFileExtension(filePathOrExtension))));

	if(formattedFileExtension.empty()) {
		formattedFileExtension = filePathOrExtension;
	}

	gameFileFactoryIterator = m_gameFileFactories.find(formattedFileExtension);

	if(gameFileFactoryIterator != m_gameFileFactories.cend()) {
		return gameFileFactoryIterator;
	}

	return std::find_if(m_gameFileFactories.cbegin(), m_gameFileFactories.cend(), [&filePathOrExtension](const auto & gameFileFactory) {
		return Utilities::endsWith(filePathOrExtension, gameFileFactory.first, false);
	});
}

std::string GameFileFactoryRegistry::formatFileNameOrExtension(const std::string & fileNameOrExtension) {
	return Utilities::toLowerCase(Utilities::trimString(fileNameOrExtension));
}
