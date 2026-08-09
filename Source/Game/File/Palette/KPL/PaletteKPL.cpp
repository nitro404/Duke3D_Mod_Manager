#include "PaletteKPL.h"

#include <ByteBuffer.h>
#include <Point2D.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/TinyXML2Utilities.h>

#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <sstream>

const std::string PaletteKPL::MIME_TYPE("application/x-krita-palette");
const std::string PaletteKPL::COLOR_SET_VERSION("1.0");

static const std::string KRITAL_PALETTE_MIME_TYPE_FILE_NAME("mimetype");
static const std::string KRITAL_PALETTE_COLOR_SET_FILE_NAME("colorset.xml");

static const std::string KRITA_PALETTE_GROUP_ELEMENT_NAME("Group");

static const std::string KRITA_PALETTE_COLOR_SET_ELEMENT_NAME("ColorSet");
static const std::string KRITA_PALETTE_COLOR_SET_ELEMENT_NAME_ATTRIBUTE_NAME("name");
static const std::string KRITA_PALETTE_COLOR_SET_ELEMENT_COMMENT_ATTRIBUTE_NAME("comment");
static const std::string KRITA_PALETTE_COLOR_SET_ELEMENT_VERSION_ATTRIBUTE_NAME("version");
static const std::string KRITA_PALETTE_COLOR_SET_ELEMENT_READ_ONLY_ATTRIBUTE_NAME("readonly");
static const std::string KRITA_PALETTE_COLOR_SET_ELEMENT_COLUMNS_ATTRIBUTE_NAME("columns");

static const std::string KRITA_PALETTE_COLOR_SET_ENTRY_ELEMENT_NAME("ColorSetEntry");
static const std::string KRITA_PALETTE_COLOR_SET_ENTRY_NAME_ATTRIBUTE_NAME("name");
static const std::string KRITA_PALETTE_COLOR_SET_ENTRY_ID_ATTRIBUTE_NAME("id");
static const std::string KRITA_PALETTE_COLOR_SET_ENTRY_BIT_DEPTH_ATTRIBUTE_NAME("bitdepth");
static const std::string KRITA_PALETTE_COLOR_SET_ENTRY_SPOT_ATTRIBUTE_NAME("spot");

static const std::string KRITA_PALETTE_RGB_ELEMENT_NAME("RGB");
static const std::string KRITA_PALETTE_RGB_RED_ATTRIBUTE_NAME("r");
static const std::string KRITA_PALETTE_RGB_GREEN_ATTRIBUTE_NAME("g");
static const std::string KRITA_PALETTE_RGB_BLUE_ATTRIBUTE_NAME("b");
static const std::array<const std::string *, 3> KRITA_PALETTE_RGB_ATTRIBUTE_NAMES = {
	&KRITA_PALETTE_RGB_RED_ATTRIBUTE_NAME,
	&KRITA_PALETTE_RGB_GREEN_ATTRIBUTE_NAME,
	&KRITA_PALETTE_RGB_BLUE_ATTRIBUTE_NAME
};

static const std::string KRITA_PALETTE_POSITION_ELEMENT_NAME("Position");
static const std::string KRITA_PALETTE_POSITION_ROW_ATTRIBUTE_NAME("row");
static const std::string KRITA_PALETTE_POSITION_COLUMN_ATTRIBUTE_NAME("column");
static const std::array<const std::string *, 2> KRITA_PALETTE_POSITION_ATTRIBUTE_NAMES = {
	&KRITA_PALETTE_POSITION_COLUMN_ATTRIBUTE_NAME,
	&KRITA_PALETTE_POSITION_ROW_ATTRIBUTE_NAME
};

PaletteKPL::PaletteKPL(std::string_view comment, uint8_t numberOfColumns, bool readOnly, std::unique_ptr<ZipArchive> archive, const std::string & filePath)
	: Palette(filePath)
	, m_comment(comment)
	, m_numberOfColumns(numberOfColumns)
	, m_readOnly(readOnly)
	, m_archive(archive != nullptr ? std::move(archive) : ZipArchive::createNew())
	, m_colourTable(std::make_shared<ColourTable>()) {
	updateParent();
	connectSignals();
	ensureRequiredFiles();
}

