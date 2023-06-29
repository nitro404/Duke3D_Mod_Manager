#include "AnimationANM.h"

#include <ByteBuffer.h>
#include <Endianness.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

AnimationANM::AnimationANM(const std::string & filePath)
	: Animation(filePath)
	, m_maximumNumberOfLargePages(DEFAULT_MAXIMUM_NUMBER_OF_LARGE_PAGES)
	, m_numberOfRecords(0)
	, m_numberOfRecordsPermittedInLargePage(DEFAULT_NUMBER_OF_RECORDS_PERMITTED_IN_LARGE_PAGE)
	, m_largePageFileTableOffset(DEFAULT_LARGE_PAGE_FILE_TABLE_OFFSET)
	, m_frameWidth(DEFAULT_FRAME_WIDTH)
	, m_frameHeight(DEFAULT_FRAME_HEIGHT)
	, m_variant(DEFAULT_VARIANT)
	, m_version(DEFAULT_VERSION)
	, m_hasLastDelta(DEFAULT_HAS_LAST_DELTA)
	, m_lastDeltaValid(DEFAULT_LAST_DELTA_VALID)
	, m_pixelType(DEFAULT_PIXEL_TYPE)
	, m_compressionType(DEFAULT_COMPRESSION_TYPE)
	, m_otherRecordsPerFrame(0)
	, m_bitmapType(DEFAULT_BITMAP_TYPE)
	, m_recordTypes(std::make_unique<RecordTypesData>())
	, m_numberOfFrames(0)
	, m_numberOfFramesPerSecond(DEFAULT_NUMBER_OF_FRAMES_PER_SECOND)
	, m_headerFillerData(std::make_unique<HeaderFillerData>())
	, m_colourCycles(std::make_unique<ColourCycleInfoData>())
	, m_colourTable(std::make_shared<ColourTable>(Colour::INVISIBLE))
	, m_colourFlags(std::make_unique<ColourFlagsData>())
	, m_largePageData(std::make_unique<ByteBuffer>()) {
	updateParent();
}

AnimationANM::AnimationANM(uint16_t maximumNumberOfLargePages, uint32_t numberOfRecords, uint16_t numberOfRecordsPermittedInLargePage, uint16_t largePageFileTableOffset, uint16_t frameWidth, uint16_t frameHeight, uint8_t variant, uint8_t version, bool hasLastDelta, bool lastDeltaValid, uint8_t pixelType, uint8_t compressionType, uint8_t otherRecordsPerFrame, uint8_t bitmapType, std::unique_ptr<RecordTypesData> recordTypes, uint32_t numberOfFrames, uint16_t numberOfFramesPerSecond, std::unique_ptr<HeaderFillerData> headerFillerData, std::unique_ptr<ColourCycleInfoData> colourCycles, std::unique_ptr<ColourTable> colourTable, std::unique_ptr<ColourFlagsData> colourFlags, std::unique_ptr<LargePageDescriptorData> largePageDescriptors, std::unique_ptr<ByteBuffer> largePageData)
	: Animation()
	, m_maximumNumberOfLargePages(maximumNumberOfLargePages)
	, m_numberOfRecords(numberOfRecords)
	, m_numberOfRecordsPermittedInLargePage(numberOfRecordsPermittedInLargePage)
	, m_largePageFileTableOffset(largePageFileTableOffset)
	, m_frameWidth(frameWidth)
	, m_frameHeight(frameHeight)
	, m_variant(variant)
	, m_version(version)
	, m_hasLastDelta(hasLastDelta)
	, m_lastDeltaValid(lastDeltaValid)
	, m_pixelType(pixelType)
	, m_compressionType(compressionType)
	, m_otherRecordsPerFrame(otherRecordsPerFrame)
	, m_bitmapType(bitmapType)
	, m_recordTypes(recordTypes != nullptr ? std::move(recordTypes) : std::make_unique<RecordTypesData>())
	, m_numberOfFrames(numberOfFrames)
	, m_numberOfFramesPerSecond(numberOfFramesPerSecond)
	, m_headerFillerData(std::move(headerFillerData))
	, m_colourCycles(colourCycles != nullptr ? std::move(colourCycles) : std::make_unique<ColourCycleInfoData>())
	, m_colourTable(colourTable != nullptr ? std::shared_ptr<ColourTable>(colourTable.release()) : std::make_shared<ColourTable>(Colour::INVISIBLE))
	, m_colourFlags(colourFlags != nullptr ? std::move(colourFlags) : std::make_unique<ColourFlagsData>())
	, m_largePageDescriptors(largePageDescriptors != nullptr ? std::move(largePageDescriptors) : std::make_unique<LargePageDescriptorData>())
	, m_largePageData(largePageData != nullptr ? std::move(largePageData) : std::make_unique<ByteBuffer>()) {
	updateParent();
}

