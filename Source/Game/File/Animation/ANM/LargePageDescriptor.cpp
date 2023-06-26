#include "AnimationANM.h"

#include <ByteBuffer.h>
#include <Endianness.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

struct RecordInfo final {
	union {
		struct {
			uint16_t numberOfRecords : 14;
			bool lastRecordContinuesToNextLargePage : 1;
			bool firstRecordContinuesFromPreviousLargePage : 1;
		};

		uint16_t rawValue;
	};
};

AnimationANM::LargePageDescriptor::LargePageDescriptor(AnimationANM * parent)
	: m_numberOfBytes(0)
	, m_firstRecordNumber(0)
	, m_numberOfRecords(0)
	, m_firstRecordContinuesFromPreviousLargePage(false)
	, m_lastRecordContinuesToNextLargePage(false)
	, m_parent(parent) { }

AnimationANM::LargePageDescriptor::LargePageDescriptor(uint16_t numberOfBytes, uint16_t firstRecordNumber, uint16_t numberOfRecords, bool firstRecordContinuesFromPreviousLargePage, bool lastRecordContinuesToNextLargePage, AnimationANM * parent)
	: m_firstRecordNumber(firstRecordNumber)
	, m_numberOfRecords(numberOfRecords)
	, m_lastRecordContinuesToNextLargePage(lastRecordContinuesToNextLargePage)
	, m_firstRecordContinuesFromPreviousLargePage(firstRecordContinuesFromPreviousLargePage)
	, m_numberOfBytes(numberOfBytes)
	, m_parent(parent) { }

AnimationANM::LargePageDescriptor::LargePageDescriptor(LargePageDescriptor && largePageDescriptor) noexcept
	: m_firstRecordNumber(largePageDescriptor.m_firstRecordNumber)
	, m_numberOfRecords(largePageDescriptor.m_numberOfRecords)
	, m_lastRecordContinuesToNextLargePage(largePageDescriptor.m_lastRecordContinuesToNextLargePage)
	, m_firstRecordContinuesFromPreviousLargePage(largePageDescriptor.m_firstRecordContinuesFromPreviousLargePage)
	, m_numberOfBytes(largePageDescriptor.m_numberOfBytes)
	, m_parent(nullptr) { }

AnimationANM::LargePageDescriptor::LargePageDescriptor(const LargePageDescriptor & largePageDescriptor)
	: m_firstRecordNumber(largePageDescriptor.m_firstRecordNumber)
	, m_numberOfRecords(largePageDescriptor.m_numberOfRecords)
	, m_lastRecordContinuesToNextLargePage(largePageDescriptor.m_lastRecordContinuesToNextLargePage)
	, m_firstRecordContinuesFromPreviousLargePage(largePageDescriptor.m_firstRecordContinuesFromPreviousLargePage)
	, m_numberOfBytes(largePageDescriptor.m_numberOfBytes)
	, m_parent(nullptr) { }

AnimationANM::LargePageDescriptor & AnimationANM::LargePageDescriptor::operator = (LargePageDescriptor && largePageDescriptor) noexcept {
	if(this != &largePageDescriptor) {
		m_firstRecordNumber = largePageDescriptor.m_firstRecordNumber;
		m_numberOfRecords = largePageDescriptor.m_numberOfRecords;
		m_lastRecordContinuesToNextLargePage = largePageDescriptor.m_lastRecordContinuesToNextLargePage;
		m_firstRecordContinuesFromPreviousLargePage = largePageDescriptor.m_firstRecordContinuesFromPreviousLargePage;
		m_numberOfBytes = largePageDescriptor.m_numberOfBytes;
	}

	return *this;
}

AnimationANM::LargePageDescriptor & AnimationANM::LargePageDescriptor::operator = (const LargePageDescriptor & largePageDescriptor) {
	m_firstRecordNumber = largePageDescriptor.m_firstRecordNumber;
	m_numberOfRecords = largePageDescriptor.m_numberOfRecords;
	m_lastRecordContinuesToNextLargePage = largePageDescriptor.m_lastRecordContinuesToNextLargePage;
	m_firstRecordContinuesFromPreviousLargePage = largePageDescriptor.m_firstRecordContinuesFromPreviousLargePage;
	m_numberOfBytes = largePageDescriptor.m_numberOfBytes;

	return *this;
}

AnimationANM::LargePageDescriptor::~LargePageDescriptor() = default;

uint16_t AnimationANM::LargePageDescriptor::numberOfBytes() const {
	return m_numberOfBytes;
}

uint16_t AnimationANM::LargePageDescriptor::getFirstRecordNumber() const {
	return m_firstRecordNumber;
}

uint16_t AnimationANM::LargePageDescriptor::numberOfRecords() const {
	return m_numberOfRecords;
}

bool AnimationANM::LargePageDescriptor::doesFirstRecordContinueFromPreviousLargePage() const {
	return m_firstRecordContinuesFromPreviousLargePage;
}

bool AnimationANM::LargePageDescriptor::doesLastRecordContinueToNextLargePage() const {
	return m_lastRecordContinuesToNextLargePage;
}

