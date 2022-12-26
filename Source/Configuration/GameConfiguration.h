#ifndef _GAME_CONFIGURATION_H_
#define _GAME_CONFIGURATION_H_

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

class GameConfiguration final {
public:
	class Entry;
	class Section;

	struct NameComparator {
	public:
		bool operator () (const std::string & nameA, const std::string & nameB) const;
	};

	using EntryMap = std::map<std::string, std::shared_ptr<Entry>, NameComparator>;
	using SectionMap = std::map<std::string, std::shared_ptr<Section>, NameComparator>;

	class Entry final {
		friend class GameConfiguration;
		friend class Section;

	public:
		enum class Type {
			Empty,
			Integer,
			Hexadecimal,
			String,
			MultiString
		};

		Entry(const std::string & name);
		Entry(Entry && e) noexcept;
		Entry(const Entry & e);
		Entry & operator = (Entry && e) noexcept;
		Entry & operator = (const Entry & e);
		~Entry();

		const std::string & getName() const;
		bool setName(const std::string & newName);
		Type getType() const;
		bool isEmpty() const;
		bool isInteger() const;
		bool isHexadecimal() const;
		bool isString() const;
		bool isMultiString() const;
		bool getBooleanValue() const;
		int64_t getIntegerValue() const;
		std::string getHexadecimalValue() const;
		const std::string & getStringValue() const;
		size_t numberOfMultiStringValues() const;
		bool hasMultiStringValue(const std::string & value) const;
		size_t indexOfMultiStringValue(const std::string & value) const;
		const std::string & getMultiStringValue(size_t index) const;
		const std::vector<std::string> & getMultiStringValue() const;
		void setEmpty();
		void setIntegerValue(int64_t value);
		void setHexadecimalValueFromDecimal(int64_t value);
		void setStringValue(const std::string & value);
		bool setMultiStringValue(const std::string value, size_t index);
		void setMultiStringValue(const std::string & valueA, const std::string & valueB);
		template <size_t N>
		void setMultiStringValue(const std::array<std::string, N> & value);
		void setMultiStringValue(const std::vector<std::string> & value);
		void clearValue();
		bool remove();
		const Section * getParentSection() const;
		const GameConfiguration * getParentGameConfiguration() const;

		bool isValid(bool validateParents = true) const;
		static bool isValid(const Entry * entry, bool validateParents = true);
		static bool isNameValid(const std::string & entryName);

		std::string toString() const;
		static std::unique_ptr<Entry> parseFrom(std::string_view data);

		bool operator == (const Entry & e) const;
		bool operator != (const Entry & e) const;

	private:
		std::string m_name;
		Type m_type;
		std::variant<int64_t, std::string, std::vector<std::string>> m_value;
		Section * m_parentSection;
		GameConfiguration * m_parentGameConfiguration;
	};

	class Section final {
		friend class GameConfiguration;

	public:
		Section(const std::string & name, const std::string & precedingComments = {}, const std::string & followingComments = {});
		Section(Section && s) noexcept;
		Section(const Section & s);
		Section & operator = (Section && s) noexcept;
		Section & operator = (const Section & s);
		~Section();

		const std::string & getName() const;
		bool setName(const std::string & newName);
		size_t numberOfPrecedingComments() const;
		std::string getPrecedingComment(size_t index) const;
		const std::string & getPrecedingComments() const;
		void setPrecedingComments(const std::string & precedingComments);
		void addPrecedingComment(const std::string & newComment);
		bool insertPrecedingComment(const std::string & newComment, size_t index);
		bool removePrecedingComment(size_t index);
		void clearPrecedingComments();
		size_t numberOfFollowingComments() const;
		std::string getFollowingComment(size_t index) const;
		const std::string & getFollowingComments() const;
		void setFollowingComments(const std::string & followingComments);
		void addFollowingComment(const std::string & newComment);
		bool insertFollowingComment(const std::string & newComment, size_t index);
		bool removeFollowingComment(size_t index);
		void clearFollowingComments();
		size_t numberOfEntries() const;
		bool hasEntry(const Entry & entry) const;
		bool hasEntryWithName(const std::string & entryName) const;
		size_t indexOfEntry(const Entry & entry) const;
		size_t indexOfEntryWithName(const std::string & entryName) const;
		std::shared_ptr<Entry> getEntry(size_t index) const;
		std::shared_ptr<Entry> getEntryWithName(const std::string & entryName) const;
		bool addEntry(std::shared_ptr<Entry> entry);
		bool removeEntry(size_t index);
		bool removeEntry(const Entry & entry);
		bool removeEntryWithName(const std::string & entryName);
		bool clearEntries();
		bool remove();
		const GameConfiguration * getParentGameConfiguration() const;