PaletteKPL::PaletteKPL(std::unique_ptr<ColourTable> colourTable, std::string_view comment, uint8_t numberOfColumns, bool readOnly, std::unique_ptr<ZipArchive> archive, const std::string & filePath)
	: Palette(filePath)
	, m_comment(comment)
	, m_numberOfColumns(numberOfColumns)
	, m_readOnly(readOnly)
	, m_archive(archive != nullptr ? std::move(archive) : ZipArchive::createNew())
	, m_colourTable(colourTable != nullptr ? std::move(colourTable) : std::make_shared<ColourTable>()) {
	updateParent();
	connectSignals();
	ensureRequiredFiles();
}

PaletteKPL::PaletteKPL(PaletteKPL && palette) noexcept
	: Palette(std::move(palette))
	, m_comment(std::move(palette.m_comment))
	, m_numberOfColumns(palette.m_numberOfColumns)
	, m_readOnly(palette.m_readOnly)
	, m_archive(std::move(palette.m_archive))
	, m_colourTable(std::move(palette.m_colourTable)) {
	updateParent();
	connectSignals();
	ensureRequiredFiles();
}

PaletteKPL::PaletteKPL(const PaletteKPL & palette)
	: Palette(palette)
	, m_comment(std::move(palette.m_comment))
	, m_numberOfColumns(palette.m_numberOfColumns)
	, m_readOnly(palette.m_readOnly)
	, m_archive(ZipArchive::createNew())
	, m_colourTable(palette.m_colourTable) {
	updateParent();
	connectSignals();
	ensureRequiredFiles();
}

PaletteKPL & PaletteKPL::operator = (PaletteKPL && palette) noexcept {
	if(this != &palette) {
		Palette::operator = (std::move(palette));

		m_colourTableModifiedConnection.disconnect();

		m_comment = std::move(palette.m_comment);
		m_numberOfColumns = palette.m_numberOfColumns;
		m_readOnly = palette.m_readOnly;
		m_archive = std::move(palette.m_archive);
		m_colourTable = std::move(palette.m_colourTable);

		updateParent();
		connectSignals();
		ensureRequiredFiles();
	}

	return *this;
}

PaletteKPL & PaletteKPL::operator = (const PaletteKPL & palette) {
	Palette::operator = (palette);

	m_colourTableModifiedConnection.disconnect();

	m_comment = palette.m_comment;
	m_numberOfColumns = palette.m_numberOfColumns;
	m_readOnly = palette.m_readOnly;
	m_archive = ZipArchive::createNew();
	m_colourTable = palette.m_colourTable;

	updateParent();
	connectSignals();
	ensureRequiredFiles();

	return *this;
}

PaletteKPL::~PaletteKPL() {
	m_colourTableModifiedConnection.disconnect();
}

void PaletteKPL::setModified(bool modified) const {
	if(!modified) {
		m_colourTable->setModified(false);
	}

	Palette::setModified(modified);
}

const std::string & PaletteKPL::getName() const {
	return m_colourTable->getName();
}

void PaletteKPL::setName(std::string_view name) {
	m_colourTable->setName(name);
}

const std::string & PaletteKPL::getComment() const {
	return m_comment;
}

void PaletteKPL::setComment(std::string_view comment) {
	if(Utilities::areStringsEqual(m_comment, comment)) {
		return;
	}

	m_comment = comment;

	setModified(true);
}

uint8_t PaletteKPL::getNumberOfColumns() const {
	return m_numberOfColumns;
}

void PaletteKPL::setNumberOfColumns(uint8_t numberOfColumns) {
	if(m_numberOfColumns == numberOfColumns) {
		return;
	}

	m_numberOfColumns = numberOfColumns;

	setModified(true);
}

bool PaletteKPL::isReadOnly() const {
	return m_readOnly;
}

void PaletteKPL::setReadOnly(bool readOnly) {
	if(m_readOnly == readOnly) {
		return;
	}

	m_readOnly = readOnly;

	setModified(true);
}

std::shared_ptr<ColourTable> PaletteKPL::getColourTable(uint8_t colourTableIndex) const {
	if(colourTableIndex != 0) {
		return nullptr;
	}

	return m_colourTable;
}