AnimationANM::LargePageDescriptor AnimationANM::LargePageDescriptor::getFrom(const ByteBuffer & byteBuffer, size_t offset, bool * error) {
	bool internalError = false;
	size_t newOffset = offset;

	uint16_t firstRecordNumber = byteBuffer.getUnsignedShort(newOffset, &internalError);

	if(internalError) {
		spdlog::error("Missing large page descriptor first record number value.");

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	newOffset += sizeof(uint16_t);

	RecordInfo recordInfo({ 0 });
	recordInfo.rawValue = byteBuffer.getUnsignedShort(newOffset, &internalError);

	if(internalError) {
		spdlog::error("Missing large page descriptor record info value.");

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	newOffset += sizeof(uint16_t);

	uint16_t numberOfBytes = byteBuffer.getUnsignedShort(newOffset, &internalError);

	if(internalError) {
		spdlog::error("Missing large page descriptor number of bytes value.");

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	return LargePageDescriptor(
		numberOfBytes,
		firstRecordNumber,
		+recordInfo.numberOfRecords,
		+recordInfo.firstRecordContinuesFromPreviousLargePage,
		+recordInfo.lastRecordContinuesToNextLargePage
	);
}

std::optional<AnimationANM::LargePageDescriptor> AnimationANM::LargePageDescriptor::getFrom(const ByteBuffer & byteBuffer, size_t offset) {
	bool error = false;
	LargePageDescriptor value(getFrom(byteBuffer, byteBuffer.getReadOffset(), &error));

	if(error) {
		return {};
	}

	return value;
}

AnimationANM::LargePageDescriptor AnimationANM::LargePageDescriptor::readFrom(const ByteBuffer & byteBuffer, bool * error) {
	bool internalError = false;
	LargePageDescriptor value(getFrom(byteBuffer, byteBuffer.getReadOffset(), &internalError));

	if(internalError) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		byteBuffer.skipReadBytes(SIZE_BYTES);
	}

	return value;
}

std::optional<AnimationANM::LargePageDescriptor> AnimationANM::LargePageDescriptor::readFrom(const ByteBuffer & byteBuffer) {
	bool error = false;
	LargePageDescriptor value(readFrom(byteBuffer, &error));

	if(error) {
		return {};
	}

	return value;
}

bool AnimationANM::LargePageDescriptor::putIn(ByteBuffer & byteBuffer, size_t offset) const {
	size_t newOffset = offset;

	if(!byteBuffer.putUnsignedShort(m_firstRecordNumber, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	RecordInfo recordInfo({
		m_numberOfRecords,
		m_lastRecordContinuesToNextLargePage,
		m_firstRecordContinuesFromPreviousLargePage
	});

	if(!byteBuffer.putUnsignedShort(recordInfo.rawValue, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(!byteBuffer.putUnsignedShort(m_numberOfBytes, newOffset)) {
		return false;
	}

	return true;
}

bool AnimationANM::LargePageDescriptor::insertIn(ByteBuffer & byteBuffer, size_t offset) const {
	size_t newOffset = offset;

	if(!byteBuffer.insertUnsignedShort(m_firstRecordNumber, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	RecordInfo recordInfo({
		m_numberOfRecords,
		m_lastRecordContinuesToNextLargePage,
		m_firstRecordContinuesFromPreviousLargePage
	});

	if(!byteBuffer.insertUnsignedShort(recordInfo.rawValue, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(!byteBuffer.insertUnsignedShort(m_numberOfBytes, newOffset)) {
		return false;
	}

	return true;
}

bool AnimationANM::LargePageDescriptor::writeTo(ByteBuffer & byteBuffer) const {
	if(!byteBuffer.writeUnsignedShort(m_firstRecordNumber)) {
		return false;
	}

	RecordInfo recordInfo({
		m_numberOfRecords,
		m_lastRecordContinuesToNextLargePage,
		m_firstRecordContinuesFromPreviousLargePage
	});

	if(!byteBuffer.writeUnsignedShort(recordInfo.rawValue)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(m_numberOfBytes)) {
		return false;
	}

	return true;
}

bool AnimationANM::LargePageDescriptor::isValid() const {
	return m_numberOfBytes != 0 &&
		   m_numberOfRecords < 16384;
}

bool AnimationANM::LargePageDescriptor::isParentValid() const {
	return m_parent != nullptr &&
		   m_parent->isValid(false);
}

AnimationANM * AnimationANM::LargePageDescriptor::getParent() const {
	return m_parent;
}

void AnimationANM::LargePageDescriptor::setParent(AnimationANM * parent) {
	m_parent = parent;
}

void AnimationANM::LargePageDescriptor::clearParent() {
	m_parent = nullptr;
}

bool AnimationANM::LargePageDescriptor::operator == (const LargePageDescriptor & largePageDescriptor) const {
	return m_firstRecordNumber == largePageDescriptor.m_firstRecordNumber &&
		   m_numberOfRecords == largePageDescriptor.m_numberOfRecords &&
		   m_lastRecordContinuesToNextLargePage == largePageDescriptor.m_lastRecordContinuesToNextLargePage &&
		   m_firstRecordContinuesFromPreviousLargePage == largePageDescriptor.m_firstRecordContinuesFromPreviousLargePage &&
		   m_numberOfBytes == largePageDescriptor.m_numberOfBytes;
}

bool AnimationANM::LargePageDescriptor::operator != (const LargePageDescriptor & largePageDescriptor) const {
	return !operator == (largePageDescriptor);
}
