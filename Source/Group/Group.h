#ifndef _GROUP_H_
#define _GROUP_H_

#include "GroupFile.h"

#include <ByteBuffer.h>

#include <boost/signals2.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Group final {
public:
	Group(std::string_view filePath = {});
	Group(Group && g) noexcept;
	Group(const Group & g);
	Group & operator = (Group && g) noexcept;
	Group & operator = (const Group & g);
	~Group();

	bool isModified() const;
	const std::string & getFilePath() const;
	std::string_view getFileName() const;
	std::string_view getFileExtension() const;
	void setFilePath(const std::string & filePath);

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

	bool isValid(bool verifyParent = true) const;
	static bool isValid(const Group * g, bool verifyParent = true);

	static std::unique_ptr<Group> createFrom(const std::string & directoryPath);
	static std::unique_ptr<Group> loadFrom(const std::string & filePath);
	bool save(bool overwrite = true);

	size_t getGroupSize() const;
	std::string getGroupSizeAsString() const;

	bool operator == (const Group & g) const;
	bool operator != (const Group & g) const;

	boost::signals2::signal<void (Group & /* group */)> modified;

	static const bool DEFAULT_REPLACE_FILES;

	static const Endianness FILE_ENDIANNESS;

	// Note: This file name is defined inline in the header file so that it is available at compile time in other classes
	static inline const std::string DUKE_NUKEM_3D_GROUP_FILE_NAME = "DUKE3D.GRP";

	// Note: These SHA1 hashes are defined inline in the header file so that they are available at compile time in other classes
	static inline const std::string DUKE_NUKEM_3D_REGULAR_VERSION_GROUP_SHA1_FILE_HASH = "3d508eaf3360605b0204301c259bd898717cf468";
	static inline const std::string DUKE_NUKEM_3D_PLUTONIUM_PAK_GROUP_SHA1_FILE_HASH = "61e70f883df9552395406bf3d64f887f3c709438";
	static inline const std::string DUKE_NUKEM_3D_ATOMIC_EDITION_GROUP_SHA1_FILE_HASH = "4fdef8559e2d35b1727fe92f021df9c148cf696c";
	static inline const std::string DUKE_NUKEM_3D_WORLD_TOUR_GROUP_SHA1_FILE_HASH = "d745396afc3e734029ec2b9bd8b20bdb3a11b3a2";

	static const std::string HEADER_TEXT;

private:
	void setModified(bool modified);
	void onGroupFileModified(GroupFile & groupFile);
	void notifyGroupModified() const;
	void updateParentGroup();

	std::string m_filePath;
	std::vector<std::shared_ptr<GroupFile>> m_files;
	std::vector<boost::signals2::connection> m_fileConnections;
	bool m_modified;
};

#endif // _GROUP_H_
