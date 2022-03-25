#ifndef _GROUP_FILE_H_
#define _GROUP_FILE_H_

#include <ByteBuffer.h>

#include <cstdint>
#include <string>
#include <vector>

class GroupFile final {
public:
	GroupFile(const std::string & fileName);
	GroupFile(const std::string & fileName, const uint8_t * data, size_t size);
	GroupFile(const std::string & fileName, const std::vector<uint8_t> & data);
	GroupFile(const std::string & fileName, ByteBuffer && data) noexcept;
	GroupFile(const std::string & fileName, const ByteBuffer & data);
	GroupFile(GroupFile && g) noexcept;
	GroupFile(const GroupFile & g);
	GroupFile & operator = (GroupFile && g) noexcept;
	GroupFile & operator = (const GroupFile & g);
	~GroupFile();

	const std::string & getFileName() const;
	std::string_view getFileExtension() const;
	size_t getSize() const;
	const ByteBuffer & getData() const;

	void setFileName(const std::string & fileName);
	void setData(const uint8_t * data, size_t size);
	void setData(std::vector<uint8_t> && data) noexcept;
	void setData(const std::vector<uint8_t> & data);
	void setData(const ByteBuffer & data);
	void clearData();

	std::string toString() const;

	bool isValid() const;
	static bool isValid(const GroupFile * g);

	bool writeTo(const std::string & directoryName, bool overwrite = DEFAULT_OVERWRITE_FILES, const std::string & alternateFileName = std::string()) const;

	bool operator == (const GroupFile & g) const;
	bool operator != (const GroupFile & g) const;

	static const uint8_t MAX_FILE_NAME_LENGTH;
	static const bool DEFAULT_OVERWRITE_FILES;

private:
	std::string m_fileName;
	ByteBuffer m_data;
};

#endif // _GROUP_FILE_H_
