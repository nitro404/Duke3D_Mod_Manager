#include "AnimationANM.h"

#include <ByteBuffer.h>
#include <Endianness.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

AnimationANM::ColourCycleInfo::ColourCycleInfo(AnimationANM * parent)
	: m_count(0)
	, m_rate(0)
	, m_flags(0)
	, m_low(0)
	, m_high(0)
	, m_parent(parent) { }

AnimationANM::ColourCycleInfo::ColourCycleInfo(uint16_t count, uint16_t rate, uint16_t flags, uint8_t low, uint8_t high, AnimationANM * parent)
	: m_count(count)
	, m_rate(rate)
	, m_flags(flags)
	, m_low(low)
	, m_high(high)
	, m_parent(parent) { }

AnimationANM::ColourCycleInfo::ColourCycleInfo(ColourCycleInfo && colourCycleInfo) noexcept
	: m_count(colourCycleInfo.m_count)
	, m_rate(colourCycleInfo.m_rate)
	, m_flags(colourCycleInfo.m_flags)
	, m_low(colourCycleInfo.m_low)
	, m_high(colourCycleInfo.m_high)
	, m_parent(nullptr) { }

AnimationANM::ColourCycleInfo::ColourCycleInfo(const ColourCycleInfo & colourCycleInfo)
	: m_count(colourCycleInfo.m_count)
	, m_rate(colourCycleInfo.m_rate)
	, m_flags(colourCycleInfo.m_flags)
	, m_low(colourCycleInfo.m_low)
	, m_high(colourCycleInfo.m_high)
	, m_parent(nullptr) { }

AnimationANM::ColourCycleInfo & AnimationANM::ColourCycleInfo::operator = (ColourCycleInfo && colourCycleInfo) noexcept {
	if(this != &colourCycleInfo) {
		m_count = colourCycleInfo.m_count;
		m_rate = colourCycleInfo.m_rate;
		m_flags = colourCycleInfo.m_flags;
		m_low = colourCycleInfo.m_low;
		m_high = colourCycleInfo.m_high;
	}

	return *this;
}

AnimationANM::ColourCycleInfo & AnimationANM::ColourCycleInfo::operator = (const ColourCycleInfo & colourCycleInfo) {
	m_count = colourCycleInfo.m_count;
	m_rate = colourCycleInfo.m_rate;
	m_flags = colourCycleInfo.m_flags;
	m_low = colourCycleInfo.m_low;
	m_high = colourCycleInfo.m_high;

	return *this;
}

AnimationANM::ColourCycleInfo::~ColourCycleInfo() = default;

uint16_t AnimationANM::ColourCycleInfo::getCount() const {
	return m_count;
}

uint16_t AnimationANM::ColourCycleInfo::getRate() const {
	return m_rate;
}

uint16_t AnimationANM::ColourCycleInfo::getFlags() const {
	return m_flags;
}

uint8_t AnimationANM::ColourCycleInfo::getLow() const {
	return m_low;
}

uint8_t AnimationANM::ColourCycleInfo::getHigh() const {
	return m_high;
}

AnimationANM::ColourCycleInfo AnimationANM::ColourCycleInfo::getFrom(const ByteBuffer & byteBuffer, size_t offset, bool * error) {
	bool internalError = false;
	size_t newOffset = offset;

	uint16_t count = byteBuffer.getUnsignedShort(newOffset, &internalError);

	if(internalError) {
		spdlog::error("Missing colour cycle info count value.");

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	newOffset += sizeof(uint16_t);

	uint16_t rate = byteBuffer.getUnsignedShort(newOffset, &internalError);

	if(internalError) {
		spdlog::error("Missing colour cycle info rate value.");

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	newOffset += sizeof(uint16_t);

	uint16_t flags = byteBuffer.getUnsignedShort(newOffset, &internalError);

	if(internalError) {
		spdlog::error("Missing colour cycle info flags value.");

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	newOffset += sizeof(uint16_t);

	uint8_t low = byteBuffer.getUnsignedByte(newOffset, &internalError);

	if(internalError) {
		spdlog::error("Missing colour cycle info low value.");

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	newOffset += sizeof(uint8_t);

	uint8_t high = byteBuffer.getUnsignedByte(newOffset, &internalError);

	if(internalError) {
		spdlog::error("Missing colour cycle info high value.");

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	return ColourCycleInfo(count, rate, flags, low, high);
}

std::optional<AnimationANM::ColourCycleInfo> AnimationANM::ColourCycleInfo::getFrom(const ByteBuffer & byteBuffer, size_t offset) {
	bool error = false;
	ColourCycleInfo value(getFrom(byteBuffer, byteBuffer.getReadOffset(), &error));

	if(error) {
		return {};
	}

	return value;
}

AnimationANM::ColourCycleInfo AnimationANM::ColourCycleInfo::readFrom(const ByteBuffer & byteBuffer, bool * error) {
	bool internalError = false;
	ColourCycleInfo value(getFrom(byteBuffer, byteBuffer.getReadOffset(), &internalError));

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

std::optional<AnimationANM::ColourCycleInfo> AnimationANM::ColourCycleInfo::readFrom(const ByteBuffer & byteBuffer) {
	bool error = false;
	ColourCycleInfo value(readFrom(byteBuffer, &error));

	if(error) {
		return {};
	}

	return value;
}

bool AnimationANM::ColourCycleInfo::putIn(ByteBuffer & byteBuffer, size_t offset) const {
	size_t newOffset = offset;

	if(!byteBuffer.putUnsignedShort(m_count, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(!byteBuffer.putUnsignedShort(m_rate, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(!byteBuffer.putUnsignedShort(m_flags, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(!byteBuffer.putUnsignedByte(m_low, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(!byteBuffer.putUnsignedByte(m_high, newOffset)) {
		return false;
	}

	return true;
}

bool AnimationANM::ColourCycleInfo::insertIn(ByteBuffer & byteBuffer, size_t offset) const {
	size_t newOffset = offset;

	if(!byteBuffer.insertUnsignedShort(m_count, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(!byteBuffer.insertUnsignedShort(m_rate, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(!byteBuffer.insertUnsignedShort(m_flags, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(!byteBuffer.insertUnsignedByte(m_low, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(!byteBuffer.insertUnsignedByte(m_high, newOffset)) {
		return false;
	}

	return true;
}

bool AnimationANM::ColourCycleInfo::writeTo(ByteBuffer & byteBuffer) const {
	if(!byteBuffer.writeUnsignedShort(m_count)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(m_rate)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(m_flags)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedByte(m_low)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedByte(m_high)) {
		return false;
	}

	return true;
}

bool AnimationANM::ColourCycleInfo::isParentValid() const {
	return m_parent != nullptr &&
		   m_parent->isValid(false);
}

AnimationANM * AnimationANM::ColourCycleInfo::getParent() const {
	return m_parent;
}

void AnimationANM::ColourCycleInfo::setParent(AnimationANM * parent) {
	m_parent = parent;
}

void AnimationANM::ColourCycleInfo::clearParent() {
	m_parent = nullptr;
}

bool AnimationANM::ColourCycleInfo::operator == (const ColourCycleInfo & colourCycleInfo) const {
	return m_count == colourCycleInfo.m_count &&
		   m_rate == colourCycleInfo.m_rate &&
		   m_flags == colourCycleInfo.m_flags &&
		   m_low == colourCycleInfo.m_low &&
		   m_high == colourCycleInfo.m_high;
}

bool AnimationANM::ColourCycleInfo::operator != (const ColourCycleInfo & colourCycleInfo) const {
	return !operator == (colourCycleInfo);
}