AnimationANM::AnimationANM(AnimationANM && animation) noexcept
	: Animation(std::move(animation))
	, m_maximumNumberOfLargePages(animation.m_maximumNumberOfLargePages)
	, m_numberOfRecords(animation.m_numberOfRecords)
	, m_numberOfRecordsPermittedInLargePage(animation.m_numberOfRecordsPermittedInLargePage)
	, m_largePageFileTableOffset(animation.m_largePageFileTableOffset)
	, m_frameWidth(animation.m_frameWidth)
	, m_frameHeight(animation.m_frameHeight)
	, m_variant(animation.m_variant)
	, m_version(animation.m_version)
	, m_hasLastDelta(animation.m_hasLastDelta)
	, m_lastDeltaValid(animation.m_lastDeltaValid)
	, m_pixelType(animation.m_pixelType)
	, m_compressionType(animation.m_compressionType)
	, m_otherRecordsPerFrame(animation.m_otherRecordsPerFrame)
	, m_bitmapType(animation.m_bitmapType)
	, m_recordTypes(std::move(animation.m_recordTypes))
	, m_numberOfFrames(animation.m_numberOfFrames)
	, m_numberOfFramesPerSecond(animation.m_numberOfFramesPerSecond)
	, m_headerFillerData(std::move(animation.m_headerFillerData))
	, m_colourCycles(std::move(animation.m_colourCycles))
	, m_colourTable(std::move(animation.m_colourTable))
	, m_colourFlags(std::move(animation.m_colourFlags))
	, m_largePageDescriptors(std::move(animation.m_largePageDescriptors))
	, m_largePageData(std::move(animation.m_largePageData)) {
	updateParent();
}

AnimationANM::AnimationANM(const AnimationANM & animation)
	: Animation(animation)
	, m_maximumNumberOfLargePages(animation.m_maximumNumberOfLargePages)
	, m_numberOfRecords(animation.m_numberOfRecords)
	, m_numberOfRecordsPermittedInLargePage(animation.m_numberOfRecordsPermittedInLargePage)
	, m_largePageFileTableOffset(animation.m_largePageFileTableOffset)
	, m_frameWidth(animation.m_frameWidth)
	, m_frameHeight(animation.m_frameHeight)
	, m_variant(animation.m_variant)
	, m_version(animation.m_version)
	, m_hasLastDelta(animation.m_hasLastDelta)
	, m_lastDeltaValid(animation.m_lastDeltaValid)
	, m_pixelType(animation.m_pixelType)
	, m_compressionType(animation.m_compressionType)
	, m_otherRecordsPerFrame(animation.m_otherRecordsPerFrame)
	, m_bitmapType(animation.m_bitmapType)
	, m_recordTypes(std::make_unique<RecordTypesData>(*animation.m_recordTypes))
	, m_numberOfFrames(animation.m_numberOfFrames)
	, m_numberOfFramesPerSecond(animation.m_numberOfFramesPerSecond)
	, m_headerFillerData(std::make_unique<HeaderFillerData>(*animation.m_headerFillerData))
	, m_colourCycles(std::make_unique<ColourCycleInfoData>(*animation.m_colourCycles))
	, m_colourTable(std::make_shared<ColourTable>(*animation.m_colourTable))
	, m_colourFlags(std::make_unique<ColourFlagsData>(*animation.m_colourFlags))
	, m_largePageDescriptors(std::make_unique<LargePageDescriptorData>(*animation.m_largePageDescriptors))
	, m_largePageData(std::make_unique<ByteBuffer>(*animation.m_largePageData)) {
	updateParent();
}

