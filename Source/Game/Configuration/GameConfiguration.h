#ifndef _GAME_CONFIGURATION_H_
#define _GAME_CONFIGURATION_H_

#include <BitmaskOperators.h>
#include <Dimension.h>

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

class GameConfiguration final {
public:
	enum class Style : uint8_t {
		None = 0,
		NewlineAfterSections = 1,
		Default = None
	};

	enum class NewlineType : uint8_t {
		Unix,
		Windows
	};

	class Entry;
	class Section;

	struct NameComparator {
	public:
		bool operator () (const std::string & nameA, const std::string & nameB) const;
	};

	using EntryMap = std::multimap<std::string, std::shared_ptr<Entry>, NameComparator>;
	using SectionMap = std::map<std::string, std::shared_ptr<Section>, NameComparator>;

	class Entry final {
		friend class GameConfiguration;
		friend class Section;

	public:
		enum class Type {
			Empty,
			Integer,
			Float,
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
		bool isFloat() const;
		bool isHexadecimal() const;
		bool isString() const;
		bool isMultiString() const;
		bool getBooleanValue() const;
		int64_t getIntegerValue() const;
		float getFloatValue() const;
		std::string getHexadecimalValue() const;
		const std::string & getStringValue() const;
		size_t numberOfMultiStringValues() const;
		bool hasMultiStringValue(const std::string & value) const;
		size_t indexOfMultiStringValue(const std::string & value) const;
		const std::string & getMultiStringValue(size_t index) const;
		const std::vector<std::string> & getMultiStringValue() const;
		void setEmpty();
		void setIntegerValue(int64_t value);
		void setFloatValue(float value);
		void setHexadecimalValueFromDecimal(int64_t value);
		void setStringValue(const std::string & value);
		bool setMultiStringValue(const std::string & value, size_t index, bool resizeValue = false);
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

		static constexpr char ASSIGNMENT_CHARACTER = '=';
		static constexpr char EMPTY_VALUE_CHARACTER = '~';

	private:
		std::string m_name;
		Type m_type;
		std::variant<int64_t, float, std::string, std::vector<std::string>> m_value;
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
		bool addEmptyEntry(const std::string & entryName);
		bool addIntegerEntry(const std::string & entryName, int64_t value);
		bool addFloatEntry(const std::string & entryName, float value);
		bool addHexadecimalEntryUsingDecimal(const std::string & entryName, uint64_t value);
		bool addStringEntry(const std::string & entryName, const std::string & value);
		bool addMultiStringEntry(const std::string & entryName, const std::string & valueA, const std::string & valueB);
		template <size_t N>
		bool addMultiStringEntry(const std::string & entryName, const std::array<std::string, N> & values);
		bool addMultiStringEntry(const std::string & entryName, const std::vector<std::string> & values);
		bool setEntryEmptyValue(const std::string & entryName, bool createEntry = false);
		bool setEntryIntegerValue(const std::string & entryName, int64_t value, bool createEntry = false);
		bool setEntryFloatValue(const std::string & entryName, float value, bool createEntry = false);
		bool setEntryHexadecimalValueUsingDecimal(const std::string & entryName, uint64_t value, bool createEntry = false);
		bool setEntryStringValue(const std::string & entryName, const std::string & value, bool createEntry = false);
		bool setEntryMultiStringValue(const std::string & entryName, const std::string & value, size_t index, bool resizeValue = false, bool createEntry = false);
		bool setEntryMultiStringValue(const std::string & entryName, const std::string & valueA, const std::string & valueB, bool createEntry = false);
		template <size_t N>
		bool setEntryMultiStringValue(const std::string & entryName, const std::array<std::string, N> & values, bool createEntry = false);
		bool setEntryMultiStringValue(const std::string & entryName, const std::vector<std::string> & values, bool createEntry = false);
		bool removeEntry(size_t index);
		bool removeEntry(const Entry & entry);
		bool removeEntryWithName(const std::string & entryName);
		bool clearEntries();
		bool remove();
		const GameConfiguration * getParentGameConfiguration() const;

		std::string toString(const std::string & newline = {}) const;
		static std::unique_ptr<Section> parseFrom(const std::string & data, size_t & offset);

		bool isValid(bool validateParents = true) const;
		static bool isValid(const Section * section, bool validateParents = true);
		static bool isNameValid(const std::string & sectionName);

		bool operator == (const Section & s) const;
		bool operator != (const Section & s) const;

