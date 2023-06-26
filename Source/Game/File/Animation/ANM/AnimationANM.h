#ifndef _ANIMATION_ANM_H_
#define _ANIMATION_ANM_H_

#include "../Animation.h"
#include "../../Palette/ColourTable.h"

#include <Endianness.h>

#include <cstdint>
#include <array>
#include <memory>
#include <optional>
#include <string>

class ByteBuffer;

class AnimationANM final : public Animation {
public:
	static constexpr uint8_t NUMBER_OF_RECORD_TYPES = 32;
	static constexpr uint8_t HEADER_FILLER_SIZE_BYTES = 58;
	static constexpr uint8_t NUMBER_OF_COLOUR_CYCLES = 16;

	using RecordTypesData = std::array<uint8_t, NUMBER_OF_RECORD_TYPES>;
	using HeaderFillerData = std::array<uint8_t, HEADER_FILLER_SIZE_BYTES>;
	using ColourFlagsData = std::array<uint8_t, ColourTable::NUMBER_OF_COLOURS>;

	class ColourCycleInfo final {
	public:
		ColourCycleInfo(AnimationANM * parent = nullptr);
		ColourCycleInfo(uint16_t count, uint16_t rate, uint16_t flags, uint8_t low, uint8_t high, AnimationANM * parent = nullptr);
		ColourCycleInfo(ColourCycleInfo && colourCycleInfo) noexcept;
		ColourCycleInfo(const ColourCycleInfo & colourCycleInfo);
		ColourCycleInfo & operator = (ColourCycleInfo && colourCycleInfo) noexcept;
		ColourCycleInfo & operator = (const ColourCycleInfo & colourCycleInfo);
		~ColourCycleInfo();

		uint16_t getCount() const;
		uint16_t getRate() const;
		uint16_t getFlags() const;
		uint8_t getLow() const;
		uint8_t getHigh() const;

		static ColourCycleInfo getFrom(const ByteBuffer & byteBuffer, size_t offset, bool * error);
		static std::optional<ColourCycleInfo> getFrom(const ByteBuffer & byteBuffer, size_t offset);
		static ColourCycleInfo readFrom(const ByteBuffer & byteBuffer, bool * error);
		static std::optional<ColourCycleInfo> readFrom(const ByteBuffer & byteBuffer);
		bool putIn(ByteBuffer & byteBuffer, size_t offset) const;
		bool insertIn(ByteBuffer & byteBuffer, size_t offset) const;
		bool writeTo(ByteBuffer & byteBuffer) const;

		bool isParentValid() const;
		AnimationANM * getParent() const;
		void setParent(AnimationANM * parent);
		void clearParent();

		bool operator == (const ColourCycleInfo & colourCycleInfo) const;
		bool operator != (const ColourCycleInfo & colourCycleInfo) const;

		static constexpr uint8_t SIZE_BYTES = (sizeof(uint16_t) * 3) + (sizeof(uint8_t) * 2);

	private:
		uint16_t m_count;
		uint16_t m_rate;
		uint16_t m_flags;
		uint8_t m_low;
		uint8_t m_high;

		AnimationANM * m_parent;
	};

	using ColourCycleInfoData = std::array<ColourCycleInfo, NUMBER_OF_COLOUR_CYCLES>;

	class LargePageDescriptor final {
	public:
		LargePageDescriptor(AnimationANM * parent = nullptr);
		LargePageDescriptor(uint16_t numberOfBytes, uint16_t firstRecordNumber, uint16_t numberOfRecords, bool firstRecordContinuesFromPreviousLargePage, bool lastRecordContinuesToNextLargePage, AnimationANM * parent = nullptr);
		LargePageDescriptor(LargePageDescriptor && largePageDescriptor) noexcept;
		LargePageDescriptor(const LargePageDescriptor & largePageDescriptor);
		LargePageDescriptor & operator = (LargePageDescriptor && largePageDescriptor) noexcept;
		LargePageDescriptor & operator = (const LargePageDescriptor & largePageDescriptor);
		~LargePageDescriptor();