AnimationANM & AnimationANM::operator = (AnimationANM && animation) noexcept {
	if(this != &animation) {
		Animation::operator = (std::move(animation));

		m_maximumNumberOfLargePages = animation.m_maximumNumberOfLargePages;
		m_numberOfRecords = animation.m_numberOfRecords;
		m_numberOfRecordsPermittedInLargePage = animation.m_numberOfRecordsPermittedInLargePage;
		m_largePageFileTableOffset = animation.m_largePageFileTableOffset;
		m_frameWidth = animation.m_frameWidth;
		m_frameHeight = animation.m_frameHeight;
		m_variant = animation.m_variant;
		m_version = animation.m_version;
		m_hasLastDelta = animation.m_hasLastDelta;
		m_lastDeltaValid = animation.m_lastDeltaValid;
		m_pixelType = animation.m_pixelType;
		m_compressionType = animation.m_compressionType;
		m_otherRecordsPerFrame = animation.m_otherRecordsPerFrame;
		m_bitmapType = animation.m_bitmapType;
		m_recordTypes = std::move(animation.m_recordTypes);
		m_numberOfFrames = animation.m_numberOfFrames;
		m_numberOfFramesPerSecond = animation.m_numberOfFramesPerSecond;
		m_headerFillerData = std::move(animation.m_headerFillerData);
		m_colourCycles = std::move(animation.m_colourCycles);
		m_colourTable = std::move(animation.m_colourTable);
		m_colourFlags = std::move(animation.m_colourFlags);
		m_largePageDescriptors = std::move(animation.m_largePageDescriptors);
		m_largePageData = std::move(animation.m_largePageData);

		updateParent();
	}

	return *this;
}

AnimationANM & AnimationANM::operator = (const AnimationANM & animation) {
	Animation::operator = (animation);

	m_maximumNumberOfLargePages = animation.m_maximumNumberOfLargePages;
	m_numberOfRecords = animation.m_numberOfRecords;
	m_numberOfRecordsPermittedInLargePage = animation.m_numberOfRecordsPermittedInLargePage;
	m_largePageFileTableOffset = animation.m_largePageFileTableOffset;
	m_frameWidth = animation.m_frameWidth;
	m_frameHeight = animation.m_frameHeight;
	m_variant = animation.m_variant;
	m_version = animation.m_version;
	m_hasLastDelta = animation.m_hasLastDelta;
	m_lastDeltaValid = animation.m_lastDeltaValid;
	m_pixelType = animation.m_pixelType;
	m_compressionType = animation.m_compressionType;
	m_otherRecordsPerFrame = animation.m_otherRecordsPerFrame;
	m_bitmapType = animation.m_bitmapType;
	m_recordTypes = std::make_unique<RecordTypesData>(*animation.m_recordTypes);
	m_numberOfFrames = animation.m_numberOfFrames;
	m_numberOfFramesPerSecond = animation.m_numberOfFramesPerSecond;
	m_headerFillerData = std::make_unique<HeaderFillerData>(*animation.m_headerFillerData);
	m_colourCycles = std::make_unique<ColourCycleInfoData>(*animation.m_colourCycles);
	m_colourTable = std::make_shared<ColourTable>(*animation.m_colourTable);
	m_colourFlags = std::make_unique<ColourFlagsData>(*animation.m_colourFlags);
	m_largePageDescriptors = std::make_unique<LargePageDescriptorData>(*animation.m_largePageDescriptors);
	m_largePageData = std::make_unique<ByteBuffer>(*animation.m_largePageData);

	updateParent();

	return *this;
}

AnimationANM::~AnimationANM() { }

uint16_t AnimationANM::maximumNumberOfLargePages() const {
	return m_maximumNumberOfLargePages;
}

uint32_t AnimationANM::numberOfRecords() const {
	return m_numberOfRecords;
}

uint32_t AnimationANM::numberOfRecordsPermittedInLargePage() const {
	return m_numberOfRecordsPermittedInLargePage;
}

uint16_t AnimationANM::getLargePageFileTableOffset() const {
	return m_largePageFileTableOffset;
}

uint16_t AnimationANM::getFrameWidth() const {
	return m_frameWidth;
}

uint16_t AnimationANM::getFrameHeight() const {
	return m_frameHeight;
}

