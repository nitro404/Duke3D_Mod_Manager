#ifndef _COLOUR_TABLE_H_
#define _COLOUR_TABLE_H_

#include <Colour.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
#include <string>

class GameFile;

class ColourTable final {
public:
	ColourTable(uint16_t numberOfColours = 0, std::optional<uint8_t> transparentColourIndex = {}, bool alpha = false, GameFile * parent = nullptr);
	ColourTable(const Colour & fillColour, uint16_t numberOfColours = NUMBER_OF_COLOURS, std::optional<uint8_t> transparentColourIndex = {}, bool alpha = false, GameFile * parent = nullptr);
	ColourTable(std::vector<Colour> && colours, std::optional<uint8_t> transparentColourIndex = {}, bool alpha = false, GameFile * parent = nullptr);
	ColourTable(const std::vector<Colour> & colours, std::optional<uint8_t> transparentColourIndex = {}, bool alpha = false, GameFile * parent = nullptr);
	ColourTable(ColourTable && colourTable) noexcept;
	ColourTable(const ColourTable & colourTable);
	ColourTable & operator = (ColourTable && colourTable) noexcept;
	ColourTable & operator = (const ColourTable & colourTable);
	~ColourTable();

	uint16_t numberOfColours() const;
	const Colour & getColour(uint8_t colourIndex, bool * error = nullptr) const;
	const std::vector<Colour> & getColours() const;
	bool setColour(uint8_t colourIndex, const Colour & colour);
	bool setColour(uint8_t colourIndex, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
	bool setColours(const ColourTable & colourTable);
	void fillWithColour(const Colour & colour);
	bool setNumberOfColours(uint16_t colourCount, const Colour & fillColour = Colour::INVISIBLE);
	bool hasTransparentColourIndex() const;
	const std::optional<uint8_t> & getTransparentColourIndex() const;
	void setTransparentColourIndex(uint8_t transparentColourIndex);
	void clearTransparentColourIndex();
	bool hasAlphaChannel() const;
	void setHasAlphaChannel(bool alpha);
	const std::string & getName() const;
	void setName(const std::string & name);
	void clearName();
	bool isParentValid() const;
	GameFile * getParent() const;
	void setParent(GameFile * gameFile);
	void clearParent();

	static std::unique_ptr<ColourTable> getFrom(const ByteBuffer & byteBuffer, size_t offset, uint16_t numberOfColours = NUMBER_OF_COLOURS, bool alpha = false, Colour::ByteOrder = Colour::DEFAULT_BYTE_ORDER);
	static std::unique_ptr<ColourTable> readFrom(const ByteBuffer & byteBuffer, uint16_t numberOfColours = NUMBER_OF_COLOURS, bool alpha = false, Colour::ByteOrder = Colour::DEFAULT_BYTE_ORDER);
	bool putIn(ByteBuffer & byteBuffer, size_t offset, const std::optional<Colour> & paddingColour = {}, Colour::ByteOrder = Colour::DEFAULT_BYTE_ORDER) const;
	bool putIn(ByteBuffer & byteBuffer, size_t offset, bool alpha, const std::optional<Colour> & paddingColour = {}, Colour::ByteOrder = Colour::DEFAULT_BYTE_ORDER) const;
	bool insertIn(ByteBuffer & byteBuffer, size_t offset, const std::optional<Colour> & paddingColour = {}, Colour::ByteOrder = Colour::DEFAULT_BYTE_ORDER) const;
	bool insertIn(ByteBuffer & byteBuffer, size_t offset, bool alpha, const std::optional<Colour> & paddingColour = {}, Colour::ByteOrder = Colour::DEFAULT_BYTE_ORDER) const;
	bool writeTo(ByteBuffer & byteBuffer, const std::optional<Colour> & paddingColour = {}, Colour::ByteOrder = Colour::DEFAULT_BYTE_ORDER) const;
	bool writeTo(ByteBuffer & byteBuffer, bool alpha, const std::optional<Colour> & paddingColour = {}, Colour::ByteOrder = Colour::DEFAULT_BYTE_ORDER) const;

	bool isValid() const;
	static bool isValid(const ColourTable * colourTable);

	const Colour & operator[] (size_t colourIndex) const;

	bool operator == (const ColourTable & colourTable) const;
	bool operator != (const ColourTable & colourTable) const;

	static constexpr uint16_t NUMBER_OF_COLOURS = 256;
	static const ColourTable EMPTY_COLOUR_TABLE;

private:
	std::vector<Colour> m_colours;
	std::optional<uint8_t> m_transparentColourIndex;
	bool m_alpha;
	std::string m_name;
	GameFile * m_parent;
};

#endif // _COLOUR_TABLE_H_