		uint16_t numberOfBytes() const;
		uint16_t getFirstRecordNumber() const;
		uint16_t numberOfRecords() const;
		bool doesFirstRecordContinueFromPreviousLargePage() const;
		bool doesLastRecordContinueToNextLargePage() const;

		static LargePageDescriptor getFrom(const ByteBuffer & byteBuffer, size_t offset, bool * error);
		static std::optional<LargePageDescriptor> getFrom(const ByteBuffer & byteBuffer, size_t offset);
		static LargePageDescriptor readFrom(const ByteBuffer & byteBuffer, bool * error);
		static std::optional<LargePageDescriptor> readFrom(const ByteBuffer & byteBuffer);
		bool putIn(ByteBuffer & byteBuffer, size_t offset) const;
		bool insertIn(ByteBuffer & byteBuffer, size_t offset) const;
		bool writeTo(ByteBuffer & byteBuffer) const;

		bool isValid() const;
		bool isParentValid() const;
		AnimationANM * getParent() const;
		void setParent(AnimationANM * parent);
		void clearParent();

		bool operator == (const LargePageDescriptor & largePageDescriptor) const;
		bool operator != (const LargePageDescriptor & largePageDescriptor) const;

		static constexpr uint8_t SIZE_BYTES = sizeof(uint16_t) * 3;

	private:
		uint16_t m_firstRecordNumber;
		uint16_t m_numberOfRecords;
		bool m_lastRecordContinuesToNextLargePage;
		bool m_firstRecordContinuesFromPreviousLargePage;
		uint16_t m_numberOfBytes;

		AnimationANM * m_parent;
	};

	using LargePageDescriptorData = std::vector<LargePageDescriptor>;

	AnimationANM(const std::string & filePath = {});
	AnimationANM(uint16_t maximumNumberOfLargePages, uint32_t numberOfRecords, uint16_t numberOfRecordsPermittedInLargePage, uint16_t largePageFileTableOffset, uint16_t frameWidth, uint16_t frameHeight, uint8_t variant, uint8_t version, bool hasLastDelta, bool lastDeltaValid, uint8_t pixelType, uint8_t compressionType, uint8_t otherRecordsPerFrame, uint8_t bitmapType, std::unique_ptr<RecordTypesData> recordTypes, uint32_t numberOfFrames, uint16_t numberOfFramesPerSecond, std::unique_ptr<HeaderFillerData> headerFillerData, std::unique_ptr<ColourCycleInfoData> colourCycles, std::unique_ptr<ColourTable> colourTable, std::unique_ptr<ColourFlagsData> colourFlags, std::unique_ptr<LargePageDescriptorData> largePageDescriptors, std::unique_ptr<ByteBuffer> largePageData);
	AnimationANM(AnimationANM && animation) noexcept;
	AnimationANM(const AnimationANM & animation);
	AnimationANM & operator = (AnimationANM && animation) noexcept;
	AnimationANM & operator = (const AnimationANM & animation);
	virtual ~AnimationANM();

	uint16_t maximumNumberOfLargePages() const;
	uint32_t numberOfRecords() const;
	uint32_t numberOfRecordsPermittedInLargePage() const;
	uint16_t getLargePageFileTableOffset() const;
	virtual uint16_t getFrameWidth() const override;
	virtual uint16_t getFrameHeight() const override;
	uint8_t getVariant() const;
	uint8_t getVersion() const;
	bool hasLastDelta() const;
	bool isLastDeltaValid() const;
	uint8_t getPixelType() const;
	uint8_t getCompressionType() const;
	uint8_t getOtherRecordsPerFrame() const;
	uint8_t getBitmapType() const;
	const RecordTypesData & getRecordTypes();
	virtual uint32_t numberOfFrames() const override;
	uint16_t numberOfFramesPerSecond() const;
	virtual std::chrono::milliseconds getDuration() const override;
	const HeaderFillerData & getHeaderFillerData() const;
	virtual std::shared_ptr<ColourTable> getColourTable() const override;
	const ColourFlagsData & getColourFlags() const;
	const std::vector<LargePageDescriptor> & getLargePageDescriptors() const;
	const ByteBuffer & getLargePageData() const;

