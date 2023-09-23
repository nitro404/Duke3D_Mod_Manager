#ifndef _MOD_COLLECTION_H_
#define _MOD_COLLECTION_H_

#include <boost/signals2.hpp>
#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class GameVersion;
class GameVersionCollection;
class Mod;
class ModDependency;
class ModDownload;
class ModFile;
class ModGameVersion;
class ModVersion;
class ModVersionType;
class StandAloneMod;

namespace tinyxml2 {
	class XMLDocument;
	class XMLElement;
}

class ModCollection final {
public:
	ModCollection();
	ModCollection(ModCollection && m) noexcept;
	ModCollection(const ModCollection & m);
	ModCollection & operator = (ModCollection && m) noexcept;
	ModCollection & operator = (const ModCollection & m);
	~ModCollection();

	size_t numberOfMods() const;
	bool hasMod(const Mod & mod) const;
	bool hasModWithID(const std::string & id) const;
	bool hasModWithName(const std::string & name) const;
	size_t indexOfMod(const Mod & mod) const;
	size_t indexOfModWithID(const std::string & id) const;
	size_t indexOfModWithName(const std::string & name) const;
	std::shared_ptr<Mod> getMod(size_t index) const;
	std::shared_ptr<Mod> getModWithID(const std::string & id) const;
	std::shared_ptr<ModVersion> getModVersionWithModID(const std::string & id, const std::string & version) const;
	std::shared_ptr<ModVersionType> getModVersionTypeWithModID(const std::string & id, const std::string & version, const std::string & versionType) const;
	std::shared_ptr<ModGameVersion> getModGameVersionByIDWithModID(const std::string & id, const std::string & version, const std::string & versionType, const std::string & gameVersionID) const;
	std::shared_ptr<Mod> getModWithName(const std::string & name) const;
	std::shared_ptr<ModVersionType> getModVersionTypeFromDependency(const ModDependency & modDependency) const;
	std::shared_ptr<ModVersion> getStandAloneModVersion(const StandAloneMod & standAloneMod) const;
	const std::vector<std::shared_ptr<Mod>> & getMods() const;
	bool addMod(const Mod & mod);
	bool removeMod(size_t index);
	bool removeMod(const Mod & mod);
	bool removeModWithID(const std::string & id);
	bool removeModWithName(const std::string & name);
	void clearMods();
	bool copyHiddenPropertiesFrom(const ModCollection & modCollection);

	std::shared_ptr<ModGameVersion> getModDependencyGameVersion(const ModDependency & modDependency, const std::string & gameVersionID, const GameVersionCollection * gameVersions = nullptr, bool allowCompatibleGameVersions = true) const;
	std::vector<std::shared_ptr<ModGameVersion>> getModDependencyGameVersions(const std::vector<std::shared_ptr<ModDependency>> & modDependencies, const std::string & gameVersionID, const GameVersionCollection * gameVersions = nullptr, bool allowCompatibleGameVersions = true) const;
	std::vector<std::shared_ptr<ModGameVersion>> getModDependencyGameVersions(const ModGameVersion & modGameVersion, const GameVersionCollection * gameVersions = nullptr, bool allowCompatibleGameVersions = true) const;
	std::shared_ptr<ModDownload> getModDependencyDownload(const ModDependency & modDependency, const std::string & gameVersionID, const GameVersionCollection * gameVersions = nullptr, bool allowCompatibleGameVersions = true) const;
	std::vector<std::shared_ptr<ModDownload>> getModDependencyDownloads(const std::vector<std::shared_ptr<ModDependency>> & modDependencies, const std::string & gameVersionID, const GameVersionCollection * gameVersions = nullptr, bool allowCompatibleGameVersions = true) const;
	std::vector<std::shared_ptr<ModDownload>> getModDependencyDownloads(const ModGameVersion & modGameVersion, const GameVersionCollection * gameVersions = nullptr, bool allowCompatibleGameVersions = true) const;
	std::vector<std::shared_ptr<ModFile>> getModDependencyGroupFiles(const ModDependency & modDependency, const GameVersion & gameVersion, const GameVersionCollection * gameVersions = nullptr, bool allowCompatibleGameVersions = true, bool recursive = true) const;
	std::vector<std::shared_ptr<ModFile>> getModDependencyGroupFiles(const std::vector<std::shared_ptr<ModDependency>> & modDependencies, const GameVersion & gameVersion, const GameVersionCollection * gameVersions = nullptr, bool allowCompatibleGameVersions = true, bool recursive = true) const;
	std::vector<std::shared_ptr<ModFile>> getModDependencyGroupFiles(const ModGameVersion & modGameVersion, const GameVersion & gameVersion, const GameVersionCollection * gameVersions = nullptr, bool allowCompatibleGameVersions = true, bool recursive = true) const;

	rapidjson::Document toJSON() const;
	tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument* document) const;
	static std::unique_ptr<ModCollection> parseFrom(const rapidjson::Value & modCollectionValue, bool skipFileInfoValidation = false);
	static std::unique_ptr<ModCollection> parseFrom(const tinyxml2::XMLElement * modsElement, bool skipFileInfoValidation = false);

	bool loadFrom(const std::string & filePath, const GameVersionCollection * gameVersions = nullptr, bool skipFileInfoValidation = false);
	bool loadFromXML(const std::string & filePath, const GameVersionCollection * gameVersions = nullptr, bool skipFileInfoValidation = false);
	bool loadFromJSON(const std::string & filePath, const GameVersionCollection * gameVersions = nullptr, bool skipFileInfoValidation = false);
	bool saveTo(const std::string & filePath, bool overwrite = true) const;
	bool saveToXML(const std::string & filePath, bool overwrite = true) const;
	bool saveToJSON(const std::string & filePath, bool overwrite = true) const;

	bool checkGameVersions(const GameVersionCollection & gameVersions, bool verbose = true) const;

	bool isValid(const GameVersionCollection * gameVersions = nullptr, bool skipFileInfoValidation = false) const;
	static bool isValid(const ModCollection * m, const GameVersionCollection * gameVersions = nullptr, bool skipFileInfoValidation = false);

	bool operator == (const ModCollection & m) const;
	bool operator != (const ModCollection & m) const;

	boost::signals2::signal<void (ModCollection & /* mods */)> updated;

	static const std::string GAME_ID;
	static const std::string FILE_TYPE;
	static const std::string FILE_FORMAT_VERSION;

private:
	std::vector<std::shared_ptr<Mod>> m_mods;
};

#endif // _MOD_COLLECTION_H_
