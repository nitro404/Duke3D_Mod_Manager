#ifndef _MOD_IDENTIFIER_H_
#define _MOD_IDENTIFIER_H_

#include <rapidjson/document.h>

#include <memory>
#include <string>

class ModIdentifier;

class ModIdentifier final {
public:
	ModIdentifier(const std::string & name, const std::string & version = std::string(), const std::string & versionType = std::string());
	ModIdentifier(ModIdentifier && m) noexcept;
	ModIdentifier(const ModIdentifier & m);
	ModIdentifier & operator = (ModIdentifier && m) noexcept;
	ModIdentifier & operator = (const ModIdentifier & m);
	~ModIdentifier();

	const std::string & getName() const;
	const std::string & getVersion() const;
	const std::string & getVersionType() const;
	std::string getFullName() const;

	void setName(const std::string & name);
	void setVersion(const std::string & version);
	void setVersionType(const std::string & versionType);

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<ModIdentifier> parseFrom(const rapidjson::Value & modIdentifierValue);

	bool isValid() const;
	static bool isValid(const ModIdentifier * m);

	bool operator == (const ModIdentifier & m) const;
	bool operator != (const ModIdentifier & m) const;

private:
	std::string m_name;
	std::string m_version;
	std::string m_versionType;
};

#endif // _MOD_IDENTIFIER_H_
