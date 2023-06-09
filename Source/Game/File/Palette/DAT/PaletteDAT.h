#ifndef _PALETTE_DAT_H_
#define _PALETTE_DAT_H_

#include "../Palette.h"

#include <array>

class PaletteDAT final : public Palette {
public:
	static constexpr uint32_t TRANSLUCENCY_TABLE_SIZE_BYTES = NUMBER_OF_COLOURS * NUMBER_OF_COLOURS;

	enum class Type {
		Palette,
		Lookup
	};

	class ShadeTable final {
		friend class PaletteDAT;

	public:
		using ShadeData = std::array<uint8_t, NUMBER_OF_COLOURS>;

		ShadeTable(Palette * parent = nullptr);
		ShadeTable(std::unique_ptr<ShadeData> shadeData, Palette * parent = nullptr);
		ShadeTable(const ShadeData & shadeData, Palette * parent = nullptr);
		ShadeTable(ShadeTable && shadeTable) noexcept;
		ShadeTable(const ShadeTable & shadeTable);
		ShadeTable & operator = (ShadeTable && shadeTable) noexcept;
		ShadeTable & operator = (const ShadeTable & shadeTable);
		~ShadeTable();

		const ShadeData & getData() const;
		void setData(std::unique_ptr<ShadeData> shadeData);
		void setData(const ShadeData & shadeData);
		bool setData(const std::vector<uint8_t> & shadeData);

		static std::unique_ptr<ShadeTable> getFrom(const ByteBuffer & byteBuffer, size_t offset);
		static std::unique_ptr<ShadeTable> readFrom(const ByteBuffer & byteBuffer);
		bool putIn(ByteBuffer & byteBuffer, size_t offset) const;
		bool insertIn(ByteBuffer & byteBuffer, size_t offset) const;
		bool writeTo(ByteBuffer & byteBuffer) const;

		bool isParentValid() const;
		Palette * getParent() const;

		uint8_t operator[] (size_t index) const;

		bool operator == (const ShadeTable & shadeTable) const;
		bool operator != (const ShadeTable & shadeTable) const;

	private:
		void setParent(Palette * palette);
		void clearParent();

		std::unique_ptr<ShadeData> m_shadeData;
		Palette * m_parent;
	};

	class TranslucencyTable final {
		friend class PaletteDAT;

	public:
		using TranslucencyData = std::array<uint8_t, TRANSLUCENCY_TABLE_SIZE_BYTES>;

		TranslucencyTable(Palette * parent = nullptr);
		TranslucencyTable(std::unique_ptr<TranslucencyData> translucencyData, Palette * parent = nullptr);
		TranslucencyTable(const TranslucencyData & translucencyData, Palette * parent = nullptr);
		TranslucencyTable(TranslucencyTable && translucencyTable) noexcept;
		TranslucencyTable(const TranslucencyTable & translucencyTable);
		TranslucencyTable & operator = (TranslucencyTable && translucencyTable) noexcept;
		TranslucencyTable & operator = (const TranslucencyTable & translucencyTable);
		~TranslucencyTable();

		const TranslucencyData & getData() const;
		void setData(std::unique_ptr<TranslucencyData> translucencyData);
		void setData(const TranslucencyData & translucencyData);
		bool setData(const std::vector<uint8_t> & translucencyData);

		static std::unique_ptr<TranslucencyTable> getFrom(const ByteBuffer & byteBuffer, size_t offset);
		static std::unique_ptr<TranslucencyTable> readFrom(const ByteBuffer & byteBuffer);
		bool putIn(ByteBuffer & byteBuffer, size_t offset) const;
		bool insertIn(ByteBuffer & byteBuffer, size_t offset) const;
		bool writeTo(ByteBuffer & byteBuffer) const;

		bool isParentValid() const;
		Palette * getParent() const;

		uint8_t operator[] (size_t index) const;

		bool operator == (const TranslucencyTable & translucencyTable) const;
		bool operator != (const TranslucencyTable & translucencyTable) const;

	private:
		void setParent(Palette * palette);
		void clearParent();

		std::unique_ptr<TranslucencyData> m_translucencyData;
		Palette * m_parent;
	};

	class SwapTable final {
		friend class PaletteDAT;

	public:
		using SwapData = std::array<uint8_t, NUMBER_OF_COLOURS>;

		SwapTable(Palette * parent = nullptr);
		SwapTable(std::unique_ptr<SwapData> swapData, uint8_t swapIndex, Palette * parent = nullptr);
		SwapTable(const SwapData & swapData, uint8_t swapIndex, Palette * parent = nullptr);
		SwapTable(SwapTable && swapTable) noexcept;
		SwapTable(const SwapTable & swapTable);
		SwapTable & operator = (SwapTable && swapTable) noexcept;
		SwapTable & operator = (const SwapTable & swapTable);
		~SwapTable();

		uint8_t getSwapIndex() const;
		void setSwapIndex(uint8_t index);
		const SwapData & getData() const;
		void setData(std::unique_ptr<SwapData> swapData);
		void setData(const SwapData & swapData);
		bool setData(const std::vector<uint8_t> & swapData);

		static std::unique_ptr<SwapTable> getFrom(const ByteBuffer & byteBuffer, size_t offset);
		static std::unique_ptr<SwapTable> readFrom(const ByteBuffer & byteBuffer);
		bool putIn(ByteBuffer & byteBuffer, size_t offset) const;
		bool insertIn(ByteBuffer & byteBuffer, size_t offset) const;
		bool writeTo(ByteBuffer & byteBuffer) const;

		bool isParentValid() const;
		Palette * getParent() const;

		uint8_t operator[] (size_t index) const;

		bool operator == (const SwapTable & swapTable) const;
		bool operator != (const SwapTable & swapTable) const;

