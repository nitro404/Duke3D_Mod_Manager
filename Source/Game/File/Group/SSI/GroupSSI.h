#ifndef _GROUP_SSI_H_
#define _GROUP_SSI_H_

#include "../Group.h"

class GroupSSI final : public Group {
public:
	static constexpr uint8_t NUMBER_OF_DESCRIPTIONS = 3;

	GroupSSI(uint32_t version = DEFAULT_VERSION, const std::string & filePath = {});
	GroupSSI(std::vector<std::unique_ptr<GroupFile>> files, uint32_t version, const std::string & title = {}, const std::array<std::string, NUMBER_OF_DESCRIPTIONS> & descriptions = {}, const std::string & runFile = {}, std::unique_ptr<ByteBuffer> trailingData = nullptr, const std::string & filePath = {});
	GroupSSI(std::vector<std::unique_ptr<GroupFile>> files, const std::string & title = {}, const std::array<std::string, NUMBER_OF_DESCRIPTIONS> & descriptions = {}, std::unique_ptr<ByteBuffer> trailingData = nullptr, const std::string & filePath = {});
	GroupSSI(GroupSSI && group) noexcept;
	GroupSSI(const GroupSSI & group);
	GroupSSI & operator = (GroupSSI && group) noexcept;
	GroupSSI & operator = (const GroupSSI & group);
	virtual ~GroupSSI();

	uint32_t getVersion() const;
	bool setVersion(uint32_t version);
	bool hasTitle() const;
	const std::string & getTitle() const;
	bool setTitle(const std::string & title);
	void clearTitle();
	bool hasAnyDescription() const;
	size_t numberOfDescriptions() const;
	const std::string & getDescription(size_t index) const;
	bool setDescription(size_t index, const std::string description);
	bool clearDescription(size_t index);
	void clearAllDescriptions();
	bool versionSupportsRunFile() const;
	static bool versionSupportsRunFile(uint32_t version);
	bool hasRunFile() const;
	const std::string & getRunFile() const;
	bool setRunFile(const std::string & runFile);
	void clearRunFile();
	bool hasTrailingData() const;
	const ByteBuffer & getTrailingData() const;
	bool setTrailingData(std::vector<uint8_t> & trailingData);
	bool setTrailingData(std::unique_ptr<ByteBuffer> trailingData);
	bool setTrailingData(const ByteBuffer & trailingData);
	void clearTrailingData();

	static std::unique_ptr<GroupSSI> readFrom(const ByteBuffer & byteBuffer);
	virtual bool writeTo(ByteBuffer & byteBuffer) const override;

	static std::unique_ptr<GroupSSI> createFrom(const std::string & directoryPath);
	static std::unique_ptr<GroupSSI> loadFrom(const std::string & filePath);

	virtual void addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const override;
	virtual Endianness getEndianness() const override;
	virtual size_t getSizeInBytes() const override;

	virtual bool isValid(bool verifyParent = true) const override;
	static bool isValidVersion(uint32_t version);

	bool operator == (const GroupSSI & group) const;
	bool operator != (const GroupSSI & group) const;

	static constexpr Endianness ENDIANNESS = Endianness::LittleEndian;
	static constexpr uint32_t DEFAULT_VERSION = 2;
	static constexpr uint8_t MAX_TITLE_LENGTH = 32;
	static constexpr uint8_t MAX_RUN_FILE_LENGTH = 12;
	static constexpr uint8_t MAX_DESCRIPTION_LENGTH = 70;
	static constexpr uint8_t TRAILING_FILE_DATA_LENGTH = 34 + 1 + 69;

private:
	uint32_t m_version;
	std::string m_title;
	std::array<std::string, NUMBER_OF_DESCRIPTIONS> m_descriptions;
	std::string m_runFile;
	std::unique_ptr<ByteBuffer> m_trailingData;
};

#endif // _GROUP_SSI_H_
