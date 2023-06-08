#ifndef _PALETTE_GPL_H_
#define _PALETTE_GPL_H_

#include "../Palette.h"

#include <string>

class PaletteGPL final : public Palette {
public:
	PaletteGPL(const std::string & filePath = {});
	PaletteGPL(std::unique_ptr<ColourTable> colourTable, std::vector<std::string> && colourNames = {}, std::optional<uint8_t> columnCount = {}, std::vector<std::string> && comments = {}, const std::string & filePath = {});
	PaletteGPL(const ColourTable & colourTable, const std::vector<std::string> & colourNames = {}, std::optional<uint8_t> columnCount = {}, const std::vector<std::string> & comments = {}, const std::string & filePath = {});
	PaletteGPL(PaletteGPL && palette) noexcept;
	PaletteGPL(const PaletteGPL & palette);
	PaletteGPL & operator = (PaletteGPL && palette) noexcept;
	PaletteGPL & operator = (const PaletteGPL & palette);
	virtual ~PaletteGPL();

	virtual std::shared_ptr<ColourTable> getColourTable(uint8_t colourTableIndex = 0) const override;
	size_t numberOfNamedColours() const;
	bool hasColourName(const std::string & colourName) const;
	size_t indexOfColourName(const std::string & colourName) const;
	const std::string & getColourName(size_t index) const;
	const std::vector<std::string> & getColourNames() const;
	bool setColourName(size_t index, const std::string & colourName);
	bool clearColourName(size_t index);
	void clearColourNames();
	bool hasNumberOfColumns() const;
	const std::optional<uint8_t> numberOfColumns() const;
	void setNumberOfColumns(uint8_t columnCount);
	void clearNumberOfColumns();
	bool hasComments() const;
	size_t numberOfComments() const;
	bool hasComment(const std::string & comment) const;
	size_t indexOfComment(const std::string & comment) const;
	const std::string & getComment(size_t index) const;
	const std::vector<std::string> & getComments() const;
	void addComment(const std::string & comment);
	void addComments(const std::vector<std::string> & comments);
	void setComments(const std::vector<std::string> & comments);
	void setComments(std::vector<std::string> && comments);
	bool insertComment(size_t index, const std::string & comment);
	bool replaceComment(size_t index, const std::string & newComment);
	bool replaceComment(const std::string & comment, const std::string & newComment);
	bool removeComment(size_t index);
	bool removeComment(const std::string & comment);
	void clearComments();
	static std::unique_ptr<PaletteGPL> readFrom(const ByteBuffer & byteBuffer);
	static std::unique_ptr<PaletteGPL> loadFrom(const std::string & filePath);
	virtual bool writeTo(ByteBuffer & byteBuffer) const override;
	virtual void addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const override;
	virtual Endianness getEndianness() const override;
	virtual size_t getSizeInBytes() const override;

	bool operator == (const PaletteGPL & palette) const;
	bool operator != (const PaletteGPL & palette) const;

	static inline const std::string MAGIC = "GIMP Palette";
	static constexpr size_t MAGIC_SIZE_BYTES = 12;
	static inline const std::string NAME_KEY = "Name";
	static inline const std::string NUMBER_OF_COLUMNS_KEY = "Columns";
	static constexpr char COMMENT_CHARACTER = '#';
	static constexpr uint8_t COMMENT_PADDING = 7;

private:
	void updateParent();

	std::shared_ptr<ColourTable> m_colourTable;
	std::vector<std::string> m_colourNames;
	std::vector<std::string> m_comments;
	std::optional<uint8_t> m_numberOfColumns;
};

#endif // _PALETTE_GPL_H_
