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
	GroupFile(const std::string & fileName, const uint8_t * data, size_t dataSize, const uint8_t * trailingData = nullptr, size_t trailingDataSize = 0);
	GroupFile(const std::string & fileName, const std::vector<uint8_t> & data, const std::vector<uint8_t> & trailingData);
	GroupFile(const std::string & fileName, const ByteBuffer & data, const ByteBuffer & trailingData = {});
	GroupFile(const std::string & fileName, std::unique_ptr<ByteBuffer> data, std::unique_ptr<ByteBuffer> trailingData = nullptr);
	GroupFile(GroupFile && file) noexcept;
	GroupFile(const GroupFile & file);
	GroupFile & operator = (GroupFile && file) noexcept;
	GroupFile & operator = (const GroupFile & file);
	~GroupFile();

	bool isModified() const;
	const std::string & getFileName() const;
	bool hasFileExtension(std::string_view fileExtension) const;
	std::string_view getFileExtension() const;
	size_t getSize() const;
	size_t getTrailingDataSize() const;
	std::string getSizeAsString() const;
	bool hasData() const;
	const ByteBuffer & getData() const;
	ByteBuffer & getData();
	std::unique_ptr<ByteBuffer> transferData();
	bool hasTrailingData() const;
	const ByteBuffer & getTrailingData() const;
	std::unique_ptr<ByteBuffer> transferTrailingData();

	bool setFileName(const std::string & newFileName);
	void setData(const uint8_t * data, size_t size);
	void setData(std::vector<uint8_t> && data) noexcept;
	void setData(const std::vector<uint8_t> & data);
	void setData(const ByteBuffer & data);
	void setData(std::unique_ptr<ByteBuffer> data);
	void clearData();
	void setTrailingData(const uint8_t * data, size_t size);
	void setTrailingData(std::vector<uint8_t> && data) noexcept;
	void setTrailingData(const std::vector<uint8_t> & data);
	void setTrailingData(const ByteBuffer & data);
	void setTrailingData(std::unique_ptr<ByteBuffer> data);
	void clearTrailingData();

	Group * getParentGroup() const;

	bool isValid() const;
	static bool isValid(const GroupFile * file);

	bool writeTo(const std::string & directoryName, bool overwrite = DEFAULT_OVERWRITE_FILES, const std::string & alternateFileName = std::string(), bool createParentDirectories = true) const;

	static std::string formatFileName(std::string_view fileName);

	bool operator == (const GroupFile & file) const;
	bool operator != (const GroupFile & file) const;

	boost::signals2::signal<void (GroupFile & /* file */)> modified;

	static const uint8_t MAX_FILE_NAME_LENGTH;
	static const bool DEFAULT_OVERWRITE_FILES;

private:
	void setModified(bool modified);

	std::string m_fileName;
	std::unique_ptr<ByteBuffer> m_data;
	std::unique_ptr<ByteBuffer> m_trailingData;
	bool m_modified;
	mutable Group * m_parentGroup;
};

#endif // _GROUP_FILE_H_
