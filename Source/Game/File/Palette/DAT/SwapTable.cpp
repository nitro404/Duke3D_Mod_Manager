#include "PaletteDAT.h"

#include <ByteBuffer.h>

#include <spdlog/spdlog.h>

PaletteDAT::SwapTable::SwapTable(Palette * parent)
	: m_swapIndex(0)
	, m_swapData(std::make_unique<SwapData>())
	, m_parent(parent) { }

PaletteDAT::SwapTable::SwapTable(std::unique_ptr<SwapData> swapData, uint8_t swapIndex, Palette * parent)
	: m_swapIndex(swapIndex)
	, m_swapData(std::move(swapData))
	, m_parent(parent) { }

PaletteDAT::SwapTable::SwapTable(const SwapData & swapData, uint8_t swapIndex, Palette * parent)
	: m_swapIndex(swapIndex)
	, m_swapData(std::make_unique<SwapData>(swapData))
	, m_parent(parent) { }

PaletteDAT::SwapTable::SwapTable(SwapTable && swapTable) noexcept
	: m_swapIndex(swapTable.m_swapIndex)
	, m_swapData(std::move(swapTable.m_swapData))
	, m_parent(nullptr) { }

PaletteDAT::SwapTable::SwapTable(const SwapTable & swapTable)
	: m_swapIndex(swapTable.m_swapIndex)
	, m_swapData(std::make_unique<SwapData>(*swapTable.m_swapData))
	, m_parent(nullptr) { }

PaletteDAT::SwapTable & PaletteDAT::SwapTable::operator = (SwapTable && swapTable) noexcept {
	if(this != &swapTable) {
		m_swapIndex = swapTable.m_swapIndex;
		m_swapData = std::move(swapTable.m_swapData);
	}

	return *this;
}

PaletteDAT::SwapTable & PaletteDAT::SwapTable::operator = (const SwapTable & swapTable) {
	m_swapIndex = swapTable.m_swapIndex;
	m_swapData = std::make_unique<SwapData>(*swapTable.m_swapData);

	return *this;
}

PaletteDAT::SwapTable::~SwapTable() { }

uint8_t PaletteDAT::SwapTable::getSwapIndex() const {
	return m_swapIndex;
}

void PaletteDAT::SwapTable::setSwapIndex(uint8_t swapIndex) {
	m_swapIndex = swapIndex;
}

const PaletteDAT::SwapTable::SwapData & PaletteDAT::SwapTable::getData() const {
	return *m_swapData;
}

void PaletteDAT::SwapTable::setData(std::unique_ptr<SwapData> swapData) {
	m_swapData = std::move(swapData);
}

void PaletteDAT::SwapTable::setData(const SwapData & swapData) {
	m_swapData = std::make_unique<SwapData>(swapData);
}

bool PaletteDAT::SwapTable::setData(const std::vector<uint8_t> & swapData) {
	if(swapData.size() != NUMBER_OF_COLOURS) {
		return false;
	}

	std::copy_n(std::make_move_iterator(swapData.begin()), NUMBER_OF_COLOURS, m_swapData->begin());

	return true;
}

std::unique_ptr<PaletteDAT::SwapTable> PaletteDAT::SwapTable::getFrom(const ByteBuffer & byteBuffer, size_t offset) {
	std::optional<uint8_t> optionalIndex(byteBuffer.getUnsignedByte(offset));

	if(!optionalIndex.has_value()) {
		return nullptr;
	}

	std::unique_ptr<SwapData> swapData(byteBuffer.getBytes<NUMBER_OF_COLOURS>(offset + 1));

	if(swapData == nullptr) {
		return nullptr;
	}

	return std::make_unique<SwapTable>(std::move(swapData), optionalIndex.value());
}

std::unique_ptr<PaletteDAT::SwapTable> PaletteDAT::SwapTable::readFrom(const ByteBuffer & byteBuffer) {
	std::optional<uint8_t> optionalIndex(byteBuffer.readUnsignedByte());

	if(!optionalIndex.has_value()) {
		return nullptr;
	}

	std::unique_ptr<SwapData> swapData(byteBuffer.readBytes<NUMBER_OF_COLOURS>());

	if(swapData == nullptr) {
		return nullptr;
	}

	return std::make_unique<SwapTable>(std::move(swapData), optionalIndex.value());
}

bool PaletteDAT::SwapTable::putIn(ByteBuffer & byteBuffer, size_t offset) const {
	if(!byteBuffer.putUnsignedByte(m_swapIndex, offset)) {
		return false;
	}

	if(!byteBuffer.putBytes(*m_swapData, offset + 1)) {
		return false;
	}

	return true;
}

bool PaletteDAT::SwapTable::insertIn(ByteBuffer & byteBuffer, size_t offset) const {
	if(!byteBuffer.insertUnsignedByte(m_swapIndex, offset)) {
		return false;
	}

	if(!byteBuffer.insertBytes(*m_swapData, offset + 1)) {
		return false;
	}

	return true;
}

bool PaletteDAT::SwapTable::writeTo(ByteBuffer & byteBuffer) const {
	if(!byteBuffer.writeUnsignedByte(m_swapIndex)) {
		return false;
	}

	if(!byteBuffer.writeBytes(*m_swapData)) {
		return false;
	}

	return true;
}

bool PaletteDAT::SwapTable::isParentValid() const {
	return m_parent != nullptr &&
		   m_parent->isValid();
}

Palette * PaletteDAT::SwapTable::getParent() const {
	return m_parent;
}

void PaletteDAT::SwapTable::setParent(Palette * palette) {
	m_parent = palette;
}

void PaletteDAT::SwapTable::clearParent() {
	m_parent = nullptr;
}

uint8_t PaletteDAT::SwapTable::operator[] (size_t index) const {
	if(index >= m_swapData->size()) {
		return 0;
	}

	return (*m_swapData)[index];
}

bool PaletteDAT::SwapTable::operator == (const SwapTable & swapTable) const {
	return m_swapIndex == swapTable.m_swapIndex &&
		   *m_swapData == *swapTable.m_swapData;
}

bool PaletteDAT::SwapTable::operator != (const SwapTable & swapTable) const {
	return !operator == (swapTable);
}