std::unique_ptr<PaletteKPL> PaletteKPL::readFrom(const ByteBuffer & byteBuffer) {
	std::unique_ptr<ZipArchive> archive(ZipArchive::createFrom(std::make_unique<ByteBuffer>(byteBuffer)));

	if(archive == nullptr) {
		spdlog::error("Failed to create Krita palette archive handle from data.");
		return nullptr;
	}

	const std::shared_ptr<ArchiveEntry> mimeTypeFile(archive->getEntry(KRITAL_PALETTE_MIME_TYPE_FILE_NAME));

	if(mimeTypeFile == nullptr) {
		spdlog::error("Krita palette archive is missing '{}' file.", KRITAL_PALETTE_MIME_TYPE_FILE_NAME);
		return nullptr;
	}

	const std::unique_ptr<ByteBuffer> mimeTypeData(mimeTypeFile->getData());

	if(mimeTypeData == nullptr) {
		spdlog::error("Failed to decompress Krita palette '{}' file data.", KRITAL_PALETTE_MIME_TYPE_FILE_NAME);
		return nullptr;
	}

	bool error = false;
	const std::string mimeType(mimeTypeData->readString(MIME_TYPE.length(), &error));

	if(error) {
		spdlog::error("Failed to read Krita palette mime type data.");
		return nullptr;
	}

	if(!Utilities::areStringsEqualIgnoreCase(mimeType, MIME_TYPE)) {
		spdlog::error("Unsupported Krita palette mime type: '{}', expected: '{}'.", mimeType, MIME_TYPE);
		return nullptr;
	}

	std::shared_ptr<ArchiveEntry> colorSetFile(archive->getEntry(KRITAL_PALETTE_COLOR_SET_FILE_NAME));

	if(colorSetFile == nullptr) {
		spdlog::error("Krita palette archive is missing '{}' file.", KRITAL_PALETTE_COLOR_SET_FILE_NAME);
		return nullptr;
	}

	const std::unique_ptr<ByteBuffer> colorSetData(colorSetFile->getData());

	if(colorSetData == nullptr) {
		spdlog::error("Failed to decompress Krita palette '{}' file data.", KRITAL_PALETTE_COLOR_SET_FILE_NAME);
		return nullptr;
	}

	tinyxml2::XMLDocument document;

	if(document.Parse(reinterpret_cast<const char *>(colorSetData->getRawData()), colorSetData->getSize()) != tinyxml2::XML_SUCCESS) {
		spdlog::error("Failed to parse Krita palette '{}' file data with error: '{}'.", KRITAL_PALETTE_COLOR_SET_FILE_NAME, document.ErrorStr());
		return {};
	}

	const tinyxml2::XMLElement * colorSetElement = document.RootElement();

	if(colorSetElement == nullptr || !Utilities::areStringsEqualIgnoreCase(colorSetElement->Name(), KRITA_PALETTE_COLOR_SET_ELEMENT_NAME)) {
		spdlog::error("Krita palette color set is missing '{}' root element.", KRITA_PALETTE_COLOR_SET_ELEMENT_NAME);
		return {};
	}

	// Color Set Version
	const char * colorSetVersionData = colorSetElement->Attribute(KRITA_PALETTE_COLOR_SET_ELEMENT_VERSION_ATTRIBUTE_NAME.c_str());

	if(Utilities::isEmptyString(colorSetVersionData)) {
		spdlog::error("Krita palette '{}' element attribute '{}' is missing.", KRITA_PALETTE_COLOR_SET_ELEMENT_NAME, KRITA_PALETTE_COLOR_SET_ELEMENT_VERSION_ATTRIBUTE_NAME);
		return nullptr;
	}

	if(!Utilities::areStringsEqualIgnoreCase(colorSetVersionData, COLOR_SET_VERSION)) {
		spdlog::error("Krita palette color set version '{}' detected, while only version '{}' is supported.", colorSetVersionData, COLOR_SET_VERSION);
		return nullptr;
	}

	// Color Set Name
	std::string colorSetName;
	const char * colorSetNameData = colorSetElement->Attribute(KRITA_PALETTE_COLOR_SET_ELEMENT_NAME_ATTRIBUTE_NAME.c_str());

	if(!Utilities::isEmptyString(colorSetNameData)) {
		colorSetName = colorSetNameData;
	}

	// Color Set Comment
	std::string colorSetComment;
	const char * colorSetCommentData = colorSetElement->Attribute(KRITA_PALETTE_COLOR_SET_ELEMENT_COMMENT_ATTRIBUTE_NAME.c_str());

	if(!Utilities::isEmptyString(colorSetCommentData)) {
		colorSetComment = colorSetCommentData;
	}

	// Color Set Read-Only Flag
	bool readOnly = DEFAULT_READ_ONLY;
	const char * colorSetReadOnlyData = colorSetElement->Attribute(KRITA_PALETTE_COLOR_SET_ELEMENT_READ_ONLY_ATTRIBUTE_NAME.c_str());

	if(Utilities::isNonEmptyString(colorSetReadOnlyData)) {
		readOnly = Utilities::parseBoolean(colorSetReadOnlyData, &error);

		if(error) {
			spdlog::error("Krita palette color set has invalid '{}' attribute value: '{}', expected boolean.", KRITA_PALETTE_COLOR_SET_ELEMENT_READ_ONLY_ATTRIBUTE_NAME, colorSetReadOnlyData);
			return nullptr;
		}
	}

	// Color Set Number of Columns
	const char * colorSetNumberOfColumnsData = colorSetElement->Attribute(KRITA_PALETTE_COLOR_SET_ELEMENT_COLUMNS_ATTRIBUTE_NAME.c_str());

	if(Utilities::isEmptyString(colorSetNumberOfColumnsData)) {
		spdlog::error("Krita palette color set is missing '{}' attribute.", KRITA_PALETTE_COLOR_SET_ELEMENT_COLUMNS_ATTRIBUTE_NAME);
		return nullptr;
	}

	const uint8_t numberOfColumns = Utilities::parseUnsignedByte(colorSetNumberOfColumnsData, &error);

	if(error) {
		spdlog::error("Krita palette color set has invalid '{}' attribute value: '{}', expected integer between 1 and 255.", KRITA_PALETTE_COLOR_SET_ELEMENT_COLUMNS_ATTRIBUTE_NAME, colorSetNumberOfColumnsData);
		return nullptr;
	}

	// Color Set Entries
	const tinyxml2::XMLElement * colorSetEntryElement = colorSetElement->FirstChildElement();

	if(colorSetEntryElement == nullptr) {
		spdlog::error("Krita palette color set has no children.");
		return nullptr;
	}

	const tinyxml2::XMLElement * colorSetEntryRGBElement = nullptr;
	const tinyxml2::XMLElement * colorSetEntryPositionElement = nullptr;
	size_t colourSetEntryNumber = 0;
	const char * colorSetEntryBitDepthData = nullptr;
	std::optional<BitDepth> optionalDetectedBitDepth;
	std::optional<BitDepth> optionalBitDepth;
	BitDepth bitDepth = DEFAULT_BIT_DEPTH;
	const char * colorSetEntryRGBComponentData = nullptr;
	Colour colour;
	const char * colorSetEntryPositionComponentData = nullptr;
	Point2D position;
	bool anyColourFound = false;
	std::array<std::optional<Colour>, 256> orderedColours;
	std::vector<Colour> colours;
	colours.reserve(256);

	while(true) {
		if(colorSetEntryElement == nullptr) {
			break;
		}

		if(Utilities::areStringsEqualIgnoreCase(colorSetEntryElement->Name(), KRITA_PALETTE_GROUP_ELEMENT_NAME)) {
			spdlog::error("Krita palette color set groups are not supported.");
			return nullptr;
		}

		if(!Utilities::areStringsEqualIgnoreCase(colorSetEntryElement->Name(), KRITA_PALETTE_COLOR_SET_ENTRY_ELEMENT_NAME)) {
			spdlog::error("Unexpected Krita palette color set child element with name: '{}', expected '{}'.", colorSetEntryElement->Name(), KRITA_PALETTE_COLOR_SET_ENTRY_ELEMENT_NAME);
			return nullptr;
		}

		colourSetEntryNumber++;

		// Color Set Entry Bit Depth
		colorSetEntryBitDepthData = colorSetEntryElement->Attribute(KRITA_PALETTE_COLOR_SET_ENTRY_BIT_DEPTH_ATTRIBUTE_NAME.c_str());

		if(Utilities::isEmptyString(colorSetEntryBitDepthData)) {
			spdlog::error("Krita palette color set entry #{} is missing '{}' attribute.", colourSetEntryNumber, KRITA_PALETTE_COLOR_SET_ENTRY_BIT_DEPTH_ATTRIBUTE_NAME);
			return nullptr;
		}

		optionalBitDepth = magic_enum::enum_cast<BitDepth>(colorSetEntryBitDepthData);

		if(!optionalBitDepth.has_value()) {
			constexpr auto bitDepths = magic_enum::enum_values<BitDepth>();
			std::stringstream validBitDepthNamesStream;

			for(size_t i = 0; i < magic_enum::enum_count<BitDepth>(); i++) {
				if(validBitDepthNamesStream.tellp() != 0) {
					validBitDepthNamesStream << ", ";
				}

				if(i == magic_enum::enum_count<BitDepth>() - 1) {
					validBitDepthNamesStream << "or ";
				}

				validBitDepthNamesStream << magic_enum::enum_name(bitDepths[i]);
			}

			spdlog::error("Krita palette color set entry #{} element attribute '{}' has invalid value: '{}', expected one of {}.", colourSetEntryNumber, KRITA_PALETTE_COLOR_SET_ENTRY_BIT_DEPTH_ATTRIBUTE_NAME, colorSetEntryBitDepthData, validBitDepthNamesStream.str());
			return nullptr;
		}

		bitDepth = optionalBitDepth.value();

		if(!optionalDetectedBitDepth.has_value()) {
			spdlog::debug("Detected Krita palette color set '{}' bit depth.", magic_enum::enum_name(bitDepth));

			optionalDetectedBitDepth = bitDepth;
		}

		// Color Set Entry Children
		colorSetEntryRGBElement = Utilities::findFirstXMLElementWithName(colorSetEntryElement, KRITA_PALETTE_RGB_ELEMENT_NAME);

		if(colorSetEntryRGBElement == nullptr) {
			spdlog::error("Krita palette color set entry #{} is missing '{}' child element.", colourSetEntryNumber, KRITA_PALETTE_RGB_ELEMENT_NAME);
			return nullptr;
		}

		colorSetEntryPositionElement = Utilities::findFirstXMLElementWithName(colorSetEntryElement, KRITA_PALETTE_POSITION_ELEMENT_NAME);

		if(colorSetEntryPositionElement == nullptr) {
			spdlog::error("Krita palette color set entry #{} is missing '{}' child element.", colourSetEntryNumber, KRITA_PALETTE_POSITION_ELEMENT_NAME);
			return nullptr;
		}

		// Color Set Entry RGB Element
		for(size_t i = 0; i < 3; i++) {
			const std::string & rgbAttributeName = *KRITA_PALETTE_RGB_ATTRIBUTE_NAMES[i];
			colorSetEntryRGBComponentData = colorSetEntryRGBElement->Attribute(rgbAttributeName.c_str());

			if(Utilities::isEmptyString(colorSetEntryRGBComponentData)) {
				spdlog::error("Krita palette color set entry #{} is missing '{}' attribute '{}' value.", colourSetEntryNumber, KRITA_PALETTE_RGB_ELEMENT_NAME, rgbAttributeName);
				return nullptr;
			}

			switch(bitDepth) {
				case BitDepth::U8: {
					colour.c[i] = Utilities::parseUnsignedByte(colorSetEntryRGBComponentData, &error);
					break;
				}
				case BitDepth::U16: {
					colour.c[i] = static_cast<uint8_t>(std::round((static_cast<double>(Utilities::parseUnsignedShort(colorSetEntryRGBComponentData, &error)) * 255.0) / 65535.0));
					break;
				}
				case BitDepth::F16:
				case BitDepth::F32: {
					colour.c[i] = static_cast<uint8_t>(std::round(std::clamp(Utilities::parseFloat(colorSetEntryRGBComponentData, &error), 0.0f, 1.0f) * 255.0f));
					break;
				}
			}

			if(error) {
				spdlog::error("Krita palette color set entry #{} has invalid '{}' attribute '{}' '{}' value: '{}'.", colourSetEntryNumber, KRITA_PALETTE_RGB_ELEMENT_NAME, rgbAttributeName, magic_enum::enum_name(bitDepth), colorSetEntryRGBComponentData);
				return nullptr;
			}
		}

		// Color Set Entry Position Element
		for(size_t i = 0; i < 2; i++) {
			const std::string & positionAttributeName = *KRITA_PALETTE_POSITION_ATTRIBUTE_NAMES[i];
			colorSetEntryPositionComponentData = colorSetEntryPositionElement->Attribute(positionAttributeName.c_str());

			if(Utilities::isEmptyString(colorSetEntryPositionComponentData)) {
				spdlog::error("Krita palette color set entry #{} is missing '{}' attribute '{}' value.", colourSetEntryNumber, KRITA_PALETTE_POSITION_ELEMENT_NAME, positionAttributeName);
				return nullptr;
			}

			position.p[i] = static_cast<int32_t>(Utilities::parseUnsignedInteger(colorSetEntryPositionComponentData, &error));

			if(error) {
				spdlog::error("Krita palette color set entry #{} has invalid '{}' attribute '{}' value: '{}'.", colourSetEntryNumber, KRITA_PALETTE_POSITION_ELEMENT_NAME, positionAttributeName, colorSetEntryPositionComponentData);
				return nullptr;
			}
		}

		orderedColours[position.x + (position.y * numberOfColumns)] = colour;
		anyColourFound = true;

		colorSetEntryElement = colorSetEntryElement->NextSiblingElement();
	}

	if(!anyColourFound) {
		spdlog::error("No colours found in Krita palette data.");
		return nullptr;
	}

	for(size_t i = 0; i < orderedColours.size(); i++) {
		const std::optional<Colour> & optionalColour = orderedColours[i];

		if(!optionalColour.has_value()) {
			break;
		}

		colours.emplace_back(optionalColour.value());
	}

	return std::make_unique<PaletteKPL>(std::make_unique<ColourTable>(std::move(colours), std::optional<uint8_t>(), false, colorSetName), colorSetComment, numberOfColumns, readOnly, std::move(archive));
}