uint8_t AnimationANM::getVariant() const {
	return m_variant;
}

uint8_t AnimationANM::getVersion() const {
	return m_version;
}

bool AnimationANM::hasLastDelta() const {
	return m_hasLastDelta;
}

bool AnimationANM::isLastDeltaValid() const {
	return m_lastDeltaValid;
}

uint8_t AnimationANM::getPixelType() const {
	return m_pixelType;
}

uint8_t AnimationANM::getCompressionType() const {
	return m_compressionType;
}

uint8_t AnimationANM::getOtherRecordsPerFrame() const {
	return m_otherRecordsPerFrame;
}

uint8_t AnimationANM::getBitmapType() const {
	return m_bitmapType;
}

const AnimationANM::RecordTypesData & AnimationANM::getRecordTypes() {
	return *m_recordTypes;
}

uint32_t AnimationANM::numberOfFrames() const {
	return m_numberOfFrames;
}

uint16_t AnimationANM::numberOfFramesPerSecond() const {
	return m_numberOfFramesPerSecond;
}

std::chrono::milliseconds AnimationANM::getDuration() const {
	return std::chrono::milliseconds(static_cast<int64_t>((static_cast<double>(m_numberOfFrames) / static_cast<double>(m_numberOfFramesPerSecond)) * 1000.0));
}

const AnimationANM::HeaderFillerData & AnimationANM::getHeaderFillerData() const {
	return *m_headerFillerData;
}

std::shared_ptr<ColourTable> AnimationANM::getColourTable() const {
	return m_colourTable;
}

const AnimationANM::ColourFlagsData & AnimationANM::getColourFlags() const {
	return *m_colourFlags;
}

const AnimationANM::LargePageDescriptorData & AnimationANM::getLargePageDescriptors() const {
	return *m_largePageDescriptors;
}

const ByteBuffer & AnimationANM::getLargePageData() const {
	return *m_largePageData;
}