		std::string toString() const;
		static std::unique_ptr<Section> parseFrom(const std::string & data, size_t & offset);

		bool isValid(bool validateParents = true) const;
		static bool isValid(const Section * section, bool validateParents = true);
		static bool isNameValid(const std::string & sectionName);

		bool operator == (const Section & s) const;
		bool operator != (const Section & s) const;

	private:
		static std::string formatComments(const std::string & unformattedComments);
		void updateParent();

		std::string m_name;
		std::string m_precedingComments;
		std::string m_followingComments;
		std::vector<std::shared_ptr<Entry>> m_entries;
		GameConfiguration * m_parentGameConfiguration;
	};

	GameConfiguration();
	GameConfiguration(GameConfiguration && c) noexcept;
	GameConfiguration(const GameConfiguration & c);
	GameConfiguration & operator = (GameConfiguration && c) noexcept;
	GameConfiguration & operator = (const GameConfiguration & c);
	~GameConfiguration();

	bool hasFilePath() const;
	const std::string & getFilePath() const;
	void setFilePath(const std::string & filePath);
	void clearFilePath();

	size_t numberOfEntries() const;
	bool hasEntry(const Entry & entry) const;
	bool hasEntryWithName(const std::string & entryName) const;
	std::shared_ptr<Entry> getEntry(const Entry & entry) const;
	std::shared_ptr<Entry> getEntryWithName(const std::string & entryName) const;
	bool setEntryName(const std::string & oldEntryName, const std::string & newEntryName);
	bool setEntryName(Entry & entry, const std::string & newEntryName);
	bool addEntryToSection(std::shared_ptr<Entry> entry, const Section & section);
	bool addEntryToSectionWithName(std::shared_ptr<Entry> entry, const std::string & sectionName);
	bool removeEntry(const Entry & entry);
	bool removeEntryWithName(const std::string & entryName);
	size_t removeEntries(const std::vector<std::shared_ptr<Entry>> & entries);
	void clearEntries();

	size_t numberOfSections() const;
	bool hasSection(const Section & section) const;
	bool hasSectionWithName(const std::string & sectionName) const;
	std::shared_ptr<Section> getSection(const Section & section) const;
	std::shared_ptr<Section> getSectionWithName(const std::string & sectionName) const;
	bool setSectionName(const std::string & oldSectionName, const std::string & newSectionName);
	bool setSectionName(Section & section, const std::string & newSectionName);
	bool addSection(std::shared_ptr<Section> section);
	bool removeSection(const Section & section);
	bool removeSectionWithName(const std::string & sectionName);
	void clearSections();

	std::string toString() const;
	static std::unique_ptr<GameConfiguration> parseFrom(const std::string & data);

	static std::unique_ptr<GameConfiguration> loadFrom(const std::string & filePath);
	bool save(bool overwrite = true, bool createParentDirectories = true) const;
	bool saveTo(const std::string & filePath, bool overwrite = false, bool createParentDirectories = true) const;

	bool isValid(bool validateParents = true) const;
	static bool isValid(const GameConfiguration * gameConfiguration, bool validateParents = true);

	bool operator == (const GameConfiguration & c) const;
	bool operator != (const GameConfiguration & c) const;

	static const char COMMENT_CHARACTER;
	static const char SECTION_NAME_START_CHARACTER;
	static const char SECTION_NAME_END_CHARACTER;
	static const char ASSIGNMENT_CHARACTER;
	static const char EMPTY_VALUE_CHARACTER;

private:
	void updateParent();

	std::string m_filePath;
	EntryMap m_entries;
	SectionMap m_sections;
};

template <size_t N>
void GameConfiguration::Entry::setMultiStringValue(const std::array<std::string, N> & value) {
	m_type = Type::MultiString;

	std::vector<std::string> multiStringValue;

	for(const std::string & stringValue : value) {
		multiStringValue.push_back(value);
	}

	m_value = multiStringValue;
}

#endif // _GAME_CONFIGURATION_H_