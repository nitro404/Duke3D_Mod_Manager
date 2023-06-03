#ifndef _GROUP_FILE_H_
#define _GROUP_FILE_H_

#include <ByteBuffer.h>

#include <boost/signals2.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Group;

class GroupFile final {
	friend class Group;

public:
	GroupFile(const std::string & fileName);
	GroupFile(const std::string & fileName, const uint8_t * data, size_t size);
	GroupFile(const std::string & fileName, const std::vector<uint8_t> & data);
	GroupFile(const std::string & fileName, const ByteBuffer & data);
	GroupFile(const std::string & fileName, std::unique_ptr<ByteBuffer> data);
	GroupFile(GroupFile && f) noexcept;
	GroupFile(const GroupFile & f);
	GroupFile & operator = (GroupFile && f) noexcept;
	GroupFile & operator = (const GroupFile & f);
	~GroupFile();

	bool isModified() const;
	const std::string & getFileName() const;
	std::string_view getFileExtension() const;
	size_t getSize() const;
	std::string getSizeAsString() const;
	const ByteBuffer & getData() const;
	std::unique_ptr<ByteBuffer> transferData();

	bool setFileName(const std::string & newFileName);
	void setData(const uint8_t * data, size_t size);
	void setData(std::vector<uint8_t> && data) noexcept;
	void setData(const std::vector<uint8_t> & data);
	void setData(const ByteBuffer & data);
	void setData(std::unique_ptr<ByteBuffer> data);
	void clearData();

	Group * getParentGroup() const;

	bool isValid() const;
	static bool isValid(const GroupFile * f);

	bool writeTo(const std::string & directoryName, bool overwrite = DEFAULT_OVERWRITE_FILES, const std::string & alternateFileName = std::string(), bool createParentDirectories = true) const;

	static std::string formatFileName(std::string_view fileName);

	bool operator == (const GroupFile & f) const;
	bool operator != (const GroupFile & f) const;

	boost::signals2::signal<void (GroupFile & /* file */)> modified;

	static const uint8_t MAX_FILE_NAME_LENGTH;
	static const bool DEFAULT_OVERWRITE_FILES;

private:
	void setModified(bool modified);

	std::string m_fileName;
	std::unique_ptr<ByteBuffer> m_data;
	bool m_modified;
	mutable Group * m_parentGroup;
};

#endif // _GROUP_FILE_H_