std::unique_ptr<AnimationANM> AnimationANM::readFrom(const ByteBuffer & byteBuffer) {
	// https://wiki.multimedia.cx/index.php?title=DeluxePaint_Animation

	byteBuffer.setEndianness(ENDIANNESS);

	bool error = false;

	std::string identifier(Utilities::fromIntegerString(byteBuffer.readUnsignedInteger(&error), Endianness::LittleEndian));

	if(error) {
		spdlog::error("Electronic Arts Deluxe Paint ANM animation data is truncated, missing '{}' identifier.", IDENTIFIER);
		return nullptr;
	}

	if(!Utilities::areStringsEqual(identifier, IDENTIFIER)) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation identifier: '{}', expected: '{}'.", identifier, IDENTIFIER);
		return nullptr;
	}

	uint16_t maximumLargePageCount = byteBuffer.readUnsignedShort(&error);

	if(error) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing maximum number of large pages value.");
		return nullptr;
	}

	uint16_t largePageCount = byteBuffer.readUnsignedShort(&error);

	if(error) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing number of large pages value.");
		return nullptr;
	}

	uint32_t recordCount = byteBuffer.readUnsignedInteger(&error);

	if(error) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing number of records value.");
		return nullptr;
	}

	uint16_t largePagePermittedRecordsCount = byteBuffer.readUnsignedShort(&error);

	if(error) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing number of records permitted in large page value.");
		return nullptr;
	}

	uint16_t largePageFileTableOffset = byteBuffer.readUnsignedShort(&error);

	if(error) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing large page file table offset value.");
		return nullptr;
	}

	std::string contentType(Utilities::fromIntegerString(byteBuffer.readUnsignedInteger(&error), Endianness::LittleEndian));

	if(error) {
		spdlog::error("Electronic Arts Deluxe Paint ANM animation data is truncated, missing '{}' content type.", CONTENT_TYPE);
		return nullptr;
	}

	if(!Utilities::areStringsEqual(contentType, CONTENT_TYPE)) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation content type: '{}', expected: '{}'.", contentType, CONTENT_TYPE);
		return nullptr;
	}

	uint16_t frameWidth = byteBuffer.readUnsignedShort(&error);

	if(error) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing frame width value.");
		return nullptr;
	}

	uint16_t frameHeight = byteBuffer.readUnsignedShort(&error);

	if(error) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing frame height value.");
		return nullptr;
	}

	uint8_t variant = byteBuffer.readUnsignedByte(&error);

	if(error) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing variant value.");
		return nullptr;
	}

	if(!isValidVariant(variant)) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation variant value: {}.", variant);
		return nullptr;
	}

	uint8_t version = byteBuffer.readUnsignedByte(&error);

	if(error) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing version value.");
		return nullptr;
	}

	if(!isValidVersion(version)) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation version value: {}.", version);
		return nullptr;
	}

	bool lastDelta = byteBuffer.readUnsignedByte(&error) != 0;

	if(error) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing has last delta value.");
		return nullptr;
	}

	bool lastDeltaValid = byteBuffer.readUnsignedByte(&error) != 0;

	if(error) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing last delta valid value.");
		return nullptr;
	}

	uint8_t pixelType = byteBuffer.readUnsignedByte(&error);

	if(error) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing pixel type value.");
		return nullptr;
	}

	if(!isValidPixelType(pixelType)) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation pixel type value: {}.", pixelType);
		return nullptr;
	}

	uint8_t compressionType = byteBuffer.readUnsignedByte(&error);

	if(error) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing compression type value.");
		return nullptr;
	}

	if(!isValidCompressionType(compressionType)) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation compression type value: {}.", compressionType);
		return nullptr;
	}

	uint8_t otherRecordsPerFrame = byteBuffer.readUnsignedByte(&error);

	if(error) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing other records per frame value.");
		return nullptr;
	}

	if(otherRecordsPerFrame != 0) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation other records per frame value: {}.", otherRecordsPerFrame);
		return nullptr;
	}

	uint8_t bitmapType = byteBuffer.readUnsignedByte(&error);

	if(error) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing bitmap type value.");
		return nullptr;
	}

	if(!isValidBitmapType(bitmapType)) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation bitmap type value: {}.", bitmapType);
		return nullptr;
	}

	std::unique_ptr<RecordTypesData> recordTypes(byteBuffer.readBytes<NUMBER_OF_RECORD_TYPES>());

	if(recordTypes == nullptr) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing record types data.");
		return nullptr;
	}

	uint32_t frameCount = byteBuffer.readUnsignedInteger(&error);

	if(error) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing frame count value.");
		return nullptr;
	}

	uint16_t framesPerSecond = byteBuffer.readUnsignedShort(&error);

	if(error) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing frames per second value.");
		return nullptr;
	}

	if(framesPerSecond == 0) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation frames per second value: {}.", framesPerSecond);
		return nullptr;
	}

	std::unique_ptr<HeaderFillerData> headerFillerData(byteBuffer.readBytes<HEADER_FILLER_SIZE_BYTES>());

	if(headerFillerData == nullptr) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing header filler data.");
		return nullptr;
	}

	ColourCycleInfo colourCycleInfo;
	std::unique_ptr<ColourCycleInfoData> colourCycles(std::make_unique<ColourCycleInfoData>());

	for(uint8_t i = 0; i < NUMBER_OF_COLOUR_CYCLES; i++) {
		colourCycleInfo = ColourCycleInfo::readFrom(byteBuffer, &error);

		if(error) {
			spdlog::error("Failed to read Electronic Arts Deluxe Paint ANM animation colour cycle info #{}.", i + 1);
			return nullptr;
		}

		(*colourCycles)[i] = colourCycleInfo;
	}

	std::vector<Colour> colours;
	colours.reserve(ColourTable::NUMBER_OF_COLOURS);
	std::unique_ptr<ColourFlagsData> colourFlags(std::make_unique<ColourFlagsData>());

	for(uint16_t i = 0; i < ColourTable::NUMBER_OF_COLOURS; i++) {
		colours.emplace_back(Colour::readFrom(byteBuffer, false, Colour::ByteOrder::BGRA, &error));

		if(error) {
			spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing colour table colour #{}.", i + 1);
			return nullptr;
		}

		(*colourFlags)[i] = byteBuffer.readUnsignedByte(&error);

		if(error) {
			spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing colour flag #{}.", i + 1);
			return nullptr;
		}
	}

	LargePageDescriptor largePageDescriptor;
	std::unique_ptr<LargePageDescriptorData> largePageDescriptors(std::make_unique<LargePageDescriptorData>());

	for(uint16_t i = 0; i < largePageCount; i++) {
		largePageDescriptor = LargePageDescriptor::readFrom(byteBuffer, &error);

		if(error) {
			spdlog::error("Failed to read Electronic Arts Deluxe Paint ANM animation large page descriptor #{}.", i + 1);
			return nullptr;
		}

		largePageDescriptors->emplace_back(std::move(largePageDescriptor));
	}

	// regardless of the number of large pages value, there are always 256 large page entries, so skip over the empty ones
	if(!byteBuffer.skipReadBytes(static_cast<size_t>(LargePageDescriptor::SIZE_BYTES) * (NUMBER_OF_LARGE_PAGE_DESCRIPTORS - largePageCount))) {
		spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, missing large page descriptor filler data.");
		return nullptr;
	}

	std::unique_ptr<ByteBuffer> largePageData(byteBuffer.getRemainingBytes());

	size_t dataOffset = byteBuffer.getReadOffset();
	uint16_t frameLargePageIndex = 0;

	for(uint16_t frameIndex = 0; frameIndex < frameCount; frameIndex++) {
		frameLargePageIndex = std::numeric_limits<uint16_t>::max();

		for(uint16_t largePageIndex = 0; largePageIndex < largePageDescriptors->size(); largePageIndex++) {
			uint16_t currentLargePageFirstRecordNumber = (*largePageDescriptors)[largePageIndex].getFirstRecordNumber();
			uint16_t currentLargePageRecordCount = (*largePageDescriptors)[largePageIndex].numberOfRecords();

			if(currentLargePageFirstRecordNumber <= frameIndex && frameIndex < currentLargePageFirstRecordNumber + currentLargePageRecordCount) {
				frameLargePageIndex = largePageIndex;
				break;
			}
		}

		if(frameLargePageIndex == std::numeric_limits<uint16_t>::max()) {
			spdlog::error("Invalid Electronic Arts Deluxe Paint ANM animation data, frame #{} is not contained within any available large page descriptors.");
			return nullptr;
		}
	}

	return std::make_unique<AnimationANM>(
		maximumLargePageCount,
		recordCount,
		largePagePermittedRecordsCount,
		largePageFileTableOffset,
		frameWidth,
		frameHeight,
		variant,
		version,
		lastDelta,
		lastDeltaValid,
		pixelType,
		compressionType,
		otherRecordsPerFrame,
		bitmapType,
		std::move(recordTypes),
		frameCount,
		framesPerSecond,
		std::move(headerFillerData),
		std::move(colourCycles),
		std::make_unique<ColourTable>(std::move(colours)),
		std::move(colourFlags),
		std::move(largePageDescriptors),
		std::move(largePageData)
	);
}

