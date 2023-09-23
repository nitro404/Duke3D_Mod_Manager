#ifndef _MOD_VERSION_TYPE_H_
#define _MOD_VERSION_TYPE_H_

#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class GameVersion;
class GameVersionCollection;
class Mod;
class ModDependency;
class ModVersion;
class ModGameVersion;

namespace tinyxml2 {
	class XMLDocument;
	class XMLElement;
}

class ModVersionType final {
	friend class ModVersion;

public:
	ModVersionType(const std::string & type = std::string());
	ModVersionType(ModVersionType && t) noexcept;
	ModVersionType(const ModVersionType & t);
	ModVersionType & operator = (ModVersionType && t) noexcept;
	ModVersionType & operator = (const ModVersionType & t);
	~ModVersionType();

	bool isDefault() const;
	bool isStandAlone() const;
	const std::string & getType() const;
	std::string getFullName() const;
	const Mod * getParentMod() const;
	const ModVersion * getParentModVersion() const;
	void setType(const std::string & type);
	bool copyHiddenPropertiesFrom(const ModVersionType & modVersionType);

	size_t numberOfGameVersions() const;
	bool hasGameVersionWithID(const std::string & gameVersionID) const;
	bool hasGameVersion(const ModGameVersion & gameVersion) const;
	size_t indexOfGameVersionWithID(const std::string & gameVersionID) const;
	size_t indexOfGameVersion(const ModGameVersion & gameVersion) const;
	std::shared_ptr<ModGameVersion> getGameVersion(size_t index) const;
	std::shared_ptr<ModGameVersion> getGameVersionWithID(const std::string & gameVersionID) const;
	const std::vector<std::shared_ptr<ModGameVersion>> & getGameVersions() const;
	bool addGameVersion(const ModGameVersion & gameVersion);
	bool removeGameVersion(size_t index);
	bool removeGameVersionWithID(const std::string & gameVersionID);
	bool removeGameVersion(const ModGameVersion & gameVersion);
	void clearGameVersions();

	bool hasDependencies() const;
	size_t numberOfDependencies() const;
	bool hasDependency(const std::string & modID, const std::string & modVersion, const std::string & modVersionType) const;
	bool hasDependency(const ModDependency & modDependency) const;
	size_t indexOfDependency(const std::string & modID, const std::string & modVersion, const std::string & modVersionType) const;
	size_t indexOfDependency(const ModDependency & modDependency) const;
	std::shared_ptr<ModDependency> getDependency(size_t index) const;
	std::shared_ptr<ModDependency> getDependency(const std::string & modID, const std::string & modVersion, const std::string & modVersionType) const;
	std::shared_ptr<ModDependency> getDependency(const ModDependency & modDependency) const;
	const std::vector<std::shared_ptr<ModDependency>> & getDependencies() const;
	bool addDependency(const std::string & modID, const std::string & modVersion, const std::string & modVersionType);
	bool addDependency(const ModDependency & modDependency);
	bool addDependency(std::unique_ptr<ModDependency> modDependency);
	bool removeDependency(size_t index);
	bool removeDependency(const std::string & modID, const std::string & modVersion, const std::string & modVersionType);
	bool removeDependency(const ModDependency & modDependency);
	void clearDependencies();

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument * document) const;
	static std::unique_ptr<ModVersionType> parseFrom(const rapidjson::Value & modVersionTypeValue, const rapidjson::Value & modValue, bool skipFileInfoValidation = false);
	static std::unique_ptr<ModVersionType> parseFrom(const tinyxml2::XMLElement * modVersionTypeElement, bool skipFileInfoValidation = false);

	bool isGameVersionSupported(const GameVersion & gameVersion) const;
	bool isGameVersionCompatible(const GameVersion & gameVersion) const;
	std::vector<std::shared_ptr<ModGameVersion>> getCompatibleModGameVersions(const GameVersion & gameVersion) const;
	std::vector<std::string> getModGameVersionIdentifiers() const;
	std::vector<std::string> getModGameVersionLongNames(const GameVersionCollection & gameVersions) const;
	std::vector<std::string> getModGameVersionShortNames(const GameVersionCollection & gameVersions) const;
	std::vector<std::string> getCompatibleModGameVersionIdentifiers(const GameVersion & gameVersion) const;
	std::vector<std::string> getCompatibleModGameVersionLongNames(const GameVersion & gameVersion, const GameVersionCollection & gameVersions) const;
	std::vector<std::string> getCompatibleModGameVersionShortNames(const GameVersion & gameVersion, const GameVersionCollection & gameVersions) const;

	bool isValid(bool skipFileInfoValidation = false) const;
	static bool isValid(const ModVersionType * t, bool skipFileInfoValidation = false);

	bool operator == (const ModVersionType & t) const;
	bool operator != (const ModVersionType & t) const;

protected:
	bool hadXMLElement() const;
	void setParentModVersion(const ModVersion * modVersion);
	void updateParent();

private:
	std::string m_type;
	bool m_hadXMLElement;
	std::vector<std::shared_ptr<ModGameVersion>> m_gameVersions;
	std::vector<std::shared_ptr<ModDependency>> m_dependencies;
	const ModVersion * m_parentModVersion;
};

#endif // _MOD_VERSION_TYPE_H_
