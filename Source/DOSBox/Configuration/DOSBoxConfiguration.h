#ifndef _DOSBOX_CONFIGURATION_H_
#define _DOSBOX_CONFIGURATION_H_

#include "CommentCollection.h"

#include <BitmaskOperators.h>
#include <ByteBuffer.h>

#include <cstdint>
#include <memory>
#include <string>
#include <map>
#include <vector>

class DOSBoxConfiguration final : public CommentCollection {
public:
	enum class Style : uint8_t {
		None = 0,
		WhitespaceAfterEntryNames = 1,
		NewlineAfterSectionComments = 1 << 1,
		All = WhitespaceAfterEntryNames | NewlineAfterSectionComments,
		Default = NewlineAfterSectionComments
	};

	class Section;

	struct NameComparator {
	public:
		bool operator () (const std::string & nameA, const std::string & nameB) const;
	};

	using SectionMap = std::map<std::string, std::shared_ptr<Section>, NameComparator>;

	class Section final : public CommentCollection {
		friend class DOSBoxConfiguration;

	public:
		class Entry;

		using EntryMap = std::map<std::string, std::shared_ptr<Entry>, NameComparator>;

		class Entry final {
			friend class DOSBoxConfiguration;
			friend class Section;

		public:
			Entry(std::string_view name, std::string_view value, Section * parent = nullptr);
			Entry(Entry && entry) noexcept;
			Entry(const Entry & entry);
			Entry & operator = (Entry && entry) noexcept;
			Entry & operator = (const Entry & entry);
			~Entry();

			const std::string & getName() const;
			bool setName(const std::string & newName);
			bool hasValue() const;
			const std::string & getValue() const;
			void setValue(std::string_view value);
			void clearValue();
			bool remove();
			const Section * getParentSection() const;
			const DOSBoxConfiguration * getParentConfiguration() const;

			static std::unique_ptr<Entry> parseFrom(std::string_view data, Style * style = nullptr);
			bool writeTo(ByteBuffer & data, const Style & style, size_t maxLength = 0) const;

			bool isValid(bool validateParents = true) const;
			static bool isValid(const Entry * entry, bool validateParents = true);
			static bool isNameValid(std::string_view entryName);

			std::string toString() const;

			bool operator == (const Entry & entry) const;
			bool operator != (const Entry & entry) const;

			static constexpr char ASSIGNMENT_CHARACTER = '=';

		private:
			std::string m_name;
			std::string m_value;

			Section * m_parent;
		};

		Section(std::string_view name, DOSBoxConfiguration * parent = nullptr);
		Section(Section && section) noexcept;
		Section(const Section & section);
		Section & operator = (Section && section) noexcept;
		Section & operator = (const Section & section);
		virtual ~Section();

		const std::string & getName() const;
		bool setName(const std::string & newName);
		bool remove();
		const DOSBoxConfiguration * getParentConfiguration() const;

		size_t numberOfEntries() const;
		bool hasEntry(const Entry & entry) const;
		bool hasEntryWithName(const std::string & entryName) const;
		size_t indexOfEntry(const Entry & entry) const;
		size_t indexOfEntryWithName(const std::string & entryName) const;
		std::shared_ptr<Entry> getEntry(size_t entryIndex) const;
		std::shared_ptr<Entry> getEntry(const Entry & entry) const;
		std::shared_ptr<Entry> getEntryWithName(const std::string & entryName) const;
		bool setEntryName(size_t entryIndex, const std::string & newEntryName);
		bool setEntryName(const std::string & oldEntryName, const std::string & newEntryName);
		bool setEntryName(Entry & entry, const std::string & newEntryName);
		bool addEntry(std::unique_ptr<Entry> entry);
		bool addEntry(const Entry & entry);
		bool replaceEntry(size_t entryIndex, std::unique_ptr<Entry> newEntry);
		bool replaceEntry(const Entry & oldEntry, std::unique_ptr<Entry> newEntry);
		bool replaceEntryWithName(const std::string & oldEntryName, std::unique_ptr<Entry> newEntry);
		bool replaceEntry(size_t entryIndex, const Entry & newEntry);
		bool replaceEntry(const Entry & oldEntry, const Entry & newEntry);
		bool replaceEntryWithName(const std::string & oldEntryName, const Entry & newEntry);
		bool insertEntry(size_t entryIndex, std::unique_ptr<Entry> newEntry);
		bool insertEntry(size_t entryIndex, const Entry & newEntry);
		bool removeEntry(size_t entryIndex);
		bool removeEntry(const Entry & entry);
		bool removeEntryWithName(const std::string & entryName);
		void clearEntries();