std::unique_ptr<AnimationANM> AnimationANM::loadFrom(const std::string & filePath) {
	std::unique_ptr<ByteBuffer> animationData(ByteBuffer::readFrom(filePath));

	if(animationData == nullptr) {
		spdlog::error("Failed to read Electronic Arts Deluxe Paint ANM animation binary data from file: '{}'.", filePath);
		return nullptr;
	}

	std::unique_ptr<AnimationANM> animation(AnimationANM::readFrom(*animationData));

	if(animation == nullptr) {
		spdlog::error("Failed to parse Electronic Arts Deluxe Paint ANM animation binary data from file contents: '{}'.", filePath);
		return nullptr;
	}

	return animation;
}

bool AnimationANM::writeTo(ByteBuffer & byteBuffer) const {
	byteBuffer.setEndianness(ENDIANNESS);

	if(!byteBuffer.writeUnsignedInteger(Utilities::toIntegerString(IDENTIFIER, getEndianness()))) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(m_maximumNumberOfLargePages)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(m_largePageDescriptors->size())) {
		return false;
	}

	if(!byteBuffer.writeUnsignedInteger(m_numberOfRecords)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(m_numberOfRecordsPermittedInLargePage)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(m_largePageFileTableOffset)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedInteger(Utilities::toIntegerString(CONTENT_TYPE, getEndianness()))) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(m_frameWidth)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(m_frameHeight)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedByte(m_variant)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedByte(m_version)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedByte(m_hasLastDelta ? 1 : 0)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedByte(m_lastDeltaValid ? 1 : 0)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedByte(m_pixelType)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedByte(m_compressionType)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedByte(m_otherRecordsPerFrame)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedByte(m_bitmapType)) {
		return false;
	}

	if(!byteBuffer.writeBytes(*m_recordTypes)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedInteger(m_numberOfFrames)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(m_numberOfFramesPerSecond)) {
		return false;
	}

	if(!byteBuffer.writeBytes(*m_headerFillerData)) {
		return false;
	}

	for(const ColourCycleInfo & colourCycleInfo : *m_colourCycles) {
		if(!colourCycleInfo.writeTo(byteBuffer)) {
			return false;
		}
	}

	bool error = false;

	for(uint16_t i = 0; i < ColourTable::NUMBER_OF_COLOURS; i++) {
		const Colour & colour = m_colourTable->getColour(i, &error);

		if(error || !colour.writeTo(byteBuffer, false, Colour::ByteOrder::BGRA)) {
			return false;
		}

		if(!byteBuffer.writeByte((*m_colourFlags)[i])) {
			return false;
		}
	}

	for(const LargePageDescriptor & largePageDescriptor : *m_largePageDescriptors) {
		if(!largePageDescriptor.writeTo(byteBuffer)) {
			return false;
		}
	}

	// regardless of the number of large pages value, there are always 256 large page entries, so fill the empty space
	if(!byteBuffer.skipWriteBytes(static_cast<size_t>(LargePageDescriptor::SIZE_BYTES) * (NUMBER_OF_LARGE_PAGE_DESCRIPTORS - m_largePageDescriptors->size()))) {
		return false;
	}

	if(!byteBuffer.writeBytes(*m_largePageData)) {
		return false;
	}

	return true;
}