		static constexpr char COMMENT_CHARACTER = ';';
		static constexpr char NAME_START_CHARACTER = '[';
		static constexpr char NAME_END_CHARACTER = ']';

	private:
		static std::string formatComments(const std::string & unformattedComments);
		std::shared_ptr<Entry> getOrCreateEntry(const std::string & entryName, bool create, bool & exists);
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
	bool hasNewlineAfterSections() const;
	Style getStyle() const;
	bool hasStyle(Style style) const;
	void setStyle(Style style);
	void addStyle(Style style);
	void removeStyle(Style style);
	NewlineType getNewlineType() const;
	void setNewlineType(NewlineType newlineType);

	size_t numberOfEntries() const;
	bool hasEntry(const Entry & entry) const;
	bool hasEntryWithName(const std::string & entryName) const;
	std::shared_ptr<Entry> getEntry(const Entry & entry) const;
	std::vector<std::shared_ptr<Entry>> getEntriesWithName(const std::string & entryName) const;
	bool setEntryName(Entry & entry, const std::string & newEntryName);
	bool addEntryToSection(std::shared_ptr<Entry> entry, const Section & section);
	bool addEntryToSectionWithName(std::shared_ptr<Entry> entry, const std::string & sectionName);
	bool removeEntry(const Entry & entry);
	size_t removeAllEntriesWithName(const std::string & entryName);
	size_t removeEntries(const std::vector<std::shared_ptr<Entry>> & entries);
	void clearEntries();

	size_t numberOfSections() const;
	bool hasSection(const Section & section) const;
	bool hasSectionWithName(const std::string & sectionName) const;
	size_t indexOfSection(const Section & section) const;
	size_t indexOfSectionWithName(const std::string & sectionName) const;
	std::shared_ptr<Section> getSection(size_t index) const;
	std::shared_ptr<Section> getSection(const Section & section) const;
	std::shared_ptr<Section> getSectionWithName(const std::string & sectionName) const;
	bool setSectionName(size_t index, const std::string & newSectionName);
	bool setSectionName(const std::string & oldSectionName, const std::string & newSectionName);
	bool setSectionName(Section & section, const std::string & newSectionName);
	bool addSection(std::shared_ptr<Section> section);
	bool removeSection(size_t index);
	bool removeSection(const Section & section);
	bool removeSectionWithName(const std::string & sectionName);
	void clearSections();

	static std::unique_ptr<GameConfiguration> generateDefaultGameConfiguration(const std::string & gameVersionID);
	bool updateForDOSBox();
	bool updateWithBetterSettings();
	bool updateWithBetterControls();

	std::string toString() const;
	static std::unique_ptr<GameConfiguration> parseFrom(const std::string & data);

	static std::unique_ptr<GameConfiguration> loadFrom(const std::string & filePath);
	bool save(bool overwrite = true, bool createParentDirectories = true) const;
	bool saveTo(const std::string & filePath, bool overwrite = true, bool createParentDirectories = true) const;

	bool isValid(bool validateParents = true) const;
	static bool isValid(const GameConfiguration * gameConfiguration, bool validateParents = true);

	bool operator == (const GameConfiguration & c) const;
	bool operator != (const GameConfiguration & c) const;

