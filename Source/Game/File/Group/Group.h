#ifndef _GROUP_H_
#define _GROUP_H_

#include "../GameFile.h"
#include "GroupFile.h"

#include <ByteBuffer.h>

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
	size_t indexOfFile(const GroupFile & file) const;
	size_t indexOfFileWithName(const std::string & fileName) const;
	size_t indexOfFirstFileWithExtension(const std::string & extension) const;
	size_t indexOfLastFileWithExtension(const std::string & extension) const;
	std::shared_ptr<GroupFile> getFile(size_t index) const;
	std::shared_ptr<GroupFile> getFileWithName(const std::string & fileName) const;
	std::shared_ptr<GroupFile> getFirstFileWithExtension(const std::string & extension) const;
	std::shared_ptr<GroupFile> getLastFileWithExtension(const std::string & extension) const;
	const std::vector<std::shared_ptr<GroupFile>> & getFiles() const;
	std::vector<std::shared_ptr<GroupFile>> getFilesWithExtension(const std::string & extension) const;
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
	void updateParent();

	std::vector<std::shared_ptr<GroupFile>> m_files;
	std::vector<boost::signals2::connection> m_fileConnections;
};

#endif // _GROUP_H_
