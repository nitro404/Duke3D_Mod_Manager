#ifndef _MOD_GAME_VERSION_H_
#define _MOD_GAME_VERSION_H_

#include "ModFile.h"

#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class GameVersion;
class Mod;
class ModDownload;
class ModVersion;
class ModVersionType;

namespace tinyxml2 {
	class XMLDocument;
	class XMLElement;
}

class ModGameVersion final {
	friend class ModVersionType;

public:
	ModGameVersion(const std::string & gameVersionID, bool converted = false);
	ModGameVersion(ModGameVersion && m) noexcept;
	ModGameVersion(const ModGameVersion & m);
	ModGameVersion & operator = (ModGameVersion && m) noexcept;
	ModGameVersion & operator = (const ModGameVersion & m);
	~ModGameVersion();

	const std::string & getGameVersionID() const;
	std::shared_ptr<GameVersion> getStandAloneGameVersion() const;
	std::string getFullName(bool includeGameVersionID) const;
	bool isStandAlone() const;
	bool isConverted() const;
	const Mod * getParentMod() const;
	const ModVersion * getParentModVersion() const;
	const ModVersionType * getParentModVersionType() const;
	void setGameVersionID(const std::string & gameVersionID);
	void setConverted(bool converted);

	size_t numberOfFiles() const;
	size_t numberOfFilesOfType(const std::string & fileType);
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	size_t numberOfFilesOfType(Arguments &&... arguments) const;
	bool hasFile(const ModFile & file) const;
	bool hasFile(const std::string & fileName) const;
	bool hasFileOfType(const std::string & fileType) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	bool hasFileOfType(Arguments &&... arguments) const;
	size_t indexOfFile(const ModFile & file) const;
	size_t indexOfFile(const std::string & fileName) const;
	size_t indexOfFirstFileOfType(const std::string & fileType) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	size_t indexOfFirstFileOfType(Arguments &&... arguments) const;
	size_t indexOfLastFileOfType(const std::string & fileType) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	size_t indexOfLastFileOfType(Arguments &&... arguments) const;
	std::shared_ptr<ModFile> getFile(size_t index) const;
	std::shared_ptr<ModFile> getFile(const std::string & fileName) const;
	std::shared_ptr<ModFile> getFirstFileOfType(const std::string & fileType) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	std::shared_ptr<ModFile> getFirstFileOfType(Arguments &&... arguments) const;
	std::shared_ptr<ModFile> getLastFileOfType(const std::string & fileType) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	std::shared_ptr<ModFile> getLastFileOfType(Arguments &&... arguments) const;
	std::vector<std::shared_ptr<ModFile>> getFilesOfType(const std::string & fileType) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	std::vector<std::shared_ptr<ModFile>> getFilesOfType(Arguments &&... arguments) const;
	std::optional<std::string> getFirstFileNameOfType(const std::string & fileType) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	std::optional<std::string> getFirstFileNameOfType(Arguments &&... arguments) const;
	std::optional<std::string> getLastFileNameOfType(const std::string & fileType) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	std::optional<std::string> getLastFileNameOfType(Arguments &&... arguments) const;
	std::vector<std::string> getFileNamesOfType(const std::string & fileType) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	std::vector<std::string> getFileNamesOfType(Arguments &&... arguments) const;
	const std::vector<std::shared_ptr<ModFile>> & getFiles() const;
	std::shared_ptr<ModDownload> getDownload() const;
	bool addFile(const ModFile & file);
	bool removeFile(size_t index);
	bool removeFile(const ModFile & file);
	bool removeFile(const std::string & fileName);
	size_t removeFilesOfType(const std::string & fileType);
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	size_t removeFilesOfType(Arguments &&... arguments);
	void clearFiles();

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument * document) const;
	static std::unique_ptr<ModGameVersion> parseFrom(const rapidjson::Value & modGameVersionValue, const rapidjson::Value & modValue, bool skipFileInfoValidation = false);
	static std::unique_ptr<ModGameVersion> parseFrom(const tinyxml2::XMLElement * modGameVersionElement, bool skipFileInfoValidation = false);

	bool isGameVersionSupported(const GameVersion & gameVersion) const;
	bool isGameVersionCompatible(const GameVersion & gameVersion) const;

	bool isValid(bool skipFileInfoValidation = false) const;
	static bool isValid(const ModGameVersion * m, bool skipFileInfoValidation = false);

	bool operator == (const ModGameVersion & m) const;
	bool operator != (const ModGameVersion & m) const;

