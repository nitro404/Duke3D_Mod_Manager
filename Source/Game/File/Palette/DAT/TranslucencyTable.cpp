#include "PaletteDAT.h"

#include <ByteBuffer.h>

#include <spdlog/spdlog.h>

PaletteDAT::TranslucencyTable::TranslucencyTable(Palette * parent)
	: m_translucencyData(std::make_unique<TranslucencyData>())
	, m_parent(parent) { }

PaletteDAT::TranslucencyTable::TranslucencyTable(std::unique_ptr<TranslucencyData> translucencyData, Palette * parent)
	: m_translucencyData(std::move(translucencyData))
	, m_parent(parent) { }

PaletteDAT::TranslucencyTable::TranslucencyTable(const TranslucencyData & translucencyData, Palette * parent)
	: m_translucencyData(std::make_unique<TranslucencyData>(translucencyData))
	, m_parent(parent) { }

PaletteDAT::TranslucencyTable::TranslucencyTable(TranslucencyTable && translucencyTable) noexcept
	: m_translucencyData(std::move(translucencyTable.m_translucencyData))
	, m_parent(nullptr) { }

PaletteDAT::TranslucencyTable::TranslucencyTable(const TranslucencyTable & translucencyTable)
	: m_translucencyData(std::make_unique<TranslucencyData>(*translucencyTable.m_translucencyData))
	, m_parent(nullptr) { }

PaletteDAT::TranslucencyTable & PaletteDAT::TranslucencyTable::operator = (TranslucencyTable && translucencyTable) noexcept {
	if(this != &translucencyTable) {
		m_translucencyData = std::move(translucencyTable.m_translucencyData);
	}

	return *this;
}

PaletteDAT::TranslucencyTable & PaletteDAT::TranslucencyTable::operator = (const TranslucencyTable & translucencyTable) {
	m_translucencyData = std::make_unique<TranslucencyData>(*translucencyTable.m_translucencyData);

	return *this;
}

PaletteDAT::TranslucencyTable::~TranslucencyTable() { }

const PaletteDAT::TranslucencyTable::TranslucencyData & PaletteDAT::TranslucencyTable::getData() const {
	return *m_translucencyData;
}

void PaletteDAT::TranslucencyTable::setData(std::unique_ptr<TranslucencyData> translucencyData) {
	m_translucencyData = std::move(translucencyData);
}

void PaletteDAT::TranslucencyTable::setData(const TranslucencyData & translucencyData) {
	m_translucencyData = std::make_unique<TranslucencyData>(translucencyData);
}

bool PaletteDAT::TranslucencyTable::setData(const std::vector<uint8_t> & translucencyData) {
	if(translucencyData.size() != TRANSLUCENCY_TABLE_SIZE_BYTES) {
		return false;
	}

	std::copy_n(std::make_move_iterator(translucencyData.begin()), TRANSLUCENCY_TABLE_SIZE_BYTES, m_translucencyData->begin());

	return true;
}

std::unique_ptr<PaletteDAT::TranslucencyTable> PaletteDAT::TranslucencyTable::getFrom(const ByteBuffer & byteBuffer, size_t offset) {
	std::unique_ptr<TranslucencyData> translucencyData(byteBuffer.getBytes<TRANSLUCENCY_TABLE_SIZE_BYTES>(offset));

	if(translucencyData == nullptr) {
		return nullptr;
	}

	return std::make_unique<TranslucencyTable>(std::move(translucencyData));
}

std::unique_ptr<PaletteDAT::TranslucencyTable> PaletteDAT::TranslucencyTable::readFrom(const ByteBuffer & byteBuffer) {
	std::unique_ptr<TranslucencyData> translucencyData(byteBuffer.readBytes<TRANSLUCENCY_TABLE_SIZE_BYTES>());

	if(translucencyData == nullptr) {
		return nullptr;
	}

	return std::make_unique<TranslucencyTable>(std::move(translucencyData));
}

bool PaletteDAT::TranslucencyTable::putIn(ByteBuffer & byteBuffer, size_t offset) const {
	return byteBuffer.putBytes(*m_translucencyData, offset);
}

bool PaletteDAT::TranslucencyTable::insertIn(ByteBuffer & byteBuffer, size_t offset) const {
	return byteBuffer.insertBytes(*m_translucencyData, offset);
}

bool PaletteDAT::TranslucencyTable::writeTo(ByteBuffer & byteBuffer) const {
	return byteBuffer.writeBytes(*m_translucencyData);
}

bool PaletteDAT::TranslucencyTable::isParentValid() const {
	return m_parent != nullptr &&
		   m_parent->isValid();
}

Palette * PaletteDAT::TranslucencyTable::getParent() const {
	return m_parent;
}

void PaletteDAT::TranslucencyTable::setParent(Palette * palette) {
	m_parent = palette;
}

void PaletteDAT::TranslucencyTable::clearParent() {
	m_parent = nullptr;
}

uint8_t PaletteDAT::TranslucencyTable::operator[] (size_t index) const {
	if(index >= m_translucencyData->size()) {
		return 0;
	}

	return (*m_translucencyData)[index];
}

bool PaletteDAT::TranslucencyTable::operator == (const TranslucencyTable & translucencyTable) const {
	return *m_translucencyData == *translucencyTable.m_translucencyData;
}

bool PaletteDAT::TranslucencyTable::operator != (const TranslucencyTable & translucencyTable) const {
	return !operator == (translucencyTable);
}
