#ifndef _MOD_COLLECTION_H_
#define _MOD_COLLECTION_H_

#include "ModCollectionBroadcaster.h"

#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class GameVersionCollection;
class Mod;

namespace tinyxml2 {
	class XMLDocument;
	class XMLElement;
}

class ModCollection final : public ModCollectionBroadcaster {
public:
	ModCollection();
	ModCollection(ModCollection && m) noexcept;
	ModCollection(const ModCollection & m);
	ModCollection & operator = (ModCollection && m) noexcept;
	ModCollection & operator = (const ModCollection & m);
	virtual ~ModCollection();

	size_t numberOfMods() const;
	bool hasMod(const Mod & mod) const;
	bool hasMod(const std::string & id) const;
	bool hasModWithName(const std::string & name) const;
	size_t indexOfMod(const Mod & mod) const;
	size_t indexOfMod(const std::string & id) const;
	size_t indexOfModWithName(const std::string & name) const;
	std::shared_ptr<Mod> getMod(size_t index) const;
	std::shared_ptr<Mod> getMod(const std::string & id) const;
	std::shared_ptr<Mod> getModWithName(const std::string & name) const;
	const std::vector<std::shared_ptr<Mod>> & getMods() const;
	bool addMod(const Mod & mod);
	bool removeMod(size_t index);
	bool removeMod(const Mod & mod);
	bool removeMod(const std::string & id);
	bool removeModWithName(const std::string & name);
	void clearMods();

	rapidjson::Document toJSON() const;
	tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument* document) const;
	static std::unique_ptr<ModCollection> parseFrom(const rapidjson::Value & modCollectionValue);
	static std::unique_ptr<ModCollection> parseFrom(const tinyxml2::XMLElement * modsElement);

	bool loadFrom(const std::string & filePath);
	bool loadFromXML(const std::string & filePath);
	bool loadFromJSON(const std::string & filePath);
	bool saveTo(const std::string & filePath, bool overwrite = true) const;
	bool saveToXML(const std::string & filePath, bool overwrite = true) const;
	bool saveToJSON(const std::string & filePath, bool overwrite = true) const;

	bool checkGameVersions(const GameVersionCollection & gameVersions, bool verbose = true) const;

	bool isValid() const;
	static bool isValid(const ModCollection * m);

	bool operator == (const ModCollection & m) const;
	bool operator != (const ModCollection & m) const;

	static const std::string GAME_ID;
	static const std::string FILE_FORMAT_VERSION;

private:
	void notifyCollectionChanged() const;

private:
	std::vector<std::shared_ptr<Mod>> m_mods;
};

#endif // _MOD_COLLECTION_H_
