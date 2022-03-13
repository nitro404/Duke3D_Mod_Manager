#ifndef _MOD_VERSION_TYPE_H_
#define _MOD_VERSION_TYPE_H_

#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class GameVersion;
class Mod;
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
	const std::string & getType() const;
	std::string getFullName() const;
	const Mod * getParentMod() const;
	const ModVersion * getParentModVersion() const;
	void setType(const std::string & type);

	size_t numberOfGameVersions() const;
	bool hasGameVersion(const std::string & gameVersion) const;
	bool hasGameVersion(const ModGameVersion & gameVersion) const;
	size_t indexOfGameVersion(const std::string & gameVersion) const;
	size_t indexOfGameVersion(const ModGameVersion & gameVersion) const;
	std::shared_ptr<ModGameVersion> getGameVersion(size_t index) const;
	std::shared_ptr<ModGameVersion> getGameVersion(const std::string & gameVersion) const;
	const std::vector<std::shared_ptr<ModGameVersion>> & getGameVersions() const;
	bool addGameVersion(const ModGameVersion & gameVersion);
	bool removeGameVersion(size_t index);
	bool removeGameVersion(const std::string & gameVersion);
	bool removeGameVersion(const ModGameVersion & gameVersion);
	void clearGameVersions();

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument * document) const;
	static std::unique_ptr<ModVersionType> parseFrom(const rapidjson::Value & modVersionTypeValue);
	static std::unique_ptr<ModVersionType> parseFrom(const tinyxml2::XMLElement * modVersionTypeElement);

	bool isGameVersionSupported(const GameVersion & gameVersion) const;
	bool isGameVersionCompatible(const GameVersion & gameVersion) const;

	bool isValid() const;
	static bool isValid(const ModVersionType * t);

	bool operator == (const ModVersionType & t) const;
	bool operator != (const ModVersionType & t) const;

protected:
	void setParentModVersion(const ModVersion * modVersion);
	void updateParent();

private:
	std::string m_type;
	std::vector<std::shared_ptr<ModGameVersion>> m_gameVersions;
	const ModVersion * m_parentModVersion;
};

#endif // _MOD_VERSION_TYPE_H_
