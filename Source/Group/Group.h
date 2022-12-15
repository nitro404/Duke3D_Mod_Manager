#ifndef _GROUP_H_
#define _GROUP_H_

#include "GroupFile.h"

#include <ByteBuffer.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Group final {
public:
	Group(std::string_view filePath);
	Group(Group && g) noexcept;
	Group(const Group & g);
	Group & operator = (Group && g) noexcept;
	Group & operator = (const Group & g);
	~Group();

	const std::string & getFilePath() const;
	std::string_view getFileName() const;
	std::string_view getFileExtension() const;
	void setFilePath(const std::string & filePath);

	size_t numberOfFiles() const;
	bool hasFile(const std::string & fileName) const;
	bool hasFile(const GroupFile & groupFile) const;
	size_t indexOfFile(const std::string & fileName) const;
	size_t indexOfFile(const GroupFile & groupFile) const;
	std::shared_ptr<GroupFile> getFile(size_t index) const;
	std::shared_ptr<GroupFile> getFile(const std::string & fileName) const;
	std::vector<std::shared_ptr<GroupFile>> getFilesWithExtension(const std::string & extension) const;
	std::vector<std::string> getFileExtensions() const;
	bool extractFile(size_t index, const std::string & directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	bool extractFile(const std::string & fileName, const std::string & directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	size_t extractAllFilesWithExtension(const std::string & extension, const std::string & directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	size_t extractAllFiles(const std::string & directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	bool addFile(const GroupFile & groupFile, bool replace = DEFAULT_REPLACE_FILES);
	bool addFiles(const std::vector<std::shared_ptr<GroupFile>> & files, bool replace = DEFAULT_REPLACE_FILES);
	bool addFiles(const Group & group, bool replace = DEFAULT_REPLACE_FILES);
	bool removeFile(size_t index);
	bool removeFile(const std::string & fileName);
	bool removeFile(const GroupFile & groupFile);
	size_t removeFiles(const std::vector<std::string> & fileNames);
	size_t removeFiles(const std::vector<std::shared_ptr<GroupFile>> & files);
	void clearFiles();

	std::string toString() const;

	bool load();
	bool save(bool overwrite = true) const;

	size_t getGroupSize() const;
	bool verifyAllFiles() const;

	bool operator == (const Group & g) const;
	bool operator != (const Group & g) const;

	static const bool DEFAULT_REPLACE_FILES;

	static const Endianness FILE_ENDIANNESS;

	// Note: This file name is defined inline in the header file so that it is available at compile time in other classes
	static inline const std::string DUKE_NUKEM_3D_GROUP_FILE_NAME = "DUKE3D.GRP";

	// Note: These SHA1 hashes are defined inline in the header file so that they are available at compile time in other classes
	static inline const std::string DUKE_NUKEM_3D_REGULAR_VERSION_GROUP_SHA1_FILE_HASH = "3d508eaf3360605b0204301c259bd898717cf468";
	static inline const std::string DUKE_NUKEM_3D_ATOMIC_EDITION_GROUP_SHA1_FILE_HASH = "4fdef8559e2d35b1727fe92f021df9c148cf696c";
	static inline const std::string DUKE_NUKEM_3D_WORLD_TOUR_GROUP_SHA1_FILE_HASH = "d745396afc3e734029ec2b9bd8b20bdb3a11b3a2";

	static const std::string HEADER_TEXT;

private:
	std::string m_filePath;
	std::vector<std::shared_ptr<GroupFile>> m_files;
};

#endif // _GROUP_H_
