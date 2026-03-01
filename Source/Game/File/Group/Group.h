#ifndef _GROUP_H_
#define _GROUP_H_

#include "../GameFile.h"
#include "GroupFile.h"

#include <ByteBuffer.h>
#include <Utilities/StringUtilities.h>

#include <boost/signals2.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Group : public GameFile {
public:
	Group(const std::string & filePath = {});
	Group(std::vector<std::unique_ptr<GroupFile>> groupFiles, const std::string & filePath = {});
	Group(Group && group) noexcept;
	Group(const Group & group);
	Group & operator = (Group && group) noexcept;
	Group & operator = (const Group & group);
	virtual ~Group();

	size_t numberOfFiles() const;
	bool hasFile(const GroupFile & file) const;
	bool hasFileWithName(const std::string & fileName) const;
	bool hasFileWithExtension(const std::string & extension) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	bool hasFileWithExtension(Arguments &&... arguments) const;
	size_t indexOfFile(const GroupFile & file) const;
	size_t indexOfFileWithName(const std::string & fileName) const;
	size_t indexOfFirstFileWithExtension(const std::string & extension) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	size_t indexOfFirstFileWithExtension(Arguments &&... arguments) const;
	size_t indexOfLastFileWithExtension(const std::string & extension) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	size_t indexOfLastFileWithExtension(Arguments &&... arguments) const;
	std::shared_ptr<GroupFile> getFile(size_t index) const;
	std::shared_ptr<GroupFile> getFileWithName(const std::string & fileName) const;
	std::shared_ptr<GroupFile> getFirstFileWithExtension(const std::string & extension) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	std::shared_ptr<GroupFile> getFirstFileWithExtension(Arguments &&... arguments) const;
	std::shared_ptr<GroupFile> getLastFileWithExtension(const std::string & extension) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	std::shared_ptr<GroupFile> getLastFileWithExtension(Arguments &&... arguments) const;
	const std::vector<std::shared_ptr<GroupFile>> & getFiles() const;
	std::vector<std::shared_ptr<GroupFile>> getFilesWithExtension(const std::string & extension) const;
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	std::vector<std::shared_ptr<GroupFile>> getFilesWithExtension(Arguments &&... arguments) const;
	std::vector<std::string> getFileExtensions() const;
	std::string getFileExtensionsAsString() const;
	std::shared_ptr<GroupFile> extractFile(size_t index, const std::string & directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	std::shared_ptr<GroupFile> extractFileWithName(const std::string & fileName, const std::string & directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	std::shared_ptr<GroupFile> extractFirstFileWithExtension(const std::string & extension, const std::string & directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	std::shared_ptr<GroupFile> extractLastFileWithExtension(const std::string & extension, const std::string & directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	std::vector<std::shared_ptr<GroupFile>> extractAllFilesWithExtension(const std::string & extension, const std::string & directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	std::vector<std::shared_ptr<GroupFile>> extractAllFiles(const std::string & directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	bool addFile(const std::string & filePath, bool replace = DEFAULT_REPLACE_FILES);
	bool addFile(std::unique_ptr<GroupFile> file, bool replace = DEFAULT_REPLACE_FILES);
	bool addFile(const GroupFile & file, bool replace = DEFAULT_REPLACE_FILES);
	bool addFiles(const std::vector<std::string> & filesPaths, bool replace = DEFAULT_REPLACE_FILES);
	bool addFiles(const std::vector<std::shared_ptr<GroupFile>> & files, bool replace = DEFAULT_REPLACE_FILES);
	bool addFiles(const Group & group, bool replace = DEFAULT_REPLACE_FILES);
	bool renameFile(size_t index, const std::string & newFileName);
	bool renameFile(const GroupFile & file, const std::string & newFileName);
	bool renameFile(const std::string & oldFileName, const std::string & newFileName);
	bool reverseFileExtension(size_t index);
	bool reverseFileExtension(const GroupFile & file);
	bool reverseFileExtensionOfFileWithName(const std::string & fileName);
	size_t reverseFileExtension(const std::string & fileExtension);
	size_t reverseFileExtensions(const std::vector<std::string> & fileExtensions);
	size_t reverseAllFileExtensions();
	bool swapFilePositions(size_t indexA, int indexB);
	bool swapFilePositions(const GroupFile & fileA, const GroupFile & fileB);
	bool swapFilePositionsByName(const std::string & fileNameA, const std::string & fileNameB);
	bool replaceFile(size_t index, const std::string & newFilePath, bool keepExistingFileName = true);
	bool replaceFile(size_t index, std::unique_ptr<GroupFile> file, bool keepExistingFileName = true);
	bool replaceFile(size_t index, const GroupFile & file, bool keepExistingFileName = true);
	bool replaceFile(size_t index, std::unique_ptr<ByteBuffer> data);
	bool replaceFile(size_t index, const std::vector<uint8_t> & data);
	bool replaceFile(const GroupFile & file, const std::string & newFilePath, bool keepExistingFileName = true);
	bool replaceFile(const GroupFile & file, std::unique_ptr<GroupFile> newFile, bool keepExistingFileName = true);
	bool replaceFile(const GroupFile & file, const GroupFile & newFile, bool keepExistingFileName = true);
	bool replaceFile(const GroupFile & file, std::unique_ptr<ByteBuffer> data);
	bool replaceFile(const GroupFile & file, const std::vector<uint8_t> & data);
	bool replaceFileWithName(const std::string & fileName, const std::string & newFilePath, bool keepExistingFileName = true);
	bool replaceFileWithName(const std::string & fileName, std::unique_ptr<GroupFile> newFile, bool keepExistingFileName = true);
	bool replaceFileWithName(const std::string & fileName, const GroupFile & newFile, bool keepExistingFileName = true);
	bool replaceFileWithName(const std::string & fileName, std::unique_ptr<ByteBuffer> data);
	bool replaceFileWithName(const std::string & fileName, const std::vector<uint8_t> & data);
	bool removeFile(size_t index);
	bool removeFile(const GroupFile & file);
	bool removeFileWithName(const std::string & fileName);
	size_t removeFiles(const std::vector<std::shared_ptr<GroupFile>> & files);
	size_t removeFilesByName(const std::vector<std::string> & fileNames);
	void clearFiles();

	std::string toString() const;

	virtual void addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const override;

	virtual bool isValid(bool verifyParent = true) const override;
	static bool isValid(const Group * group, bool verifyParent = true);

	static std::vector<std::unique_ptr<GroupFile>> createGroupFilesFromDirectory(const std::string & directoryPath);

	bool operator == (const Group & group) const;
	bool operator != (const Group & group) const;

	static const bool DEFAULT_REPLACE_FILES = true;

protected:
	virtual void setModified(bool modified) const override;
	void onGroupFileModified(GroupFile & groupFile);
	void connectSignals();
	void updateParent();

	std::vector<std::shared_ptr<GroupFile>> m_files;
	std::vector<boost::signals2::connection> m_fileConnections;
};

template <typename ...Arguments, typename>
bool Group::hasFileWithExtension(Arguments &&... arguments) const {
	return indexOfFirstFileWithExtension(arguments...) != std::numeric_limits<size_t>::max();
}

template <typename ...Arguments, typename>
size_t Group::indexOfFirstFileWithExtension(Arguments &&... arguments) const {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};

	for(std::vector<std::shared_ptr<GroupFile>>::const_iterator i = m_files.cbegin(); i != m_files.cend(); ++i) {
		std::string_view currentFileExtension((*i)->getFileExtension());

		if(currentFileExtension.empty()) {
			continue;
		}

		for(size_t j = 0; j < sizeof...(arguments); j++) {
			const std::string_view & extension = unpackedArguments[j];

			if(!extension.empty() && Utilities::areStringsEqualIgnoreCase(currentFileExtension, extension)) {
				return i - m_files.cbegin();
			}
		}
	}

	return std::numeric_limits<size_t>::max();
}

template <typename ...Arguments, typename>
size_t Group::indexOfLastFileWithExtension(Arguments &&... arguments) const {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};

	for(std::vector<std::shared_ptr<GroupFile>>::const_reverse_iterator i = m_files.crbegin(); i != m_files.crend(); ++i) {
		std::string_view currentFileExtension((*i)->getFileExtension());

		if(currentFileExtension.empty()) {
			continue;
		}

		for(size_t j = 0; j < sizeof...(arguments); j++) {
			const std::string_view & extension = unpackedArguments[j];

			if(!extension.empty() && Utilities::areStringsEqualIgnoreCase(currentFileExtension, extension)) {
				return m_files.crend() - i - 1;
			}
		}
	}

	return std::numeric_limits<size_t>::max();
}

template <typename ...Arguments, typename>
std::shared_ptr<GroupFile> Group::getFirstFileWithExtension(Arguments &&... arguments) const {
	return getFile(indexOfFirstFileWithExtension(arguments...));
}

template <typename ...Arguments, typename>
std::shared_ptr<GroupFile> Group::getLastFileWithExtension(Arguments &&... arguments) const {
	return getFile(indexOfLastFileWithExtension(arguments...));
}

template <typename ...Arguments, typename>
std::vector<std::shared_ptr<GroupFile>> Group::getFilesWithExtension(Arguments &&... arguments) const {
	std::string_view unpackedArguments[sizeof...(arguments)] = {arguments...};
	std::vector<std::shared_ptr<GroupFile>> files;

	for(const std::shared_ptr<GroupFile> & file : m_files) {
		std::string_view currentFileExtension(file->getFileExtension());

		if(currentFileExtension.empty()) {
			continue;
		}

		for(size_t i = 0; i < sizeof...(arguments); i++) {
			const std::string_view & extension = unpackedArguments[i];

			if(!extension.empty() && Utilities::areStringsEqualIgnoreCase(currentFileExtension, extension)) {
				files.push_back(file);
			}
		}
	}

	return files;
}

#endif // _GROUP_H_
