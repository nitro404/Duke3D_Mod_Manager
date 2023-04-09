#ifndef _MOD_VERSION_H_
#define _MOD_VERSION_H_

#include <Date.h>

#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class GameVersion;
class Mod;
class ModVersionType;

namespace tinyxml2 {
	class XMLDocument;
	class XMLElement;
}

class ModVersion final {
	friend class Mod;

public:
	ModVersion(const std::string & version = std::string(), std::optional<Date> releaseDate = {});
	ModVersion(ModVersion && m) noexcept;
	ModVersion(const ModVersion & m);
	ModVersion & operator = (ModVersion && m) noexcept;
	ModVersion & operator = (const ModVersion & m);
	~ModVersion();

	bool isDefault() const;
	bool isStandAlone() const;
	const std::string & getVersion() const;
	std::string getFullName(size_t versionTypeIndex = std::numeric_limits<size_t>::max()) const;
	const std::optional<Date> & getReleaseDate() const;
	std::string getReleaseDateAsString() const;
	bool isRepaired() const;
	std::optional<bool> getRepaired() const;
	const Mod * getParentMod() const;
	void setVersion(const std::string & version);
	bool setReleaseDate(const Date & releaseDate);
	void clearReleaseDate();
	void setRepaired(bool repaired);
	void clearRepaired();

	size_t numberOfTypes() const;
	bool hasType(const std::string & type) const;
	bool hasType(const ModVersionType & type) const;
	size_t indexOfType(const std::string & type) const;
	size_t indexOfType(const ModVersionType & type) const;
	std::shared_ptr<ModVersionType> getType(size_t index) const;
	std::shared_ptr<ModVersionType> getType(const std::string & type) const;
	const std::vector<std::shared_ptr<ModVersionType>> & getTypes() const;
	std::vector<std::string> getTypeDisplayNames(const std::string & emptySubstitution = {}) const;
	bool addType(const ModVersionType & type);
	bool removeType(size_t index);
	bool removeType(const std::string & type);
	bool removeType(const ModVersionType & type);
	void clearTypes();

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument * document) const;
	static std::unique_ptr<ModVersion> parseFrom(const rapidjson::Value & modVersionValue, const rapidjson::Value & modValue, bool skipFileInfoValidation = false);
	static std::unique_ptr<ModVersion> parseFrom(const tinyxml2::XMLElement * modVersionElement, bool skipFileInfoValidation = false);

	bool isGameVersionSupported(const GameVersion & gameVersion) const;
	bool isGameVersionCompatible(const GameVersion & gameVersion) const;

	bool isValid(bool skipFileInfoValidation = false) const;
	static bool isValid(const ModVersion * m, bool skipFileInfoValidation = false);

	bool operator == (const ModVersion & m) const;
	bool operator != (const ModVersion & m) const;

protected:
	void setParentMod(const Mod * mod);
	void updateParent();

private:
	std::string m_version;
	std::optional<Date> m_releaseDate;
	std::optional<bool> m_repaired;
	std::vector<std::shared_ptr<ModVersionType>> m_types;
	const Mod * m_parentMod;
};

#endif // _MOD_VERSION_H_
