#ifndef _DOSBOX_CONFIGURATION_H_
#define _DOSBOX_CONFIGURATION_H_

#include "CommentCollection.h"

#include <BitmaskOperators.h>
#include <ByteBuffer.h>
#include <Signal/SignalConnectionGroup.h>

#include <boost/signals2.hpp>

#include <cstdint>
#include <memory>
#include <optional>
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

	enum class NewlineType : uint8_t {
		Unix,
		Windows
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
			bool isModified() const;
			const Section * getParentSection() const;
			const DOSBoxConfiguration * getParentConfiguration() const;

			static std::unique_ptr<Entry> parseFrom(std::string_view data, Style * style = nullptr);
			bool writeTo(ByteBuffer & data, Style style, size_t maxLength = 0, const std::string & newline = {}) const;

			bool isValid(bool validateParents = true) const;
			static bool isValid(const Entry * entry, bool validateParents = true);
			static bool isNameValid(std::string_view entryName);

			std::string toString() const;

			bool operator == (const Entry & entry) const;
			bool operator != (const Entry & entry) const;

			boost::signals2::signal<void (Entry & /* entry */)> entryModified;
			boost::signals2::signal<void (Entry & /* entry */, std::string /* oldEntryName */)> entryNameChanged;
			boost::signals2::signal<void (Entry & /* entry */, std::string /* oldEntryValue */)> entryValueChanged;

			static constexpr char ASSIGNMENT_CHARACTER = '=';

		private:
			void setModified(bool value);

			std::string m_name;
			std::string m_value;

			mutable bool m_modified;
			Section * m_parent;
		};

		Section(std::string_view name, DOSBoxConfiguration * parent = nullptr);
		Section(std::string_view name, std::vector<std::unique_ptr<Entry>> entries, DOSBoxConfiguration * parent = nullptr);
		Section(std::string_view name, const std::vector<Entry> & entries, DOSBoxConfiguration * parent = nullptr);
		Section(Section && section) noexcept;
		Section(const Section & section);
		Section & operator = (Section && section) noexcept;
		Section & operator = (const Section & section);
		virtual ~Section();

		const std::string & getName() const;
		bool setName(const std::string & newName);
		bool remove();
		bool isEmpty() const;
		bool isNotEmpty() const;
		bool mergeWith(const Section & section);
		bool setSection(const Section & section);
		void clear();
		bool isModified() const;
		const DOSBoxConfiguration * getParentConfiguration() const;

		size_t numberOfEntries() const;
		bool hasEntry(const Entry & entry) const;
		bool hasEntryWithName(const std::string & entryName) const;
		size_t indexOfEntry(const Entry & entry) const;
		size_t indexOfEntryWithName(const std::string & entryName) const;
		std::shared_ptr<Entry> getEntry(size_t entryIndex) const;
		std::shared_ptr<Entry> getEntry(const Entry & entry) const;
		std::shared_ptr<Entry> getEntryWithName(const std::string & entryName) const;
		const EntryMap & getUnorderedEntries() const;
		std::vector<std::shared_ptr<Entry>> getOrderedEntries() const;
		const std::vector<std::string> & getOrderedEntryNames() const;
		bool setEntryName(size_t entryIndex, const std::string & newEntryName);
		bool setEntryName(const std::string & oldEntryName, const std::string & newEntryName);
		bool setEntryName(Entry & entry, const std::string & newEntryName);
		bool setEntryValue(size_t entryIndex, const std::string & newEntryValue);
		bool setEntryValue(const std::string & entryName, const std::string & newEntryValue);
		bool setEntryValue(Entry & entry, const std::string & newEntryValue);
		bool addEntry(std::unique_ptr<Entry> newEntry);
		bool addEntry(const Entry & newEntry);
		bool addEntry(const std::string & newEntryName, const std::string & newEntryValue);
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
		bool writeTo(ByteBuffer & data, Style style, const std::string & newline = {}) const;

		bool isValid(bool validateParents = true) const;
		static bool isValid(const Section * section, bool validateParents = true);
		static bool isNameValid(std::string_view sectionName);

		bool operator == (const Section & section) const;
		bool operator != (const Section & section) const;

		boost::signals2::signal<void (Section & /* section */)> sectionModified;
		boost::signals2::signal<void (Section & /* section */, std::string /* oldSectionName */)> sectionNameChanged;

		boost::signals2::signal<void (Section & /* section */, std::string /* newComment */, size_t /* commentIndex */)> sectionCommentAdded;
		boost::signals2::signal<void (Section & /* section */, std::string /* newComment */, size_t /* commentIndex */, std::string /* oldComment */)> sectionCommentReplaced;
		boost::signals2::signal<void (Section & /* section */, std::string /* newComment */, size_t /* commentIndex */)> sectionCommentInserted;
		boost::signals2::signal<void (Section & /* section */, std::string /* comment */, size_t /* commentIndex */)> sectionCommentRemoved;
		boost::signals2::signal<void (Section & /* section */)> sectionCommentsCleared;

		boost::signals2::signal<void (Section & /* section */, std::shared_ptr<Section::Entry> /* entry */, size_t /* entryIndex */, std::string /* oldEntryName */)> sectionEntryNameChanged;
		boost::signals2::signal<void (Section & /* section */, std::shared_ptr<Section::Entry> /* entry */, size_t /* entryIndex */, std::string /* oldEntryValue */)> sectionEntryValueChanged;
		boost::signals2::signal<void (Section & /* section */, std::shared_ptr<Section::Entry> /* newEntry */, size_t /* entryIndex */)> sectionEntryAdded;
		boost::signals2::signal<void (Section & /* section */, std::shared_ptr<Section::Entry> /* newEntry */, size_t /* entryIndex */, std::shared_ptr<Section::Entry> /* oldEntry */)> sectionEntryReplaced;
		boost::signals2::signal<void (Section & /* section */, std::shared_ptr<Section::Entry> /* newEntry */, size_t /* entryIndex */)> sectionEntryInserted;
		boost::signals2::signal<void (Section & /* section */, std::shared_ptr<Section::Entry> /* entry */, size_t /* entryIndex */)> sectionEntryRemoved;
		boost::signals2::signal<void (Section & /* section */)> sectionEntriesCleared;

		static constexpr char NAME_START_CHARACTER = '[';
		static constexpr char NAME_END_CHARACTER = ']';

	private:
		void updateParent();
		void setModified(bool value);
		void disconnectSignals();
		void connectSignals();
		SignalConnectionGroup connectEntrySignals(Entry & entry);
		void onCommentCollectionModified(CommentCollection & commentCollection);
		void onCommentAdded(CommentCollection & commentCollection, std::string newComment, size_t commentIndex);
		void onCommentReplaced(CommentCollection & commentCollection, std::string newComment, size_t commentIndex, std::string oldComment);
		void onCommentInserted(CommentCollection & commentCollection, std::string newComment, size_t commentIndex);
		void onCommentRemoved(CommentCollection & commentCollection, std::string comment, size_t commentIndex);
		void onCommentsCleared(CommentCollection & commentCollection);
		void onEntryModified(Entry & entry);
		void onEntryNameChanged(Entry & entry, std::string oldEntryName);
		void onEntryValueChanged(Entry & entry, std::string oldEntryValue);

		std::string m_name;
		EntryMap m_entries;
		std::vector<std::string> m_orderedEntryNames;

		mutable bool m_modified;
		DOSBoxConfiguration * m_parent;
		SignalConnectionGroup m_commentCollectionConnections;
		std::vector<SignalConnectionGroup> m_entryConnections;
	};

	DOSBoxConfiguration(const std::string & filePath = {});
	DOSBoxConfiguration(std::vector<std::unique_ptr<Section>> sections, const std::string & filePath = {});
	DOSBoxConfiguration(const std::vector<Section> & sections, const std::string & filePath = {});
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
	NewlineType getNewlineType() const;
	void setNewlineType(NewlineType newlineType);
	bool isEmpty() const;
	bool isNotEmpty() const;
	bool mergeWith(const DOSBoxConfiguration & configuration);
	bool setConfiguration(const DOSBoxConfiguration & configuration);
	void clear();
	bool isModified() const;
	void setModified(bool value);

	size_t numberOfSections() const;
	size_t totalNumberOfEntries() const;
	size_t totalNumberOfComments() const;
	bool hasSection(const Section & section) const;
	bool hasSectionWithName(const std::string & sectionName) const;
	size_t indexOfSection(const Section & section) const;
	size_t indexOfSectionWithName(const std::string & sectionName) const;
	std::shared_ptr<Section> getSection(size_t sectionIndex) const;
	std::shared_ptr<Section> getSection(const Section & section) const;
	std::shared_ptr<Section> getSectionWithName(const std::string & sectionName) const;
	const SectionMap & getUnorderedSections() const;
	std::vector<std::shared_ptr<Section>> getOrderedSections() const;
	const std::vector<std::string> & getOrderedSectionNames() const;
	bool setSectionName(size_t sectionIndex, const std::string & newSectionName);
	bool setSectionName(const std::string & oldSectionName, const std::string & newSectionName);
	bool setSectionName(Section & section, const std::string & newSectionName);
	bool addSection(std::unique_ptr<Section> newSection);
	bool addSection(const Section & newSection);
	bool addSection(const std::string & newSectionName);
	bool addEntryToSection(std::unique_ptr<Section::Entry> newEntry, size_t sectionIndex);
	bool addEntryToSection(const Section::Entry & newEntry, size_t sectionIndex);
	bool addEntryToSection(const std::string & newEntryName, const std::string & newEntryValue, size_t sectionIndex);
	bool addEntryToSectionWithName(std::unique_ptr<Section::Entry> newEntry, const std::string & sectionName);
	bool addEntryToSectionWithName(const Section::Entry & newEntry, const std::string & sectionName);
	bool addEntryToSectionWithName(const std::string & newEntryName, const std::string & newEntryValue, const std::string & sectionName);
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
	bool writeTo(ByteBuffer & data, std::optional<Style> styleOverride = {}, std::optional<NewlineType> newlineTypeOverride = {}) const;

	static std::unique_ptr<DOSBoxConfiguration> loadFrom(const std::string & filePath);
	bool save(bool overwrite = true, bool createParentDirectories = true);
	bool saveTo(const std::string & filePath, bool overwrite = true, bool createParentDirectories = true);

	bool isValid(bool validateParents = true) const;
	static bool isValid(const DOSBoxConfiguration * configuration, bool validateParents = true);

	bool operator == (const DOSBoxConfiguration & configuration) const;
	bool operator != (const DOSBoxConfiguration & configuration) const;

	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */)> configurationModified;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, Style /* newStyle */, Style /* oldStyle */)> configurationStyleChanged;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, NewlineType /* newNewlineType */, NewlineType /* oldNewlineType */)> configurationNewlineTypeChanged;

	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::string /* newComment */, size_t /* commentIndex */)> configurationCommentAdded;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::string /* newComment */, size_t /* commentIndex */, std::string /* oldComment */)> configurationCommentReplaced;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::string /* newComment */, size_t /* commentIndex */)> configurationCommentInserted;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::string /* comment */, size_t /* commentIndex */)> configurationCommentRemoved;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */)> configurationCommentsCleared;

	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::shared_ptr<Section> /* section */, size_t /* sectionIndex */, std::string /* oldSectionName */)> configurationSectionNameChanged;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::shared_ptr<Section> /* newSection */, size_t /* sectionIndex */)> configurationSectionAdded;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::shared_ptr<Section> /* newSection */, size_t /* sectionIndex */, std::shared_ptr<Section> /* oldSection */)> configurationSectionReplaced;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::shared_ptr<Section> /* newSection */, size_t /* sectionIndex */)> configurationSectionInserted;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::shared_ptr<Section> /* section */, size_t /* sectionIndex */)> configurationSectionRemoved;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */)> configurationSectionsCleared;

	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::shared_ptr<Section> /* section */, size_t /* sectionIndex */, std::string /* newComment */, size_t /* commentIndex */)> configurationSectionCommentAdded;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::shared_ptr<Section> /* section */, size_t /* sectionIndex */, std::string /* newComment */, size_t /* commentIndex */, std::string /* oldComment */)> configurationSectionCommentReplaced;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::shared_ptr<Section> /* section */, size_t /* sectionIndex */, std::string /* newComment */, size_t /* commentIndex */)> configurationSectionCommentInserted;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::shared_ptr<Section> /* section */, size_t /* sectionIndex */, std::string /* comment */, size_t /* commentIndex */)> configurationSectionCommentRemoved;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::shared_ptr<Section> /* section */, size_t /* sectionIndex */)> configurationSectionCommentsCleared;

	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::shared_ptr<Section> /* section */, size_t /* sectionIndex */, std::shared_ptr<Section::Entry> /* entry */, size_t /* entryIndex */, std::string /* oldEntryName */)> configurationSectionEntryNameChanged;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::shared_ptr<Section> /* section */, size_t /* sectionIndex */, std::shared_ptr<Section::Entry> /* entry */, size_t /* entryIndex */, std::string /* oldEntryValue */)> configurationSectionEntryValueChanged;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::shared_ptr<Section> /* section */, size_t /* sectionIndex */, std::shared_ptr<Section::Entry> /* newEntry */, size_t /* entryIndex */)> configurationSectionEntryAdded;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::shared_ptr<Section> /* section */, size_t /* sectionIndex */, std::shared_ptr<Section::Entry> /* newEntry */, size_t /* entryIndex */, std::shared_ptr<Section::Entry> /* oldEntry */)> configurationSectionEntryReplaced;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::shared_ptr<Section> /* section */, size_t /* sectionIndex */, std::shared_ptr<Section::Entry> /* newEntry */, size_t /* entryIndex */)> configurationSectionEntryInserted;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::shared_ptr<Section> /* section */, size_t /* sectionIndex */, std::shared_ptr<Section::Entry> /* entry */, size_t /* entryIndex */)> configurationSectionEntryRemoved;
	boost::signals2::signal<void (DOSBoxConfiguration & /* dosboxConfiguration */, std::shared_ptr<Section> /* section */, size_t /* sectionIndex */)> configurationSectionEntriesCleared;

	static inline const std::string FILE_EXTENSION = "conf";
	static inline const std::string DEFAULT_FILE_NAME = "dosbox." + FILE_EXTENSION;
	static constexpr char COMMENT_CHARACTER = '#';

