#include "PaletteCSS.h"

#include <ByteBuffer.h>
#include <Utilities/NumberUtilities.h>
#include <Utilities/StringUtilities.h>

#include <CSSColorParser/css_color.hpp>
#include <fmt/core.h>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <limits>
#include <regex>

PaletteCSS::PaletteCSS(Format format, std::string_view comment, const std::string & filePath)
	: Palette(filePath)
	, m_format(format)
	, m_comment(comment)
	, m_colourTable(std::make_shared<ColourTable>()) {
	updateParent();
}

PaletteCSS::PaletteCSS(std::unique_ptr<ColourTable> colourTable, Format format, std::string_view comment, const std::string & filePath)
	: Palette(filePath)
	, m_format(format)
	, m_comment(comment)
	, m_colourTable(colourTable != nullptr ? std::move(colourTable) : std::make_shared<ColourTable>()) {
	updateParent();
}

PaletteCSS::PaletteCSS(PaletteCSS && palette) noexcept
	: Palette(std::move(palette))
	, m_format(palette.m_format)
	, m_comment(std::move(palette.m_comment))
	, m_colourTable(std::move(palette.m_colourTable)) {
	updateParent();
}

PaletteCSS::PaletteCSS(const PaletteCSS & palette)
	: Palette(palette)
	, m_format(palette.m_format)
	, m_comment(palette.m_comment)
	, m_colourTable(palette.m_colourTable) {
	updateParent();
}

PaletteCSS & PaletteCSS::operator = (PaletteCSS && palette) noexcept {
	if(this != &palette) {
		Palette::operator = (std::move(palette));

		m_format = palette.m_format;
		m_comment = std::move(palette.m_comment);
		m_colourTable = std::move(palette.m_colourTable);

		updateParent();
	}

	return *this;
}

PaletteCSS & PaletteCSS::operator = (const PaletteCSS & palette) {
	Palette::operator = (palette);

	m_format = palette.m_format;
	m_comment = palette.m_comment;
	m_colourTable = palette.m_colourTable;

	updateParent();

	return *this;
}

PaletteCSS::~PaletteCSS() { }

PaletteCSS::Format PaletteCSS::getFormat() const {
	return m_format;
}

void PaletteCSS::setFormat(Format format) {
	m_format = format;
}

bool PaletteCSS::hasComment() const {
	return m_comment.empty();
}

const std::string & PaletteCSS::getComment() const {
	return m_comment;
}

void PaletteCSS::setComment(std::string_view comment) {
	m_comment = comment;
}

void PaletteCSS::clearComment() {
	m_comment.clear();
}

std::shared_ptr<ColourTable> PaletteCSS::getColourTable(uint8_t colourTableIndex) const {
	if(colourTableIndex != 0) {
		return nullptr;
	}

	return m_colourTable;
}

std::unique_ptr<PaletteCSS> PaletteCSS::readFrom(const ByteBuffer & byteBuffer) {
	static const std::regex COMMENT_REGEX(R"(^\s*/\*\s*(.*?)\s*\*/\s*$)");
	static const std::regex COLOUR_STYLE_REGEX(R"(^\.(\d+)\s*\{\s*color:\s*(.*?)\s*;\s*\}$)");

	size_t lineNumber = 0;
	std::string line;
	bool error = false;
	std::smatch match;
	std::string comment;
	std::string parseData;
	std::from_chars_result parseResult;
	uint8_t colourIndex = std::numeric_limits<size_t>::max();
	css_colors::color colour;
	bool anyColourFound = false;
	std::array<std::optional<Colour>, 256> orderedColours;
	std::optional<Format> optionalFormat;
	std::vector<Colour> colours;
	colours.reserve(256);

	while(true) {
		line = byteBuffer.readLine(&error);
		lineNumber++;

		if(error || line.empty()) {
			break;
		}

		if(std::regex_match(line, match, COMMENT_REGEX)) {
			comment = match[1].str();
			continue;
		}

		if(!std::regex_match(line, match, COLOUR_STYLE_REGEX)) {
			continue;
		}

		parseData = match[1].str();
		parseResult = std::from_chars(parseData.data(), parseData.data() + parseData.length(), colourIndex, 10);

		if(parseResult.ec != std::errc{} || parseResult.ptr != parseData.data() + parseData.length()) {
			spdlog::error("Invalid colour index #{} on line #{}, expected class number between 0 and 255.", parseData, lineNumber);
			return nullptr;
		}

		parseData = match[2].str();

		if(!optionalFormat.has_value()) {
			if(parseData.find_first_of("#") != std::string::npos) {
				optionalFormat = Format::Hexadecimal;
			}
			else if(Utilities::contains(parseData, "rgb")) {
				optionalFormat = Format::RGB;
			}

			if(optionalFormat.has_value()) {
				spdlog::debug("Detected '{}' CSS palette data format.", magic_enum::enum_name(optionalFormat.value()));
			}
		}

		colour = css_colors::parse(parseData.data());

		if(!colour) {
			spdlog::error("Invalid colour #{} on line #{} has an invalid colour value: '{}'.", colourIndex, lineNumber, match[2].str());
			return nullptr;
		}

		auto formattedColour = colour.as<css_colors::colorspaces::srgb>();

		orderedColours[colourIndex] = Colour(static_cast<uint8_t>(formattedColour.second.first[0]), static_cast<uint8_t>(formattedColour.second.first[1]), static_cast<uint8_t>(formattedColour.second.first[2]), static_cast<uint8_t>(formattedColour.second.second * 255));
		anyColourFound = true;
	}

	if(!anyColourFound) {
		spdlog::error("No colours found in CSS palette data.");
		return nullptr;
	}

	for(size_t i = 0; i < orderedColours.size(); i++) {
		const std::optional<Colour> & optionalColour = orderedColours[i];

		if(!optionalColour.has_value()) {
			break;
		}

		colours.emplace_back(optionalColour.value());
	}

	return std::make_unique<PaletteCSS>(std::make_unique<ColourTable>(std::move(colours)), optionalFormat.value_or(DEFAULT_FORMAT), comment);
}