std::unique_ptr<PaletteKPL> PaletteKPL::loadFrom(const std::string & filePath) {
	std::unique_ptr<ByteBuffer> paletteData(ByteBuffer::readFrom(filePath));

	if(paletteData == nullptr) {
		spdlog::error("Failed to read Krita palette data from file: '{}'.", filePath);
		return nullptr;
	}

	std::unique_ptr<PaletteKPL> palette(PaletteKPL::readFrom(*paletteData));

	if(palette == nullptr) {
		spdlog::error("Failed to parse Krita palette data from file contents: '{}'.", filePath);
		return nullptr;
	}

	return palette;
}

bool PaletteKPL::writeTo(ByteBuffer & byteBuffer) const {
	if(!updateColorSet()) {
		return false;
	}

	const ByteBuffer * archiveData = m_archive->getData();

	if(archiveData == nullptr) {
		return false;
	}

	return byteBuffer.writeBytes(*archiveData);
}

bool PaletteKPL::updateColorSet() const {
	tinyxml2::XMLDocument document;

	tinyxml2::XMLElement * colorSetElement = document.NewElement(KRITA_PALETTE_COLOR_SET_ELEMENT_NAME.c_str());

	if(m_colourTable->hasName()) {
		colorSetElement->SetAttribute(KRITA_PALETTE_COLOR_SET_ELEMENT_NAME_ATTRIBUTE_NAME.c_str(), m_colourTable->getName().c_str());
	}

	if(!m_comment.empty()) {
		colorSetElement->SetAttribute(KRITA_PALETTE_COLOR_SET_ELEMENT_COMMENT_ATTRIBUTE_NAME.c_str(), m_comment.c_str());
	}

	colorSetElement->SetAttribute(KRITA_PALETTE_COLOR_SET_ELEMENT_VERSION_ATTRIBUTE_NAME.c_str(), COLOR_SET_VERSION.c_str());
	colorSetElement->SetAttribute(KRITA_PALETTE_COLOR_SET_ELEMENT_READ_ONLY_ATTRIBUTE_NAME.c_str(), m_readOnly ? "true" : "false");
	colorSetElement->SetAttribute(KRITA_PALETTE_COLOR_SET_ELEMENT_COLUMNS_ATTRIBUTE_NAME.c_str(), m_numberOfColumns);

	const std::vector<Colour> & colours = m_colourTable->getColours();
	tinyxml2::XMLElement * colorSetEntryElement = nullptr;
	tinyxml2::XMLElement * colorSetEntryRGBElement = nullptr;
	tinyxml2::XMLElement * colorSetEntryPositionElement = nullptr;

	for(size_t i = 0; i < colours.size(); i++) {
		const Colour & colour = colours[i];

		colorSetEntryElement = document.NewElement(KRITA_PALETTE_COLOR_SET_ENTRY_ELEMENT_NAME.c_str());
		colorSetEntryElement->SetAttribute(KRITA_PALETTE_COLOR_SET_ENTRY_NAME_ATTRIBUTE_NAME.c_str(), fmt::format("#{}", i).c_str());
		colorSetEntryElement->SetAttribute(KRITA_PALETTE_COLOR_SET_ENTRY_ID_ATTRIBUTE_NAME.c_str(), i + 1);
		colorSetEntryElement->SetAttribute(KRITA_PALETTE_COLOR_SET_ENTRY_BIT_DEPTH_ATTRIBUTE_NAME.c_str(), std::string(magic_enum::enum_name(BitDepth::U8)).c_str());
		colorSetEntryElement->SetAttribute(KRITA_PALETTE_COLOR_SET_ENTRY_SPOT_ATTRIBUTE_NAME.c_str(), "false");

		colorSetEntryRGBElement = document.NewElement(KRITA_PALETTE_RGB_ELEMENT_NAME.c_str());

		for(size_t j = 0; j < 3; j++) {
			const std::string & rgbAttributeName = *KRITA_PALETTE_RGB_ATTRIBUTE_NAMES[j];

			colorSetEntryRGBElement->SetAttribute(rgbAttributeName.c_str(), colour.c[j]);
		}

		colorSetEntryElement->InsertEndChild(colorSetEntryRGBElement);

		colorSetEntryPositionElement = document.NewElement(KRITA_PALETTE_POSITION_ELEMENT_NAME.c_str());
		colorSetEntryPositionElement->SetAttribute(KRITA_PALETTE_POSITION_ROW_ATTRIBUTE_NAME.c_str(), fmt::format("{}", i / m_numberOfColumns).c_str());
		colorSetEntryPositionElement->SetAttribute(KRITA_PALETTE_POSITION_COLUMN_ATTRIBUTE_NAME.c_str(), fmt::format("{}", i % m_numberOfColumns).c_str());

		colorSetEntryElement->InsertEndChild(colorSetEntryPositionElement);

		colorSetElement->InsertEndChild(colorSetEntryElement);
	}

	document.InsertEndChild(colorSetElement);

	std::unique_ptr<ByteBuffer> colorSetData(std::make_unique<ByteBuffer>());
	colorSetData->writeString(Utilities::documentToString(&document, true));

	if(m_archive->addData(std::move(colorSetData), KRITAL_PALETTE_COLOR_SET_FILE_NAME, true) == nullptr) {
		spdlog::error("Failed to add Krita colour set XML file to archive.");
		return false;
	}

	if(!m_archive->save()) {
		spdlog::error("Failed to update Krita Zip archive buffer.");
		return false;
	}

	return true;
}