private:
	void updateParent(bool recursive = true);
	void connectSignals();
	SignalConnectionGroup connectSectionSignals(Section & section);
	void disconnectSignals();
	void onCommentCollectionModified(CommentCollection & commentCollection);
	void onCommentAdded(CommentCollection & commentCollection, std::string newComment, size_t commentIndex);
	void onCommentReplaced(CommentCollection & commentCollection, std::string newComment, size_t commentIndex, std::string oldComment);
	void onCommentInserted(CommentCollection & commentCollection, std::string newComment, size_t commentIndex);
	void onCommentRemoved(CommentCollection & commentCollection, std::string comment, size_t commentIndex);
	void onCommentsCleared(CommentCollection & commentCollection);
	void onSectionModified(Section & section);
	void onSectionNameChanged(Section & section, std::string oldSectionName);
	void onSectionCommentAdded(Section & section, std::string newComment, size_t commentIndex);
	void onSectionCommentReplaced(Section & section, std::string newComment, size_t commentIndex, std::string oldComment);
	void onSectionCommentInserted(Section & section, std::string newComment, size_t commentIndex);
	void onSectionCommentRemoved(Section & section, std::string comment, size_t commentIndex);
	void onSectionCommentsCleared(Section & section);
	void onSectionEntryNameChanged(Section & section, std::shared_ptr<Section::Entry> entry, size_t entryIndex, std::string oldEntryName);
	void onSectionEntryValueChanged(Section & section, std::shared_ptr<Section::Entry> entry, size_t entryIndex, std::string oldEntryValue);
	void onSectionEntryAdded(Section & section, std::shared_ptr<Section::Entry> newEntry, size_t entryIndex);
	void onSectionEntryReplaced(Section & section, std::shared_ptr<Section::Entry> newEntry, size_t entryIndex, std::shared_ptr<Section::Entry> oldEntry);
	void onSectionEntryInserted(Section & section, std::shared_ptr<Section::Entry> newEntry, size_t entryIndex);
	void onSectionEntryRemoved(Section & section, std::shared_ptr<Section::Entry> entry, size_t entryIndex);
	void onSectionEntriesCleared(Section & section);

	Style m_style;
	NewlineType m_newlineType;
	SectionMap m_sections;
	std::vector<std::string> m_orderedSectionNames;
	std::string m_filePath;
	mutable bool m_modified;
	SignalConnectionGroup m_commentCollectionConnections;
	std::vector<SignalConnectionGroup> m_sectionConnections;
};

template<>
struct BitmaskOperators<DOSBoxConfiguration::Style> {
	static const bool enabled = true;
};

#endif // _DOSBOX_CONFIGURATION_H_