Endianness AnimationANM::getEndianness() const {
	return ENDIANNESS;
}

void AnimationANM::addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const {
	Animation::addMetadata(metadata);

	metadata.push_back({ "Version", std::to_string(m_version) });
	metadata.push_back({ "Variant", std::to_string(m_variant) });
	metadata.push_back({ "Pixel Type", std::to_string(m_pixelType) });
	metadata.push_back({ "Compression Type", std::to_string(m_compressionType) });
	metadata.push_back({ "Number of Frames", std::to_string(m_numberOfFramesPerSecond) });
	metadata.push_back({ "Large Page File Table Offset", std::to_string(m_largePageFileTableOffset) });
	metadata.push_back({ "Number of Large Pages", std::to_string(m_largePageDescriptors->size()) });
	metadata.push_back({ "Maximum Number of Large Pages", std::to_string(m_maximumNumberOfLargePages) });
	metadata.push_back({ "Number of Records", std::to_string(m_numberOfRecords) });
	metadata.push_back({ "Number of Records Permitted in Large Page", std::to_string(m_numberOfRecordsPermittedInLargePage) });
	metadata.push_back({ "Other Records Per Frame", std::to_string(m_otherRecordsPerFrame) });
	metadata.push_back({ "Has Last Delta", m_hasLastDelta ? "Y" : "N" });
	metadata.push_back({ "Is Last Delta Valid", m_lastDeltaValid ? "Y" : "N" });
	metadata.push_back({ "Number of Colour Cycles", std::to_string(m_colourCycles->size()) });
}

size_t AnimationANM::getSizeInBytes() const {
	return (sizeof(uint32_t) * 4) + (sizeof(uint16_t) * 7) + (sizeof(uint8_t) * 8) + (sizeof(uint8_t) * NUMBER_OF_RECORD_TYPES) + HEADER_FILLER_SIZE_BYTES + (sizeof(uint8_t) * ColourTable::NUMBER_OF_COLOURS * 4) + (ColourCycleInfo::SIZE_BYTES * NUMBER_OF_COLOUR_CYCLES) + (LargePageDescriptor::SIZE_BYTES * NUMBER_OF_LARGE_PAGE_DESCRIPTORS) + m_largePageData->getSize();
}