protected:
	void setParentModVersionType(const ModVersionType * modVersionType);
	void updateParent();

private:
	std::string m_gameVersionID;
	std::shared_ptr<GameVersion> m_standAloneGameVersion;
	bool m_converted;
	std::vector<std::shared_ptr<ModFile>> m_files;
	const ModVersionType * m_parentModVersionType;
};

template <typename ...Arguments, typename>
size_t ModGameVersion::numberOfFilesOfType(Arguments &&... arguments) const {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};
	size_t fileCount = 0;

	for(const std::shared_ptr<ModFile> & modFile : m_files) {
		for(size_t i = 0; i < sizeof...(arguments); i++) {
			const std::string_view & fileType = unpackedArguments[i];

			if(!fileType.empty() && Utilities::areStringsEqualIgnoreCase(modFile->getType(), fileType)) {
				fileCount++;
			}
		}
	}

	return fileCount;
}

template <typename ...Arguments, typename>
bool ModGameVersion::hasFileOfType(Arguments &&... arguments) const {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};

	for(const std::shared_ptr<ModFile> & modFile : m_files) {
		for(size_t i = 0; i < sizeof...(arguments); i++) {
			const std::string_view & fileType = unpackedArguments[i];

			if(!fileType.empty() && Utilities::areStringsEqualIgnoreCase(modFile->getType(), fileType)) {
				return true;
			}
		}
	}

	return false;
}

template <typename ...Arguments, typename>
size_t ModGameVersion::indexOfFirstFileOfType(Arguments &&... arguments) const {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};

	for(const std::shared_ptr<ModFile> & modFile : m_files) {
		for(size_t i = 0; i < sizeof...(arguments); i++) {
			const std::string_view & fileType = unpackedArguments[i];

			if(!fileType.empty() && Utilities::areStringsEqualIgnoreCase(modFile->getType(), fileType)) {
				return i;
			}
		}
	}

	return std::numeric_limits<size_t>::max();
}

template <typename ...Arguments, typename>
size_t ModGameVersion::indexOfLastFileOfType(Arguments &&... arguments) const {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};

	for(std::vector<std::shared_ptr<ModFile>>::const_reverse_iterator i = m_files.crbegin(); i != m_files.crend(); ++i) {
		for(size_t j = 0; j < sizeof...(arguments); j++) {
			const std::string_view & fileType = unpackedArguments[j];

			if(!fileType.empty() && Utilities::areStringsEqualIgnoreCase((*i)->getType(), fileType)) {
				return m_files.crend() - i - 1;
			}
		}
	}

	return std::numeric_limits<size_t>::max();
}

template <typename ...Arguments, typename>
std::shared_ptr<ModFile> ModGameVersion::getFirstFileOfType(Arguments &&... arguments) const {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};

	for(const std::shared_ptr<ModFile> & modFile : m_files) {
		for(size_t i = 0; i < sizeof...(arguments); i++) {
			const std::string_view & fileType = unpackedArguments[i];

			if(!fileType.empty() && Utilities::areStringsEqualIgnoreCase(modFile->getType(), fileType)) {
				return modFile;
			}
		}
	}

	return nullptr;
}

