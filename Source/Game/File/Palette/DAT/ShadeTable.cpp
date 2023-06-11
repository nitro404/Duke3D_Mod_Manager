#include "PaletteDAT.h"

#include <ByteBuffer.h>

#include <spdlog/spdlog.h>

PaletteDAT::ShadeTable::ShadeTable(Palette * parent)
	: m_shadeData(std::make_unique<ShadeData>())
	, m_parent(parent) { }

PaletteDAT::ShadeTable::ShadeTable(std::unique_ptr<ShadeData> shadeData, Palette * parent)
	: m_shadeData(std::move(shadeData))
	, m_parent(parent) { }

PaletteDAT::ShadeTable::ShadeTable(const ShadeData & shadeData, Palette * parent)
	: m_shadeData(std::make_unique<ShadeData>(shadeData))
	, m_parent(parent) { }

PaletteDAT::ShadeTable::ShadeTable(ShadeTable && shadeTable) noexcept
	: m_shadeData(std::move(shadeTable.m_shadeData))
	, m_parent(nullptr) { }

PaletteDAT::ShadeTable::ShadeTable(const ShadeTable & shadeTable)
	: m_shadeData(std::make_unique<ShadeData>(*shadeTable.m_shadeData))
	, m_parent(nullptr) { }

PaletteDAT::ShadeTable & PaletteDAT::ShadeTable::operator = (ShadeTable && shadeTable) noexcept {
	if(this != &shadeTable) {
		m_shadeData = std::move(shadeTable.m_shadeData);
	}

	return *this;
}

PaletteDAT::ShadeTable & PaletteDAT::ShadeTable::operator = (const ShadeTable & shadeTable) {
	m_shadeData = std::make_unique<ShadeData>(*shadeTable.m_shadeData);

	return *this;
}

PaletteDAT::ShadeTable::~ShadeTable() { }

const PaletteDAT::ShadeTable::ShadeData & PaletteDAT::ShadeTable::getData() const {
	return *m_shadeData;
}

void PaletteDAT::ShadeTable::setData(std::unique_ptr<ShadeData> shadeData) {
	m_shadeData = std::move(shadeData);
}

void PaletteDAT::ShadeTable::setData(const ShadeData & shadeData) {
	m_shadeData = std::make_unique<ShadeData>(shadeData);
}

bool PaletteDAT::ShadeTable::setData(const std::vector<uint8_t> & shadeData) {
	if(shadeData.size() != ColourTable::NUMBER_OF_COLOURS) {
		return false;
	}

	std::copy_n(std::make_move_iterator(shadeData.begin()), ColourTable::NUMBER_OF_COLOURS, m_shadeData->begin());

	return true;
}

std::unique_ptr<PaletteDAT::ShadeTable> PaletteDAT::ShadeTable::getFrom(const ByteBuffer & byteBuffer, size_t offset) {
	std::unique_ptr<ShadeData> shadeData(byteBuffer.getBytes<ColourTable::NUMBER_OF_COLOURS>(offset));

	if(shadeData == nullptr) {
		return nullptr;
	}

	return std::make_unique<ShadeTable>(std::move(shadeData));
}

std::unique_ptr<PaletteDAT::ShadeTable> PaletteDAT::ShadeTable::readFrom(const ByteBuffer & byteBuffer) {
	std::unique_ptr<ShadeData> shadeData(byteBuffer.readBytes<ColourTable::NUMBER_OF_COLOURS>());

	if(shadeData == nullptr) {
		return nullptr;
	}

	return std::make_unique<ShadeTable>(std::move(shadeData));
}

bool PaletteDAT::ShadeTable::putIn(ByteBuffer & byteBuffer, size_t offset) const {
	return byteBuffer.putBytes(*m_shadeData, offset);
}

bool PaletteDAT::ShadeTable::insertIn(ByteBuffer & byteBuffer, size_t offset) const {
	return byteBuffer.insertBytes(*m_shadeData, offset);
}

bool PaletteDAT::ShadeTable::writeTo(ByteBuffer & byteBuffer) const {
	return byteBuffer.writeBytes(*m_shadeData);
}

bool PaletteDAT::ShadeTable::isParentValid() const {
	return m_parent != nullptr &&
		   m_parent->isValid();
}

Palette * PaletteDAT::ShadeTable::getParent() const {
	return m_parent;
}

void PaletteDAT::ShadeTable::setParent(Palette * palette) {
	m_parent = palette;
}

void PaletteDAT::ShadeTable::clearParent() {
	m_parent = nullptr;
}

uint8_t PaletteDAT::ShadeTable::operator[] (size_t index) const {
	if(index >= m_shadeData->size()) {
		return 0;
	}

	return (*m_shadeData)[index];
}

bool PaletteDAT::ShadeTable::operator == (const ShadeTable & shadeTable) const {
	return *m_shadeData == *shadeTable.m_shadeData;
}

bool PaletteDAT::ShadeTable::operator != (const ShadeTable & shadeTable) const {
	return !operator == (shadeTable);
}