	static inline const std::string DEFAULT_FILE_EXTENSION = "CFG";
	static inline const std::string DEFAULT_FILE_NAME = "DUKE3D." + DEFAULT_FILE_EXTENSION;
	static const std::string SETUP_SECTION_NAME;
	static const std::string SETUP_VERSION_ENTRY_NAME;
	static const std::string REGULAR_VERSION_SETUP_VERSION;
	static const std::string ATOMIC_EDITION_SETUP_VERSION;
	static const std::string SCREEN_SETUP_SECTION_NAME;
	static const std::string SCREEN_MODE_ENTRY_NAME;
	static const std::string SCREEN_WIDTH_ENTRY_NAME;
	static const std::string SCREEN_HEIGHT_ENTRY_NAME;
	static const std::string SCREEN_MODE_ENTRY_NAME;
	static const std::string SCREEN_WIDTH_ENTRY_NAME;
	static const std::string SCREEN_HEIGHT_ENTRY_NAME;
	static const std::string SCREEN_SHADOWS_ENTRY_NAME;
	static const std::string SCREEN_PASSWORD_ENTRY_NAME;
	static const std::string SCREEN_ENVIRONMENT_ENTRY_NAME;
	static const std::string SCREEN_DETAIL_ENTRY_NAME;
	static const std::string SCREEN_TILT_ENTRY_NAME;
	static const std::string SCREEN_MESSAGES_ENTRY_NAME;
	static const std::string SCREEN_OUT_ENTRY_NAME;
	static const std::string SCREEN_SIZE_ENTRY_NAME;
	static const std::string SCREEN_GAMMA_ENTRY_NAME;
	static const std::string SOUND_SETUP_SECTION_NAME;
	static const std::string SOUND_FX_DEVICE_ENTRY_NAME;
	static const std::string SOUND_MUSIC_DEVICE_ENTRY_NAME;
	static const std::string SOUND_FX_VOLUME_ENTRY_NAME;
	static const std::string SOUND_MUSIC_VOLUME_ENTRY_NAME;
	static const std::string SOUND_SOUND_TOGGLE_ENTRY_NAME;
	static const std::string SOUND_VOICE_TOGGLE_ENTRY_NAME;
	static const std::string SOUND_AMBIENCE_TOGGLE_ENTRY_NAME;
	static const std::string SOUND_MUSIC_TOGGLE_ENTRY_NAME;
	static const std::string SOUND_NUM_BITS_ENTRY_NAME;
	static const std::string SOUND_MIX_RATE_ENTRY_NAME;
	static const std::string KEY_DEFINITIONS_SECTION_NAME;
	static const std::string MOVE_FORWARD_ENTRY_NAME;
	static const std::string MOVE_BACKWARD_ENTRY_NAME;
	static const std::string TURN_LEFT_ENTRY_NAME;
	static const std::string TURN_RIGHT_ENTRY_NAME;
	static const std::string FIRE_ENTRY_NAME;
	static const std::string ALT_FIRE_ENTRY_NAME;
	static const std::string QUICK_KICK_ENTRY_NAME;
	static const std::string OPEN_ENTRY_NAME;
	static const std::string RUN_ENTRY_NAME;
	static const std::string AUTO_RUN_ENTRY_NAME;
	static const std::string JUMP_ENTRY_NAME;
	static const std::string CROUCH_ENTRY_NAME;
	static const std::string TOGGLE_CROUCH_ENTRY_NAME;
	static const std::string INVENTORY_ENTRY_NAME;
	static const std::string INVENTORY_LEFT_ENTRY_NAME;
	static const std::string INVENTORY_RIGHT_ENTRY_NAME;
	static const std::string MEDKIT_ENTRY_NAME;
	static const std::string TURN_AROUND_ENTRY_NAME;
	static const std::string SEND_MESSAGE_ENTRY_NAME;
	static const std::string HOLO_DUKE_ENTRY_NAME;
	static const std::string JETPACK_ENTRY_NAME;
	static const std::string NIGHT_VISION_ENTRY_NAME;
	static const std::string PREVIOUS_WEAPON_ENTRY_NAME;
	static const std::string NEXT_WEAPON_ENTRY_NAME;
	static const std::string STRAFE_ENTRY_NAME;
	static const std::string STRAFE_LEFT_ENTRY_NAME;
	static const std::string STRAFE_RIGHT_ENTRY_NAME;
	static const std::string LOOK_UP_ENTRY_NAME;
	static const std::string LOOK_DOWN_ENTRY_NAME;
	static const std::string LOOK_LEFT_ENTRY_NAME;
	static const std::string LOOK_RIGHT_ENTRY_NAME;
	static const std::string AIM_UP_ENTRY_NAME;
	static const std::string AIM_DOWN_ENTRY_NAME;
	static const std::string SHOW_OPPONENTS_WEAPON_ENTRY_NAME;
	static const std::string MAP_FOLLOW_MODE_ENTRY_NAME;
	static const std::string SEE_COOP_VIEW_ENTRY_NAME;
	static const std::string MOUSE_AIMING_ENTRY_NAME;
	static const std::string TOGGLE_CROSSHAIR_ENTRY_NAME;
	static const std::string STEROIDS_ENTRY_NAME;
	static const std::string MAP_ENTRY_NAME;
	static const std::string SHRINK_SCREEN_ENTRY_NAME;
	static const std::string ENLARGE_SCREEN_ENTRY_NAME;
	static const std::string CENTER_VIEW_ENTRY_NAME;
	static const std::string HOLSTER_WEAPON_ENTRY_NAME;
	static const std::string THIRD_PERSON_VIEW_ENTRY_NAME;
	static const std::string CONTROLS_SECTION_NAME;
	static const std::string CONTROLLER_TYPE_ENTRY_NAME;
	static const std::string CONTROLS_MOUSE_AIMING_ENTRY_NAME;
	static const std::string GAME_MOUSE_AIMING_ENTRY_NAME;
	static const std::string MOUSE_AIMING_FLIPPED_ENTRY_NAME;
	static const std::string AIMING_FLAG_ENTRY_NAME;
	static const std::string MISC_SECTION_NAME;
	static const std::string MISC_RUN_MODE_ENTRY_NAME;
	static const std::string MISC_CROSSHAIRS_ENTRY_NAME;
	static const std::string MISC_EXECUTIONS_ENTRY_NAME;
	static const std::string COMMUNICATION_IRQ_NUMBER_ENTRY_NAME;
	static const std::string COMMUNICATION_UART_ADDRESS_ENTRY_NAME;
	static const std::string COMMUNICATION_SOCKET_NUMBER_ENTRY_NAME;

