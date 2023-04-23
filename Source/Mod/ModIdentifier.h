#ifndef _MOD_IDENTIFIER_H_
#define _MOD_IDENTIFIER_H_

#include <rapidjson/document.h>

#include <memory>
#include <optional>
#include <string>

class ModIdentifier;
class ModMatch;

class ModIdentifier final {
public:
	ModIdentifier(const std::string & name, const std::optional<std::string> & version = {}, const std::optional<std::string> & versionType = {});
	ModIdentifier(const ModMatch & modMatch);
	ModIdentifier(ModIdentifier && m) noexcept;
	ModIdentifier(const ModIdentifier & m);
	ModIdentifier & operator = (ModIdentifier && m) noexcept;
	ModIdentifier & operator = (const ModIdentifier & m);
	~ModIdentifier();

	bool isModIdentifier() const;
	bool isModVersionIdentifier() const;
	bool isModVersionTypeIdentifier() const;
	const std::string & getName() const;
	bool hasVersion() const;
	const std::optional<std::string> & getVersion() const;
	bool hasVersionType() const;
	const std::optional<std::string> & getVersionType() const;
	std::string getFullName() const;

	void setName(const std::string & name);
	void setVersion(const std::string & version);
	void clearVersion();
	void setVersionType(const std::string & versionType);
	void clearVersionType();

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<ModIdentifier> parseFrom(const rapidjson::Value & modIdentifierValue);

	bool isValid() const;
	static bool isValid(const ModIdentifier * m);

	bool operator == (const ModIdentifier & m) const;
	bool operator != (const ModIdentifier & m) const;

private:
	std::string m_name;
	std::optional<std::string> m_version;
	std::optional<std::string> m_versionType;
};

#endif // _MOD_IDENTIFIER_H_