	static std::unique_ptr<AnimationANM> readFrom(const ByteBuffer & byteBuffer);
	static std::unique_ptr<AnimationANM> loadFrom(const std::string & filePath);
	virtual bool writeTo(ByteBuffer & byteBuffer) const override;
	virtual Endianness getEndianness() const override;
	virtual void addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const override;
	virtual size_t getSizeInBytes() const override;

	virtual bool isValid(bool verifyParent = true) const override;
	static bool isValidVariant(uint8_t variant);
	static bool isValidVersion(uint8_t version);
	static bool isValidPixelType(uint8_t pixelType);
	static bool isValidCompressionType(uint8_t compressionType);
	static bool isValidBitmapType(uint8_t bitmapType);

	bool operator == (const AnimationANM & animation) const;
	bool operator != (const AnimationANM & animation) const;

	static constexpr Endianness ENDIANNESS = Endianness::LittleEndian;
	static inline const std::string IDENTIFIER = "LPF "; // large page file
	static inline const std::string CONTENT_TYPE = "ANIM"; // animation
	static constexpr uint16_t NUMBER_OF_LARGE_PAGE_DESCRIPTORS = 256;

	static constexpr uint8_t COLOUR_CYCLE_INFO_SIZE_BYTES = ColourCycleInfo::SIZE_BYTES;
	static constexpr uint8_t LARGE_PAGE_DESCRIPTOR_SIZE_BYTES = LargePageDescriptor::SIZE_BYTES;
	static constexpr uint32_t LARGE_PAGE_SIZE_BYTES = std::numeric_limits<uint16_t>::max() + 1;

	static constexpr uint16_t DEFAULT_MAXIMUM_NUMBER_OF_LARGE_PAGES = 256;
	static constexpr uint16_t DEFAULT_NUMBER_OF_RECORDS_PERMITTED_IN_LARGE_PAGE = 256;
	static constexpr uint16_t DEFAULT_LARGE_PAGE_FILE_TABLE_OFFSET = 1280;
	static constexpr uint16_t DEFAULT_FRAME_WIDTH = 320;
	static constexpr uint16_t DEFAULT_FRAME_HEIGHT = 200;
	static constexpr uint8_t DEFAULT_VARIANT = 0;
	static constexpr uint8_t DEFAULT_VERSION = 0;
	static constexpr bool DEFAULT_HAS_LAST_DELTA = true;
	static constexpr bool DEFAULT_LAST_DELTA_VALID = true;
	static constexpr uint8_t DEFAULT_PIXEL_TYPE = 0;
	static constexpr uint8_t DEFAULT_COMPRESSION_TYPE = 1;
	static constexpr uint8_t DEFAULT_BITMAP_TYPE = 1;
	static constexpr uint16_t DEFAULT_NUMBER_OF_FRAMES_PER_SECOND = 1;

private:
	void updateParent();

	uint16_t m_maximumNumberOfLargePages;
	uint32_t m_numberOfRecords;
	uint16_t m_numberOfRecordsPermittedInLargePage;
	uint16_t m_largePageFileTableOffset;
	uint16_t m_frameWidth;
	uint16_t m_frameHeight;
	uint8_t m_variant;
	uint8_t m_version;
	bool m_hasLastDelta;
	bool m_lastDeltaValid;
	uint8_t m_pixelType;
	uint8_t m_compressionType;
	uint8_t m_otherRecordsPerFrame;
	uint8_t m_bitmapType;
	std::unique_ptr<RecordTypesData> m_recordTypes;
	uint32_t m_numberOfFrames;
	uint16_t m_numberOfFramesPerSecond;
	std::unique_ptr<HeaderFillerData> m_headerFillerData;
	std::unique_ptr<ColourCycleInfoData> m_colourCycles;
	std::shared_ptr<ColourTable> m_colourTable;
	std::unique_ptr<ColourFlagsData> m_colourFlags;
	std::unique_ptr<LargePageDescriptorData> m_largePageDescriptors;
	std::unique_ptr<ByteBuffer> m_largePageData;
};

#endif // _ANIMATION_ANM_H_