	static const std::array<std::string, 10> DEFAULT_COMBAT_MACROS;
	static const std::string DEFAULT_PLAYER_NAME;
	static const Dimension DEFAULT_RESOLUTION;
	static const uint8_t SOUND_DEVICE_UNSET;
	static const uint8_t MUSIC_DEVICE_UNSET;
	static const uint32_t DEFAULT_ANALOG_SCALE;

private:
	enum class GameVersionType {
		Beta,
		RegularVersion,
		AtomicEdition,
		JFDuke3D,
		eDuke32,
		NetDuke32,
		RedNukem,
		BelgianChocolate,
		Duke3dw,
		pkDuke3D,
		xDuke,
		rDuke,
		Duke3d_w32
	};

	struct SectionFeatures {
		enum class General : uint8_t {
			None = 0,
			HasComments = 1,
			DoesSupportFloatingPointEntryValues = 1 << 1,
			Default = HasComments
		};

		enum class Setup : uint8_t {
			None = 0,
			Supported = 1,
			Populated = 1 << 1,
			HasForceSetupEntry = 1 << 2,
			HasModLoadingSupport = 1 << 3,
			HasNoAutoLoadEntry = 1 << 4,
			Default = Supported | Populated
		};

		enum class Misc : uint16_t {
			None = 0,
			Supported = 1,
			Populated = 1 << 1,
			HasHeadsUpDisplayEntries = 1 << 2,
			HasExtendedHeadsUpDisplayEntries = 1 << 3,
			HasWeaponIconsEntry = 1 << 4,
			HasWeaponHideEntry = 1 << 5,
			HasAutoSaveEntry = 1 << 6,
			HasAutoAimEntry = 1 << 7,
			HasUsePrecacheEntry = 1 << 8,
			HasShowCinematicsEntry = 1 << 9,
			HasWeaponVisibilityEntries = 1 << 10,
			HasWeaponAutoSwitchEntry = 1 << 11,
			HasWeaponChoicePreferences = 1 << 12,
			HasPredictionDebugEntry = 1 << 13,
			DoesSupportRunModeToggle = 1 << 14,
			DoesSupportCrosshairToggle = 1 << 15,
			Default = Supported | HasHeadsUpDisplayEntries | DoesSupportRunModeToggle | DoesSupportCrosshairToggle
		};

		enum class Controls : uint32_t {
			None = 0,
			Supported = 1,
			Populated = 1 << 1,
			HasAutoAimEntry = 1 << 2,
			HasWeaponSwitchModeEntry = 1 << 3,
			HasLegacyControllerEntries = 1 << 4,
			HasJoystickPortEntry = 1 << 5,
			HasMouseAnalogAxesEntries = 1 << 6,
			HasMouseAnalogScaleEntries = 1 << 7,
			HasMouseDigitalEntries = 1 << 8,
			HasRancidMeatMouseSensitivityEntries = 1 << 9,
			HasMouseYLockEntry = 1 << 10,
			HasMouseScale = 1 << 11,
			HasGameMouseAimingEntry = 1 << 12,
			HasRunKeyBehaviourEntry = 1 << 13,
			HasMouseSensitivity = 1 << 14,
			HasExternalFileNameEntry = 1 << 15,
			HasRudderEntry = 1 << 16,
			HasAimingFlagEntry = 1 << 17,
			HasMouseAimingEntry = 1 << 18,
			HasMouseAimFlippedEntry = 1 << 19,
			HasMouseButtonClickedEntries = 1 << 20,
			HasGamepadControls = 1 << 21,
			HasJoystickControls = 1 << 22,
			HasJoystickButtonClickedEntries = 1 << 23,
			HasJoystickAnalogDeadZoneEntries = 1 << 24,
			HasJoystickAnalogSaturateEntries = 1 << 25,
			HasControllerControls = 1 << 26,
			HasControllerAnalogSensitivityEntries = 1 << 27,
			HasControllerAnalogScaleEntries = 1 << 28,
			HasUseMouseAndJoystickEntries = 1 << 29,
			DoesSupportQuickKick = 1 << 30,
			Default = Supported | Populated | HasLegacyControllerEntries | HasJoystickPortEntry | HasMouseAnalogAxesEntries | HasMouseAnalogScaleEntries | HasMouseDigitalEntries | HasMouseSensitivity | HasExternalFileNameEntry | HasRudderEntry | HasMouseAimingEntry | HasMouseAimFlippedEntry | HasMouseButtonClickedEntries | HasGamepadControls | HasJoystickControls | HasJoystickButtonClickedEntries | DoesSupportQuickKick
		};