std::unique_ptr<PaletteCSS> PaletteCSS::loadFrom(const std::string & filePath) {
	std::unique_ptr<ByteBuffer> paletteData(ByteBuffer::readFrom(filePath));

	if(paletteData == nullptr) {
		spdlog::error("Failed to read CSS palette data from file: '{}'.", filePath);
		return nullptr;
	}

	std::unique_ptr<PaletteCSS> palette(PaletteCSS::readFrom(*paletteData));

	if(palette == nullptr) {
		spdlog::error("Failed to parse CSS palette data from file contents: '{}'.", filePath);
		return nullptr;
	}

	return palette;
}

bool PaletteCSS::writeTo(ByteBuffer & byteBuffer) const {
	if(!m_comment.empty()) {
		byteBuffer.writeLine(fmt::format("/* {} */", m_comment));
	}

	const std::vector<Colour> & colours = m_colourTable->getColours();

	for(size_t i = 0; i < colours.size(); i++) {
		const Colour & colour = colours[i];

		switch(m_format) {
			case Format::RGB:
			{
				if(!byteBuffer.writeLine(fmt::format(".{} {{ color: rgb({}, {}, {}); }}", i, colour.r, colour.g, colour.b))) {
					return false;
				}
				break;
			}
			case Format::Hexadecimal: {
				if(!byteBuffer.writeLine(fmt::format(".{} {{ color: #{}{}{}; }}", i, Utilities::toHexadecimal(colour.r), Utilities::toHexadecimal(colour.g), Utilities::toHexadecimal(colour.b)))) {
					return false;
				}
				break;
			}
		}
	}

	return true;
}

Endianness PaletteCSS::getEndianness() const {
	return {};
}

size_t PaletteCSS::getSizeInBytes() const {
	size_t paletteSize = 0u;

	if(!m_comment.empty()) {
		paletteSize += m_comment.length() + 7;
	}

	const std::vector<Colour> & colours = m_colourTable->getColours();

	for(size_t i = 0; i < colours.size(); i++) {
		const Colour & colour = colours[i];

		switch(m_format) {
			case Format::RGB:
			{
				paletteSize += 24u + Utilities::unsignedShortLength(static_cast<uint16_t>(i)) + Utilities::unsignedByteLength(colour.r) + Utilities::unsignedByteLength(colour.g) + Utilities::unsignedByteLength(colour.b);
				break;
			}
			case Format::Hexadecimal: {
				paletteSize += 21u + Utilities::unsignedShortLength(static_cast<uint16_t>(i));
				break;
			}
		}
	}

	return paletteSize;
}

void PaletteCSS::updateParent() {
	m_colourTable->setParent(this);
}

bool PaletteCSS::operator == (const PaletteCSS & palette) const {
	if(this == &palette) {
		return true;
	}

	return m_format == palette.m_format &&
		   Utilities::areStringsEqual(m_comment, palette.m_comment) &&
		   *m_colourTable == *palette.m_colourTable;
}

bool PaletteCSS::operator != (const PaletteCSS & palette) const {
	return !operator == (palette);
}