	private:
		void setParent(Palette * palette);
		void clearParent();

		uint8_t m_swapIndex;
		std::unique_ptr<SwapData> m_swapData;
		Palette * m_parent;
	};

	PaletteDAT(Type type, const std::string & filePath = {});
	PaletteDAT(std::unique_ptr<ColourTable> colourTable, std::vector<std::unique_ptr<ShadeTable>> shadeTables, std::unique_ptr<TranslucencyTable> translucencyTable, std::unique_ptr<ByteBuffer> trailingData = nullptr, const std::string & filePath = {});
	PaletteDAT(std::vector<std::unique_ptr<ColourTable>> colourTables, std::vector<std::unique_ptr<SwapTable>> swapTables, std::unique_ptr<ByteBuffer> trailingData = nullptr, const std::string & filePath = {});
	PaletteDAT(PaletteDAT && palette) noexcept;
	PaletteDAT(const PaletteDAT & palette);
	PaletteDAT & operator = (PaletteDAT && palette) noexcept;
	PaletteDAT & operator = (const PaletteDAT & palette);
	virtual ~PaletteDAT();

	Type getType() const;
	bool hasTrailingData() const;
	const ByteBuffer & getTrailingData() const;
	void clearTrailingData();
	bool hasTranslucencyTable() const;
	std::shared_ptr<TranslucencyTable> getTranslucencyTable() const;
	size_t numberOfShadeTables() const;
	bool hasShadeTable(const ShadeTable & shadeTable) const;
	size_t indexOfShadeTable(const ShadeTable & shadeTable) const;
	std::shared_ptr<ShadeTable> getShadeTable(size_t index) const;
	const std::vector<std::shared_ptr<ShadeTable>> & getShadeTables() const;
	bool addShadeTable(std::unique_ptr<ShadeTable> shadeTable);
	void addShadeTable(const ShadeTable & shadeTable);
	bool insertShadeRable(size_t index, std::unique_ptr<ShadeTable> shadeTable);
	bool insertShadeRable(size_t index, const ShadeTable & shadeTable);
	bool replaceShadeRable(size_t index, std::unique_ptr<ShadeTable> shadeTable);
	bool replaceShadeRable(size_t index, const ShadeTable & shadeTable);
	bool removeShadeTable(size_t index);
	bool removeShadeTable(const ShadeTable & shadeTable);
	void clearShadeTables();
	size_t numberOfSwapTables() const;
	bool hasSwapTable(const SwapTable & swapTable) const;
	bool hasSwapTableWithSwapIndex(uint8_t swapIndex) const;
	size_t indexOfSwapTable(const SwapTable & swapTable) const;
	size_t indexOfSwapTableWithSwapIndex(uint8_t swapIndex) const;
	std::shared_ptr<SwapTable> getSwapTable(size_t index) const;
	std::shared_ptr<SwapTable> getSwapTableWithSwapIndex(uint8_t swapIndex) const;
	const std::vector<std::shared_ptr<SwapTable>> & getSwapTables() const;
	bool addSwapTable(std::unique_ptr<SwapTable> swapTable);
	bool addSwapTable(const SwapTable & swapTable);
	bool insertSwapRable(size_t index, std::unique_ptr<SwapTable> swapTable);
	bool insertSwapRable(size_t index, const SwapTable & swapTable);
	bool replaceSwapRable(size_t index, std::unique_ptr<SwapTable> swapTable);
	bool replaceSwapRable(size_t index, const SwapTable & swapTable);
	bool removeSwapTable(size_t index);
	bool removeSwapTable(const SwapTable & swapTable);
	bool removeSwapTableWithIndex(uint8_t swapIndex);
	void clearSwapTables();
	static std::optional<PaletteDAT::Type> determineType(const ByteBuffer & byteBuffer);
	virtual std::shared_ptr<ColourTable> getColourTable(uint8_t colourTableIndex = 0) const override;
	virtual uint8_t numberOfColourTables() const override;
	virtual void addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const override;
	static std::unique_ptr<PaletteDAT> readFrom(const ByteBuffer & byteBuffer);
	static std::unique_ptr<PaletteDAT> loadFrom(const std::string & filePath);
	virtual bool writeTo(ByteBuffer & byteBuffer) const override;
	virtual Endianness getEndianness() const override;
	virtual size_t getSizeInBytes() const override;
	virtual bool isValid(bool verifyParent = true) const override;

	bool operator == (const PaletteDAT & palette) const;
	bool operator != (const PaletteDAT & palette) const;

	static constexpr Endianness ENDIANNESS = Endianness::LittleEndian;
	static constexpr uint8_t BYTES_PER_COLOUR = 3;
	static constexpr uint8_t COLOUR_SCALE = 4;

private:
	static std::unique_ptr<PaletteDAT> readPaletteFrom(const ByteBuffer & byteBuffer);
	static std::unique_ptr<PaletteDAT> readLookupFrom(const ByteBuffer & byteBuffer);
	bool writePaletteTo(ByteBuffer & byteBuffer) const;
	bool writeLookupTo(ByteBuffer & byteBuffer) const;
	static bool writeDownscaledColourTableTo(const ColourTable & colourTable, ByteBuffer & byteBuffer);
	static void upscaleColourTable(ColourTable & colourTable);
	void updateParent();

	Type m_type;
	std::vector<std::shared_ptr<ColourTable>> m_colourTables;
	std::vector<std::shared_ptr<ShadeTable>> m_shadeTables;
	std::shared_ptr<TranslucencyTable> m_translucencyTable;
	std::vector<std::shared_ptr<SwapTable>> m_swapTables;
	std::unique_ptr<ByteBuffer> m_trailingData;
};

#endif // _PALETTE_DAT_H_