		enum class Sound : uint16_t {
			None = 0,
			Supported = 1,
			Populated = 1 << 1,
			HasLegacyEntries = 1 << 2,
			HasDefaultEntries = 1 << 3,
			HasQualityEntries = 1 << 4,
			HasNumberOfChannelsEntry = 1 << 5,
			HasSoundDeviceEntry = 1 << 6,
			HasRandomMusicEntry = 1 << 7,
			HasOpponentSoundToggleEntry = 1 << 8,
			Default = Supported | Populated | HasLegacyEntries | HasQualityEntries | HasNumberOfChannelsEntry | HasSoundDeviceEntry
		};

		enum class Screen : uint32_t {
			None = 0,
			Supported = 1,
			Populated = 1 << 1,
			HasAdvancedGraphicsEntries = 1 << 2,
			HasPolymerSupport = 1 << 3,
			HasAmbientLightEntry = 1 << 4,
			HasUseModelsEntry = 1 << 5,
			HasDefaultEntries = 1 << 6,
			HasAllDefaultEntries = 1 << 7,
			HasShadowsEntry = 1 << 8,
			HasScreenModeEntry = 1 << 9,
			HasCustomEntries = 1 << 10,
			HasEnvironmentEntry = 1 << 11,
			HasShowFramesPerSecondEntry = 1 << 12,
			HasOpenGLAnimationSmoothingEntry = 1 << 13,
			HasOpenGLTextureModeEntry = 1 << 14,
			HasOpenGLUseTextureCompressionEntry = 1 << 15,
			HasOpenGLAnisotropyEntry = 1 << 16,
			HasOpenGLUsePreCacheEntry = 1 << 17,
			HasOpenGLUseCompressedTextureCacheEntry = 1 << 18,
			HasOpenGLUseTextureCacheCompressionEntry = 1 << 19,
			HasOpenGLWideScreenEntry = 1 << 20,
			HasOpenGLFovScreenEntry = 1 << 21,
			HasOpenGLVerticalSyncEntry = 1 << 22,
			HasOpenGLUseHighTileEntry = 1 << 23,
			HasScreenDisplayEntry = 1 << 24,
			HasWindowPropertyEntries = 1 << 25,
			HasFullScreenEntry = 1 << 26,
			HasWideScreenEntries = 1 << 27,
			HasColourFixEntry = 1 << 28,
			HasExtendedScreenSizeEntry = 1 << 29,
			HasLastInputOutputSlotEntry = 1 << 30,
			Default = Supported | Populated | HasAllDefaultEntries | HasShadowsEntry | HasScreenModeEntry
		};

		enum class KeyDefinitions : uint16_t {
			None = 0,
			Supported = 1,
			Populated = 1 << 1,
			HasShowMenuEntry = 1 << 2,
			HasShowConsoleEntry = 1 << 3,
			HasConsoleEntry = 1 << 4,
			HasExtraEntries = 1 << 5,
			HasHideWeaponEntry = 1 << 6,
			HasAutoAimEntry = 1 << 7,
			HasNormalizedKeyPadPrefix = 1 << 8,
			Default = Supported | Populated
		};

		enum class Communication : uint16_t {
			None = 0,
			Supported = 1,
			Populated = 1 << 1,
			HasDialupNetworking = 1 << 2,
			HasCombatMacros = 1 << 3,
			HasRemoteRidiculeFileName = 1 << 4,
			HasPlayerColourEntry = 1 << 5,
			HasIPAddressEntries = 1 << 6,
			HasBotNameEntries = 1 << 7,
			HasEmptyIRQNumber = 1 << 8,
			HasEmptyUARTAddress = 1 << 9,
			HasEmptySocketNumber = 1 << 10,
			Default = Supported | Populated | HasDialupNetworking | HasCombatMacros | HasRemoteRidiculeFileName | HasEmptyIRQNumber | HasEmptyUARTAddress | HasEmptySocketNumber
		};

