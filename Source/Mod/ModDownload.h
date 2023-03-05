#ifndef _MOD_DOWNLOAD_H_
#define _MOD_DOWNLOAD_H_

#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

class Mod;
class ModVersion;
class ModVersionType;

namespace tinyxml2 {
	class XMLDocument;
	class XMLElement;
}

class ModDownload final {
	friend class Mod;

public:
	ModDownload(const std::string & fileName, uint64_t fileSize, const std::string & type, const std::string & sha1 = {});
	ModDownload(ModDownload && d) noexcept;
	ModDownload(const ModDownload & d);
	ModDownload & operator = (ModDownload && d) noexcept;
	ModDownload & operator = (const ModDownload & d);
	~ModDownload();

	const std::string & getFileName() const;
	uint64_t getFileSize() const;
	uint8_t getPartNumber() const;
	uint8_t getPartCount() const;
	const std::string & getVersion() const;
	const std::string & getVersionType() const;
	const std::string & getSpecial() const;
	const std::string & getGameVersion() const;
	const std::string & getType() const;
	bool isOriginalFiles() const;
	bool isModManagerFiles() const;
	const std::string & getSubfolder() const;
	const std::string & getSHA1() const;
	bool isEDuke32() const;
	bool isConverted() const;
	std::optional<bool> getConverted() const;
	bool isCorrupted() const;
	std::optional<bool> getCorrupted() const;
	bool isRepaired() const;
	std::optional<bool> getRepaired() const;
	const Mod * getParentMod() const;
	std::shared_ptr<ModVersion> getModVersion() const;
	std::shared_ptr<ModVersionType> getModVersionType() const;

	void setFileName(const std::string & fileName);
	void setFileSize(uint64_t fileSize);
	void setPartNumber(uint8_t partNumber);
	void setPartCount(uint8_t partCount);
	void setVersion(const std::string & version);
	void setVersionType(const std::string & versionType);
	void setSpecial(const std::string & special);
	void setGameVersion(const std::string & data);
	void setType(const std::string & type);
	void setSHA1(const std::string & sha1);
	void setConverted(bool converted);
	void clearConverted();
	void setCorrupted(bool corrupted);
	void clearCorrupted();
	void setRepaired(bool repaired);
	void clearRepaired();

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument * document) const;
	static std::unique_ptr<ModDownload> parseFrom(const rapidjson::Value & modDownloadValue, bool skipFileInfoValidation = false);
	static std::unique_ptr<ModDownload> parseFrom(const tinyxml2::XMLElement * modDownloadElement, bool skipFileInfoValidation = false);

	bool isValid(bool skipFileInfoValidation = false) const;
	static bool isValid(const ModDownload * d, bool skipFileInfoValidation = false);

	bool operator == (const ModDownload & d) const;
	bool operator != (const ModDownload & d) const;

	static const std::string ORIGINAL_FILES_TYPE;
	static const std::string MOD_MANAGER_FILES_TYPE;

protected:
	void setParentMod(const Mod * mod);

private:
	std::string m_fileName;
	uint64_t m_fileSize;
	uint8_t m_partNumber;
	uint8_t m_partCount;
	std::string m_version;
	std::string m_versionType;
	std::string m_special;
	std::string m_gameVersion;
	std::string m_type;
	std::string m_sha1;
	std::optional<bool> m_converted;
	std::optional<bool> m_corrupted;
	std::optional<bool> m_repaired;
	const Mod * m_parentMod;
};

#endif // _MOD_DOWNLOAD_H_
