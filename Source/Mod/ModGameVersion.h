#ifndef _MOD_GAME_VERSION_H_
#define _MOD_GAME_VERSION_H_

#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class GameVersion;
class Mod;
class ModFile;
class ModVersion;
class ModVersionType;

namespace tinyxml2 {
	class XMLDocument;
	class XMLElement;
}

class ModGameVersion final {
	friend class ModVersionType;

public:
	ModGameVersion(const std::string & gameVersion, bool converted = false);
	ModGameVersion(ModGameVersion && m) noexcept;
	ModGameVersion(const ModGameVersion & m);
	ModGameVersion & operator = (ModGameVersion && m) noexcept;
	ModGameVersion & operator = (const ModGameVersion & m);
	~ModGameVersion();

	const std::string & getGameVersion() const;
	std::string getFullName() const;
	bool isEDuke32() const;
	bool isConverted() const;
	const Mod * getParentMod() const;
	const ModVersion * getParentModVersion() const;
	const ModVersionType * getParentModVersionType() const;
	void setGameVersion(const std::string & data);
	void setConverted(bool converted);

	size_t numberOfFiles() const;
	size_t numberOfFilesOfType(const std::string & fileType);
	bool hasFile(const ModFile & file) const;
	bool hasFile(const std::string & fileName) const;
	bool hasFileOfType(const std::string & fileType) const;
	size_t indexOfFile(const ModFile & file) const;
	size_t indexOfFile(const std::string & fileName) const;
	size_t indexOfFirstFileOfType(const std::string & fileType) const;
	std::shared_ptr<ModFile> getFile(size_t index) const;
	std::shared_ptr<ModFile> getFile(const std::string & fileName) const;
	std::shared_ptr<ModFile> getFirstFileOfType(const std::string & fileType) const;
	std::vector<std::shared_ptr<ModFile>> getFilesOfType(const std::string & fileType) const;
	std::optional<std::string> getFirstFileNameOfType(const std::string & fileType) const;
	std::vector<std::string> getFileNamesOfType(const std::string & fileType) const;
	const std::vector<std::shared_ptr<ModFile>> & getFiles() const;
	bool addFile(const ModFile & file);
	bool removeFile(size_t index);
	bool removeFile(const ModFile & file);
	bool removeFile(const std::string & fileName);
	size_t removeFilesOfType(const std::string & fileType);
	void clearFiles();

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument * document) const;
	static std::unique_ptr<ModGameVersion> parseFrom(const rapidjson::Value & modGameVersionValue);
	static std::unique_ptr<ModGameVersion> parseFrom(const tinyxml2::XMLElement * modGameVersionElement);

	bool isGameVersionSupported(const GameVersion & gameVersion) const;
	bool isGameVersionCompatible(const GameVersion & gameVersion) const;

	bool isValid() const;
	static bool isValid(const ModGameVersion * m);

	bool operator == (const ModGameVersion & m) const;
	bool operator != (const ModGameVersion & m) const;

protected:
	void setParentModVersionType(const ModVersionType * modVersionType);
	void updateParent();

private:
	std::string m_gameVersion;
	bool m_converted;
	std::vector<std::shared_ptr<ModFile>> m_files;
	const ModVersionType * m_parentModVersionType;
};

#endif // _MOD_GAME_VERSION_H_