		enum class Updates : uint8_t {
			None = 0,
			Supported = 1,
			Populated = 1 << 2,
			Default = Populated
		};

		enum class LobbyFilter : uint8_t {
			None = 0,
			Supported = 1,
			Populated = 1 << 2,
			Default = Populated
		};

		enum class Multiplayer : uint8_t {
			None = 0,
			Supported = 1,
			Populated = 1 << 2,
			Default = Populated
		};

		General general = General::Default;
		Setup setup = Setup::Default;
		Misc misc = Misc::Default;
		Controls controls = Controls::Default;
		Sound sound = Sound::Default;
		Screen screen = Screen::Default;
		KeyDefinitions keyDefinitions = KeyDefinitions::Default;
		Communication communication = Communication::Default;
		Updates updates = Updates::Default;
		LobbyFilter lobbyFilter = LobbyFilter::Default;
		Multiplayer multiplayer = Multiplayer::Default;
	};

	static std::string getGameVersionIDFromType(GameVersionType gameVersionType);
	static std::optional<GameVersionType> getGameVersionTypeFromID(std::string_view gameVersionID);
	static std::optional<SectionFeatures> getSectionFeaturesForGameVersion(std::string_view gameVersionID);
	static SectionFeatures getSectionFeaturesForGameVersion(GameVersionType gameVersionType);
	std::optional<SectionFeatures> getSectionFeatures() const;
	std::optional<GameVersionType> determineGameVersionType() const;
	void updateParent();

	Style m_style;
	NewlineType m_newlineType;
	std::string m_filePath;
	EntryMap m_entries;
	SectionMap m_sections;
	std::vector<std::string> m_orderedSectionNames;
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

template <size_t N>
bool GameConfiguration::Section::addMultiStringEntry(const std::string & entryName, const std::array<std::string, N> & values) {
	if(!Entry::isNameValid(entryName)) {
		return false;
	}

	std::shared_ptr<Entry> entry(std::make_shared<Entry>(entryName));
	entry->setMultiStringValue(values);

	if(!entry->isValid(false)) {
		return false;
	}

	return addEntry(entry);
}

template <size_t N>
bool GameConfiguration::Section::setEntryMultiStringValue(const std::string & entryName, const std::array<std::string, N> & values, bool create) {
	bool entryExists = false;
	std::shared_ptr<Entry> entry(getOrCreateEntry(entryName, create, entryExists));

	entry->setMultiStringValue(values);

	if(!entryExists) {
		return addEntry(entry);
	}

	return true;
}

template<>
struct BitmaskOperators<GameConfiguration::Style> {
	static const bool enabled = true;
};

template<>
struct BitmaskOperators<GameConfiguration::NewlineType> {
	static const bool enabled = true;
};

template<>
struct BitmaskOperators<GameConfiguration::SectionFeatures::General> {
	static const bool enabled = true;
};

template<>
struct BitmaskOperators<GameConfiguration::SectionFeatures::Setup> {
	static const bool enabled = true;
};

template<>
struct BitmaskOperators<GameConfiguration::SectionFeatures::Misc> {
	static const bool enabled = true;
};

template<>
struct BitmaskOperators<GameConfiguration::SectionFeatures::Controls> {
	static const bool enabled = true;
};

template<>
struct BitmaskOperators<GameConfiguration::SectionFeatures::Sound> {
	static const bool enabled = true;
};

template<>
struct BitmaskOperators<GameConfiguration::SectionFeatures::Screen> {
	static const bool enabled = true;
};

template<>
struct BitmaskOperators<GameConfiguration::SectionFeatures::KeyDefinitions> {
	static const bool enabled = true;
};

template<>
struct BitmaskOperators<GameConfiguration::SectionFeatures::Communication> {
	static const bool enabled = true;
};

template<>
struct BitmaskOperators<GameConfiguration::SectionFeatures::Updates> {
	static const bool enabled = true;
};

template<>
struct BitmaskOperators<GameConfiguration::SectionFeatures::LobbyFilter> {
	static const bool enabled = true;
};

template<>
struct BitmaskOperators<GameConfiguration::SectionFeatures::Multiplayer> {
	static const bool enabled = true;
};

#endif // _GAME_CONFIGURATION_H_