Endianness PaletteKPL::getEndianness() const {
	return {};
}

size_t PaletteKPL::getSizeInBytes() const {
	if(m_modified) {
		updateColorSet();
	}

	return m_archive->getCompressedSize();
}

void PaletteKPL::ensureRequiredFiles() const {
	const std::shared_ptr<ArchiveEntry> mimeTypeFile(m_archive->getEntry(KRITAL_PALETTE_MIME_TYPE_FILE_NAME));

	if(mimeTypeFile == nullptr) {
		std::unique_ptr<ByteBuffer> mimeTypeData(std::make_unique<ByteBuffer>());
		mimeTypeData->reserve(MIME_TYPE.length());
		mimeTypeData->writeString(MIME_TYPE);

		m_archive->addData(std::move(mimeTypeData), KRITAL_PALETTE_MIME_TYPE_FILE_NAME);
	}

	updateColorSet();
}

void PaletteKPL::onColourTableModified(ColourTable & colourTable) {
	if(colourTable.isModified()) {
		setModified(true);
	}
}

void PaletteKPL::connectSignals() {
	m_colourTableModifiedConnection = m_colourTable->modified.connect(std::bind(&PaletteKPL::onColourTableModified, this, std::placeholders::_1));
}

void PaletteKPL::updateParent() {
	m_colourTable->setParent(this);
}

bool PaletteKPL::operator == (const PaletteKPL & palette) const {
	if(this == &palette) {
		return true;
	}

	return Utilities::areStringsEqual(m_comment, palette.m_comment) &&
		   m_numberOfColumns == palette.m_numberOfColumns &&
		   m_readOnly == palette.m_readOnly &&
		   *m_colourTable == *palette.m_colourTable;
}

bool PaletteKPL::operator != (const PaletteKPL & palette) const {
	return !operator == (palette);
}