bool AnimationANM::isValid(bool verifyParent) const {
	if(!Animation::isValid(verifyParent) ||
	   m_numberOfFramesPerSecond == 0 ||
	   m_frameWidth == 0 ||
	   m_frameHeight == 0 ||
	   !isValidVariant(m_variant) ||
	   !isValidVersion(m_version) ||
	   !isValidPixelType(m_pixelType) ||
	   !isValidCompressionType(m_compressionType) ||
	   !isValidBitmapType(m_bitmapType) ||
	   m_recordTypes == nullptr ||
	   m_headerFillerData == nullptr ||
	   m_colourFlags == nullptr ||
	   m_largePageData == nullptr ||
	   m_colourTable == nullptr ||
	   !m_colourTable->isValid() ||
	   m_colourTable->numberOfColours() != ColourTable::NUMBER_OF_COLOURS ||
	   (!m_largePageDescriptors->empty() && m_numberOfRecords == 0 || m_largePageDescriptors->empty() && m_numberOfRecords != 0)) {
		return false;
	}

	for(const LargePageDescriptor & largePageDescriptor : *m_largePageDescriptors) {
		if(!largePageDescriptor.isValid()) {
			return false;
		}
	}

	if(verifyParent) {
		if(m_colourTable->getParent() != this) {
			return false;
		}

		for(const ColourCycleInfo & colourCycleInfo : *m_colourCycles) {
			if(colourCycleInfo.getParent() != this) {
				return false;
			}
		}

		for(const LargePageDescriptor & largePageDescriptor : *m_largePageDescriptors) {
			if(largePageDescriptor.getParent() != this) {
				return false;
			}
		}
	}

	return true;
}

bool AnimationANM::isValidVariant(uint8_t variant) {
	return variant == 0;
}

bool AnimationANM::isValidVersion(uint8_t version) {
	return version >= 0 &&
		   version <= 1;
}

bool AnimationANM::isValidPixelType(uint8_t pixelType) {
	return pixelType == 0;
}

bool AnimationANM::isValidCompressionType(uint8_t compressionType) {
	return compressionType == 1;
}

bool AnimationANM::isValidBitmapType(uint8_t bitmapType) {
	return bitmapType == 1;
}

bool AnimationANM::operator == (const AnimationANM & animation) const {
	return m_maximumNumberOfLargePages == animation.m_maximumNumberOfLargePages &&
		   m_numberOfRecords == animation.m_numberOfRecords &&
		   m_numberOfRecordsPermittedInLargePage == animation.m_numberOfRecordsPermittedInLargePage &&
		   m_largePageFileTableOffset == animation.m_largePageFileTableOffset &&
		   m_frameWidth == animation.m_frameWidth &&
		   m_frameHeight == animation.m_frameHeight &&
		   m_variant == animation.m_variant &&
		   m_version == animation.m_version &&
		   m_hasLastDelta == animation.m_hasLastDelta &&
		   m_lastDeltaValid == animation.m_lastDeltaValid &&
		   m_pixelType == animation.m_pixelType &&
		   m_compressionType == animation.m_compressionType &&
		   m_otherRecordsPerFrame == animation.m_otherRecordsPerFrame &&
		   m_bitmapType == animation.m_bitmapType &&
		   m_numberOfFrames == animation.m_numberOfFrames &&
		   m_numberOfFramesPerSecond == animation.m_numberOfFramesPerSecond &&
		   *m_recordTypes == *animation.m_recordTypes &&
		   *m_colourCycles == *animation.m_colourCycles &&
		   *m_colourTable == *animation.m_colourTable &&
		   *m_colourFlags == *animation.m_colourFlags &&
		   *m_largePageDescriptors == *animation.m_largePageDescriptors &&
		   *m_largePageData == *animation.m_largePageData;
}

bool AnimationANM::operator != (const AnimationANM & animation) const {
	return !operator == (animation);
}

void AnimationANM::updateParent() {
	m_colourTable->setParent(this);

	for(ColourCycleInfo & colourCycleInfo : *m_colourCycles) {
		colourCycleInfo.setParent(this);
	}

	for(LargePageDescriptor & largePageDescriptor : *m_largePageDescriptors) {
		largePageDescriptor.setParent(this);
	}
}