		static std::unique_ptr<Section> readFrom(const ByteBuffer & data, Style * style = nullptr);
		bool writeTo(ByteBuffer & data, const Style & style) const;

		bool isValid(bool validateParents = true) const;
		static bool isValid(const Section * section, bool validateParents = true);
		static bool isNameValid(std::string_view sectionName);

		bool operator == (const Section & section) const;
		bool operator != (const Section & section) const;

		static constexpr char NAME_START_CHARACTER = '[';
		static constexpr char NAME_END_CHARACTER = ']';

	private:
		void updateParent();

		std::string m_name;
		EntryMap m_entries;
		std::vector<std::string> m_orderedEntryNames;

		DOSBoxConfiguration * m_parent;
	};

	DOSBoxConfiguration(const std::string & filePath = {});
	DOSBoxConfiguration(DOSBoxConfiguration && configuration) noexcept;
	DOSBoxConfiguration(const DOSBoxConfiguration & configuration);
	DOSBoxConfiguration & operator = (DOSBoxConfiguration && configuration) noexcept;
	DOSBoxConfiguration & operator = (const DOSBoxConfiguration & configuration);
	virtual ~DOSBoxConfiguration();

	bool hasFilePath() const;
	const std::string & getFilePath() const;
	std::string_view getFileName() const;
	std::string_view getFileExtension() const;
	void setFilePath(const std::string & filePath);
	void clearFilePath();
	bool hasWhitespaceAfterEntryNames();
	bool hasNewlineAfterSectionComments();
	Style getStyle() const;
	bool hasStyle(Style style) const;
	void setStyle(Style style);
	void addStyle(Style style);
	void removeStyle(Style style);

