#ifndef _GAME_FILE_FACTORY_REGISTRY_H_
#define _GAME_FILE_FACTORY_REGISTRY_H_

#include "GameFile.h"

#include <ByteBuffer.h>
#include <Singleton/Singleton.h>

#include <functional>
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

class GameFileFactoryRegistry final : public Singleton<GameFileFactoryRegistry> {
public:
	GameFileFactoryRegistry();
	virtual ~GameFileFactoryRegistry();

	bool hasFactory(const std::string & fileNameOrExtension) const;
	bool setFactory(const std::string & fileNameOrExtension, std::function<std::unique_ptr<GameFile>()> createNewGameFileFunction, std::function<std::unique_ptr<GameFile>(const ByteBuffer & data)> readGameFileFunction, std::function<std::unique_ptr<GameFile>(const std::string & filePath)> loadGameFileFunction);
	size_t setFactory(const std::vector<std::string> & fileNamesOrExtensions, std::function<std::unique_ptr<GameFile>()> createNewGameFileFunction, std::function<std::unique_ptr<GameFile>(const ByteBuffer & data)> readGameFileFunction, std::function<std::unique_ptr<GameFile>(const std::string & filePath)> loadGameFileFunction);
	bool removeFactory(const std::string & fileNameOrExtension);
	void resetFactories();
	bool areDefaultFactoriesAssigned() const;
	void assignDefaultFactories();

	std::unique_ptr<GameFile> createNewGameFile(const std::string & filePathOrExtension);
	std::unique_ptr<GameFile> readGameFileFrom(const ByteBuffer & data, const std::string & filePathOrExtension);
	std::unique_ptr<GameFile> loadGameFileFrom(const std::string & filePath);

private:
	struct GameFileFactoryData {
		std::function<std::unique_ptr<GameFile>()> createNewGameFileFunction;
		std::function<std::unique_ptr<GameFile>(const ByteBuffer & /* data */)> readGameFileFunction;
		std::function<std::unique_ptr<GameFile>(const std::string /* filePath */)> loadGameFileFunction;
	};

	typedef std::map<std::string, GameFileFactoryData> GameFileFactoryMap;

	GameFileFactoryMap::const_iterator getGameFileFactoryForFilePath(const std::string & filePathOrExtension) const;

	static std::string formatFileNameOrExtension(const std::string & fileExtension);

	GameFileFactoryMap m_gameFileFactories;
	bool m_defaultFactoriesAssigned;
	mutable std::recursive_mutex m_mutex;

	GameFileFactoryRegistry(const GameFileFactoryRegistry &) = delete;
	const GameFileFactoryRegistry & operator = (const GameFileFactoryRegistry &) = delete;
};

#endif // _GAME_FILE_FACTORY_REGISTRY_H_
