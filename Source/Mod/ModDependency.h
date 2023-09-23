#ifndef _MOD_DEPENDENCY_H_
#define _MOD_DEPENDENCY_H_

#include <rapidjson/document.h>

#include <memory>
#include <string>

namespace tinyxml2 {
	class XMLDocument;
	class XMLElement;
}

class ModDependency final {
public:
	ModDependency(const std::string & name, const std::string & version = {}, const std::string & versionType = {});
	ModDependency(ModDependency && dependency) noexcept;
	ModDependency(const ModDependency & dependency);
	ModDependency & operator = (ModDependency && dependency) noexcept;
	ModDependency & operator = (const ModDependency & dependency);
	~ModDependency();

	const std::string & getID() const;
	bool hasVersion() const;
	const std::string & getVersion() const;
	bool hasVersionType() const;
	const std::string & getVersionType() const;

	bool setID(const std::string & name);
	void setVersion(const std::string & version);
	void clearVersion();
	void setVersionType(const std::string & versionType);
	void clearVersionType();

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument * document) const;
	static std::unique_ptr<ModDependency> parseFrom(const rapidjson::Value & modDependencyValue);
	static std::unique_ptr<ModDependency> parseFrom(const tinyxml2::XMLElement * modDependencyElement);

	std::string toString() const;

	bool isValid() const;
	static bool isValid(const ModDependency * dependency);

	bool operator == (const ModDependency & dependency) const;
	bool operator != (const ModDependency & dependency) const;

private:
	std::string m_id;
	std::string m_version;
	std::string m_versionType;
};

#endif // _MOD_DEPENDENCY_H_