	size_t numberOfSections() const;
	bool hasSection(const Section & section) const;
	bool hasSectionWithName(const std::string & sectionName) const;
	size_t indexOfSection(const Section & section) const;
	size_t indexOfSectionWithName(const std::string & sectionName) const;
	std::shared_ptr<Section> getSection(size_t sectionIndex) const;
	std::shared_ptr<Section> getSection(const Section & section) const;
	std::shared_ptr<Section> getSectionWithName(const std::string & sectionName) const;
	bool setSectionName(size_t sectionIndex, const std::string & newSectionName);
	bool setSectionName(const std::string & oldSectionName, const std::string & newSectionName);
	bool setSectionName(Section & section, const std::string & newSectionName);
	bool addSection(std::unique_ptr<Section> newSection);
	bool addSection(const Section & newSection);
	bool addEntryToSection(std::unique_ptr<Section::Entry> newEntry, size_t sectionIndex);
	bool addEntryToSection(const Section::Entry & newEntry, size_t sectionIndex);
	bool addEntryToSectionWithName(std::unique_ptr<Section::Entry> newEntry, const std::string & sectionName);
	bool addEntryToSectionWithName(const Section::Entry & newEntry, const std::string & sectionName);
	bool replaceSection(size_t sectionIndex, std::unique_ptr<Section> newSection);
	bool replaceSection(const Section & oldSection, std::unique_ptr<Section> newSection);
	bool replaceSection(const std::string & oldSectionName, std::unique_ptr<Section> newSection);
	bool replaceSection(size_t sectionIndex, const Section & newSection);
	bool replaceSection(const Section & oldSection, const Section & newSection);
	bool replaceSection(const std::string & oldSectionName, const Section & newSection);
	bool replaceEntryInSection(size_t entryIndex, std::unique_ptr<Section::Entry> newEntry, size_t sectionIndex);
	bool replaceEntryInSection(const Section::Entry & oldEntry, std::unique_ptr<Section::Entry> newEntry, size_t sectionIndex);
	bool replaceEntryInSection(const std::string & oldEntryName, std::unique_ptr<Section::Entry> newEntry, size_t sectionIndex);
	bool replaceEntryInSection(size_t entryIndex, const Section::Entry & newEntry, size_t sectionIndex);
	bool replaceEntryInSection(const Section::Entry & oldEntry, const Section::Entry & newEntry, size_t sectionIndex);
	bool replaceEntryInSection(const std::string & oldEntryName, const Section::Entry & newEntry, size_t sectionIndex);
	bool replaceEntryInSectionWithName(size_t entryIndex, std::unique_ptr<Section::Entry> newEntry, const std::string & sectionName);
	bool replaceEntryInSectionWithName(const Section::Entry & oldEntry, std::unique_ptr<Section::Entry> newEntry, const std::string & sectionName);
	bool replaceEntryInSectionWithName(const std::string & oldEntryName, std::unique_ptr<Section::Entry> newEntry, const std::string & sectionName);
	bool replaceEntryInSectionWithName(size_t entryIndex, const Section::Entry & newEntry, const std::string & sectionName);
	bool replaceEntryInSectionWithName(const Section::Entry & oldEntry, const Section::Entry & newEntry, const std::string & sectionName);
	bool replaceEntryInSectionWithName(const std::string & oldEntryName, const Section::Entry & newEntry, const std::string & sectionName);
	bool insertSection(size_t sectionIndex, std::unique_ptr<Section> newSection);
	bool insertEntryInSection(size_t entryIndex, std::unique_ptr<Section::Entry> newEntry, size_t sectionIndex);
	bool insertEntryInSection(size_t entryIndex, const Section::Entry & newEntry, size_t sectionIndex);
	bool insertEntryInSectionWithName(size_t entryIndex, std::unique_ptr<Section::Entry> newEntry, const std::string & sectionName);
	bool insertEntryInSectionWithName(size_t entryIndex, const Section::Entry & newEntry, const std::string & sectionName);
	bool removeSection(size_t sectionIndex);
	bool removeSection(const Section & section);
	bool removeEntryFromSection(size_t entryIndex, size_t sectionIndex);
	bool removeEntryFromSection(const Section::Entry & entry, size_t sectionIndex);
	bool removeEntryFromSectionWithName(size_t entryIndex, const std::string & sectionName);
	bool removeEntryFromSectionWithName(const Section::Entry & entry, const std::string & sectionName);
	bool removeSectionWithName(const std::string & sectionName);
	void clearSections();

	static std::unique_ptr<DOSBoxConfiguration> readFrom(const ByteBuffer & data);
	bool writeTo(ByteBuffer & data, const Style * styleOverride = nullptr) const;

	static std::unique_ptr<DOSBoxConfiguration> loadFrom(const std::string & filePath);
	bool save(bool overwrite = true, bool createParentDirectories = true) const;
	bool saveTo(const std::string & filePath, bool overwrite = true, bool createParentDirectories = true) const;

	bool isValid(bool validateParents = true) const;
	static bool isValid(const DOSBoxConfiguration * configuration, bool validateParents = true);

	bool operator == (const DOSBoxConfiguration & configuration) const;
	bool operator != (const DOSBoxConfiguration & configuration) const;

	static constexpr char COMMENT_CHARACTER = '#';

private:
	void updateParent(bool recursive = true);

	Style m_style;
	SectionMap m_sections;
	std::vector<std::string> m_orderedSectionNames;
	std::string m_filePath;
};

template<>
struct BitmaskOperators<DOSBoxConfiguration::Style> {
	static const bool enabled = true;
};

#endif // _DOSBOX_CONFIGURATION_H_