template <typename ...Arguments, typename>
std::shared_ptr<ModFile> ModGameVersion::getLastFileOfType(Arguments &&... arguments) const {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};

	for(std::vector<std::shared_ptr<ModFile>>::const_reverse_iterator i = m_files.crbegin(); i != m_files.crend(); ++i) {
		for(size_t j = 0; j < sizeof...(arguments); j++) {
			const std::string_view & fileType = unpackedArguments[j];

			if(!fileType.empty() && Utilities::areStringsEqualIgnoreCase((*i)->getType(), fileType)) {
				return *i;
			}
		}
	}

	return nullptr;
}

template <typename ...Arguments, typename>
std::vector<std::shared_ptr<ModFile>> ModGameVersion::getFilesOfType(Arguments &&... arguments) const {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};
	std::vector<std::shared_ptr<ModFile>> files;

	for(const std::shared_ptr<ModFile> & modFile : m_files) {
		for(size_t i = 0; i < sizeof...(arguments); i++) {
			const std::string_view & fileType = unpackedArguments[i];

			if(!fileType.empty() && Utilities::areStringsEqualIgnoreCase(modFile->getType(), fileType)) {
				files.push_back(modFile);
			}
		}
	}

	return files;
}

template <typename ...Arguments, typename>
std::optional<std::string> ModGameVersion::getFirstFileNameOfType(Arguments &&... arguments) const {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};

	for(const std::shared_ptr<ModFile> & modFile : m_files) {
		for(size_t i = 0; i < sizeof...(arguments); i++) {
			const std::string_view & fileType = unpackedArguments[i];

			if(!fileType.empty() && Utilities::areStringsEqualIgnoreCase(modFile->getType(), fileType)) {
				return modFile->getFileName();
			}
		}
	}

	return {};
}

template <typename ...Arguments, typename>
std::optional<std::string> ModGameVersion::getLastFileNameOfType(Arguments &&... arguments) const {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};

	for(std::vector<std::shared_ptr<ModFile>>::const_reverse_iterator i = m_files.crbegin(); i != m_files.crend(); ++i) {
		for(size_t j = 0; j < sizeof...(arguments); j++) {
			const std::string_view & fileType = unpackedArguments[j];

			if(!fileType.empty() && Utilities::areStringsEqualIgnoreCase((*i)->getType(), fileType)) {
				return (*i)->getFileName();
			}
		}
	}

	return {};
}

template <typename ...Arguments, typename>
std::vector<std::string> ModGameVersion::getFileNamesOfType(Arguments &&... arguments) const {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};
	std::vector<std::string> fileNames;

	for(const std::shared_ptr<ModFile> & modFile : m_files) {
		for(size_t i = 0; i < sizeof...(arguments); i++) {
			const std::string_view & fileType = unpackedArguments[i];

			if(!fileType.empty() && Utilities::areStringsEqualIgnoreCase(modFile->getType(), fileType)) {
				fileNames.push_back(modFile->getFileName());
			}
		}
	}

	return fileNames;
}

template <typename ...Arguments, typename>
size_t ModGameVersion::removeFilesOfType(Arguments &&... arguments) {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};
	std::vector<std::shared_ptr<ModFile>> modFilesToRemove;
	size_t numberOfFilesRemoved = 0;

	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		for(size_t j = 0; j < sizeof...(arguments); j++) {
			const std::string_view & fileType = unpackedArguments[j];

			if(!fileType.empty() && Utilities::areStringsEqualIgnoreCase((*i)->getType(), fileType)) {
				modFilesToRemove.push_back(*i);
			}
		}
	}

	for(const std::shared_ptr<ModFile> & modFileToRemove : modFilesToRemove) {
		modFileToRemove->setParentModGameVersion(nullptr);

		std::vector<std::shared_ptr<ModFile>>::const_iterator modFileIterator(std::find(m_files.cbegin(), m_files.cend(), modFileToRemove));

		if(modFileIterator == m_files.cend()) {
			continue;
		}

		m_files.erase(modFileIterator);
		numberOfFilesRemoved++;
	}

	return numberOfFilesRemoved;
}

#endif // _MOD_GAME_VERSION_H_
