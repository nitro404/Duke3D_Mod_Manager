#ifndef _MOD_FILE_H_
#define _MOD_FILE_H_

#include <rapidjson/document.h>

#include <memory>
#include <optional>
#include <string>

class Mod;
class ModGameVersion;
class ModVersion;
class ModVersionType;

namespace tinyxml2 {
	class XMLDocument;
	class XMLElement;
}

class ModFile final {
	friend class ModGameVersion;

public:
	ModFile(const std::string & name, uint64_t fileSize, const std::string & type, const std::string & sha1 = std::string());
	ModFile(ModFile && f) noexcept;
	ModFile(const ModFile & f);
	ModFile & operator = (ModFile && f) noexcept;
	ModFile & operator = (const ModFile & f);
	~ModFile();

	const std::string & getFileName() const;
	std::string_view getFileExtension() const;
	uint64_t getFileSize() const;
	const std::string & getType() const;
	const std::string & getSHA1() const;
	bool isShared() const;
	std::optional<bool> getShared() const;
	const Mod * getParentMod() const;
	const ModVersion * getParentModVersion() const;
	const ModVersionType * getParentModVersionType() const;
	const ModGameVersion * getParentModGameVersion() const;

	void setFileName(const std::string & fileName);
	void setFileSize(uint64_t fileSize);
	void setType(const std::string & type);
	void setSHA1(const std::string & sha1);
	void setShared(bool shared);
	void clearShared();

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument * document) const;
	static std::unique_ptr<ModFile> parseFrom(const rapidjson::Value & modFileValue, bool skipFileInfoValidation = false);
	static std::unique_ptr<ModFile> parseFrom(const tinyxml2::XMLElement * modFileElement, bool skipFileInfoValidation = false);

	bool isValid(bool skipFileInfoValidation = false) const;
	static bool isValid(const ModFile * f, bool skipFileInfoValidation = false);

	bool operator == (const ModFile & f) const;
	bool operator != (const ModFile & f) const;

protected:
	void setParentModGameVersion(const ModGameVersion * modGameVersion);

private:
	std::string m_fileName;
	uint64_t m_fileSize;
	bool m_hadFileSizeAttribute;
	std::string m_type;
	std::string m_sha1;
	std::optional<bool> m_shared;
	const ModGameVersion * m_parentModGameVersion;
};

#endif // _MOD_FILE_H_
