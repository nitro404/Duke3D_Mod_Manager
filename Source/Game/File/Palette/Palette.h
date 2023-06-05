#ifndef _PALETTE_H_
#define _PALETTE_H_

#include "../GameFile.h"

#include <Colour.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

class Palette : public GameFile {
public:
	class ColourTable final {
	friend class Palette;

	public:
		ColourTable(uint16_t numberOfColours = NUMBER_OF_COLOURS, std::optional<uint8_t> transparentColourIndex = {}, bool alpha = false, Palette * parent = nullptr);
		ColourTable(const Colour & fillColour, uint16_t numberOfColours = NUMBER_OF_COLOURS, std::optional<uint8_t> transparentColourIndex = {}, bool alpha = false, Palette * parent = nullptr);
		ColourTable(std::vector<Colour> && colours, std::optional<uint8_t> transparentColourIndex = {}, bool alpha = false, Palette * parent = nullptr);
		ColourTable(const std::vector<Colour> & colours, std::optional<uint8_t> transparentColourIndex = {}, bool alpha = false, Palette * parent = nullptr);
		ColourTable(ColourTable && colourTable) noexcept;
		ColourTable(const ColourTable & colourTable);
		ColourTable & operator = (ColourTable && colourTable) noexcept;
		ColourTable & operator = (const ColourTable & colourTable);
		~ColourTable();

		uint16_t numberOfColours() const;
		const Colour & getColour(uint8_t colourIndex, bool * error = nullptr) const;
		const std::vector<Colour> & getColours() const;
		bool setColour(uint8_t colourIndex, const Colour & colour);
		bool setColours(const ColourTable & colourTable);
		void fillWithColour(const Colour & colour);
		bool setNumberOfColours(uint16_t colourCount, const Colour & fillColour = Colour::INVISIBLE);
		bool hasTransparentColourIndex() const;
		const std::optional<uint8_t> & getTransparentColourIndex() const;
		void setTransparentColourIndex(uint8_t transparentColourIndex);
		void clearTransparentColourIndex();
		bool hasAlphaChannel() const;
		void setHasAlphaChannel(bool alpha);
		const std::string & getDescription() const;
		void setDescription(const std::string & description);
		void clearDescription();
		bool isParentValid() const;
		Palette * getParent() const;
		void setParent(Palette * palette);
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

		static const ColourTable EMPTY_COLOUR_TABLE;

	private:
		std::vector<Colour> m_colours;
		std::optional<uint8_t> m_transparentColourIndex;
		bool m_alpha;
		std::string m_description;
		Palette * m_parent;
	};

	Palette(Palette && palette) noexcept;
	Palette(const Palette & palette);
	Palette & operator = (Palette && palette) noexcept;
	Palette & operator = (const Palette & palette);
	virtual ~Palette();

	bool hasTransparentColourIndex(uint8_t colourTableIndex = 0) const;
	std::optional<uint8_t> getTransparentColourIndex(uint8_t colourTableIndex = 0) const;
	bool setTransparentColourIndex(uint8_t transparentColourIndex, uint8_t colourTableIndex = 0) const;
	bool clearTransparentColourIndex(uint8_t colourTableIndex = 0) const;
	std::optional<uint16_t> numberOfColours(uint8_t colourTableIndex = 0) const;
	bool setNumberOfColours(uint16_t colourCount, uint8_t colourTableIndex = 0) const;
	const Colour & getColour(uint8_t colourIndex, uint8_t colourTableIndex = 0, bool * error = nullptr) const;
	std::optional<Colour> getColour(uint8_t colourIndex, uint8_t colourTableIndex = 0) const;
	const Colour & getColour(uint8_t x, uint8_t y, uint8_t colourTableIndex = 0, bool * error = nullptr) const;
	std::optional<Colour> getColour(uint8_t x, uint8_t y, uint8_t colourTableIndex = 0) const;
	bool setColour(uint8_t x, uint8_t y, const Colour & colour, uint8_t colourTableIndex = 0);
	bool setColour(uint8_t colourIndex, const Colour & colour, uint8_t colourTableIndex = 0);
	virtual std::shared_ptr<ColourTable> getColourTable(uint8_t colourTableIndex = 0) const = 0;
	std::vector<std::shared_ptr<ColourTable>> getAllColourTables() const;
	bool updateColourTable(uint8_t colourTableIndex, const ColourTable & colourTable);
	bool updateAllColourTables(const std::vector<std::shared_ptr<const ColourTable>> & colourTables);
	bool fillColourTableWithColour(const Colour & colour, uint8_t colourTableIndex = 0);
	bool fillAllColourTablesWithColour(const Colour & colour);
	virtual uint8_t numberOfColourTables() const;
	const std::string & getColourTableDescription(uint8_t colourTableIndex) const;
	virtual void addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const override;
	virtual bool isValid(bool verifyParent = true) const override;

	static constexpr uint8_t PALETTE_WIDTH = 16;
	static constexpr uint8_t PALETTE_HEIGHT = 16;
	static constexpr uint16_t NUMBER_OF_COLOURS = PALETTE_WIDTH * PALETTE_HEIGHT;

protected:
	Palette(const std::string & filePath = {});
};

#endif // _PALETTE_H_
