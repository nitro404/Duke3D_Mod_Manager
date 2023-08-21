#include "GameConfiguration.h"

#include "Game/GameVersion.h"
#include "Game/File/Group/GRP/GroupGRP.h"

#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <sstream>

static const std::string SCREEN_BITS_PER_PIXEL_ENTRY_NAME("ScreenBPP");
static const std::string SCREEN_MAXIMUM_REFRESH_FREQUENCY_ENTRY_NAME("MaxRefreshFreq");
static const std::string SCREEN_JFDUKE3D_OPENGL_TEXTURE_MODE_ENTRY_NAME("GLTextureMode");
static const std::string SCREEN_JFDUKE3D_OPENGL_ANISOTROPY_ENTRY_NAME("GLAnisotropy");
static const std::string SCREEN_JFDUKE3D_OPENGL_USE_TEXTURE_COMPRESSION_ENTRY_NAME("GLUseTextureCompr");
static const std::string SCREEN_JFDUKE3D_OPENGL_USE_COMPRESSED_TEXTURE_CACHE_ENTRY_NAME("GLUseCompressedTextureCache");
static const std::string SCREEN_EDUKE32_POLYMER_ENTRY_NAME("Polymer");
static const std::string MISC_AUTO_AIM_ENTRY_NAME("AutoAim");
static const std::string MISC_JFDUKE3D_SHOW_LEVEL_STATISTICS_ENTRY_NAME("ShowLevelStats");
static const std::string MISC_JFDUKE3D_STATUS_BAR_SCALE_ENTRY_NAME("StatusBarScale");
static const std::string MISC_JDFUKE3D_SHOW_OPPONENT_WEAPONS_ENTRY_NAME("ShowOpponentWeapons");
static const std::string MISC_JFDUKE3D_USE_PRECACHE_ENTRY_NAME("UsePrecache");
static const std::string SETUP_EDUKE32_CONFIGURATION_VERSION("ConfigVersion");
static const std::string CONTROLS_JFDUKE3D_USE_JOYSTICK_ENTRY_NAME("UseJoystick");
static const std::string CONTROLS_JFDUKE3D_USE_MOUSE_ENTRY_NAME("UseMouse");
static const std::string CONTROLS_JFDUKE3D_RUN_KEY_BEHAVIOUR_ENTRY_NAME("RunKeyBehaviour");
static const std::string CONTROLS_AUTO_AIM_ENTRY_NAME("AutoAim");
static const std::string CONTROLS_JFDUKE3D_WEAPON_SWITCH_MODE_ENTRY_NAME("WeaponSwitchMode");
static const std::string JFDUKE3D_SHOW_MENU_ENTRY_NAME("Show_Menu");
static const std::string JFDUKE3D_SHOW_CONSOLE_ENTRY_NAME("Show_Console");
static const std::string BELGIAN_HIDE_WEAPON_ENTRY_NAME("Hide_Weapon");
static const std::string BELGIAN_AUTO_AIM_ENTRY_NAME("Auto_Aim");
static const std::string BELGIAN_CONSOLE_ENTRY_NAME("Console");
static const std::string BELGIAN_SHOW_FRAMES_PER_SECOND_ENTRY_NAME("ShowFPS");
static const std::string BELGIAN_FULL_SCREEN_ENTRY_NAME("FullScreen");
static const std::string BELGIAN_EXTENDED_SCREEN_SIZE_ENTRY_NAME("ExtScreenSize");
static const std::string BELGIAN_SHOW_CINEMATICS_ENTRY_NAME("ShowCinematics");
static const std::string BELGIAN_OPPONENT_SOUND_TOGGLE_ENTRY_NAME("OpponentSoundToggle");

static const std::string MOUSE_PREFIX("Mouse");
static const std::string GAMEPAD_PREFIX("GamePad");
static const std::string JOYSTICK_PREFIX("Joystick");
static const std::string CONTROLLER_PREFIX("Controller");
static const std::string BUTTON_SUFFIX("Button");
static const std::string CLICKED_SUFFIX("Clicked");
static const std::string ANALOG_SUFFIX("Analog");
static const std::string DIGITAL_SUFFIX("Digital");
static const std::string AXES_SUFFIX("Axes");
static const std::string SCALE_SUFFIX("Scale");
static const std::string SENSITIVITY_SUFFIX("Sensitivity");
static const std::string DEADZONE_SUFFIX("Dead");
static const std::string SATURATE_SUFFIX("Saturate");
static const std::string INVERT_SUFFIX("Invert");
static const std::string WEAPON_KEY_DEFINITION_ENTRY_NAME_PREFIX("Weapon_");
static const std::string COMBAT_MACRO_ENTRY_NAME_PREFIX("CommbatMacro#");
static const std::string PHONE_NAME_ENTRY_NAME_PREFIX("PhoneName#");
static const std::string PHONE_NUMBER_ENTRY_NAME_PREFIX("PhoneNumber#");
static const std::string WEAPON_CHOICE_PREFIX("WeaponChoice");
static const std::string RANCID_MEAT_SUFFIX("Rancid");

static constexpr uint16_t DEFAULT_JFDUKE3D_JOYSTICK_ANALOG_DEADZONE = 1024;
static constexpr uint32_t DEFAULT_JFDUKE3D_JOYSTICK_ANALOG_SATURATE = 31743;
static constexpr uint8_t DEFAULT_CONTROLLER_ANALOG_SENSITIVITY = 5;
static constexpr uint16_t DEFAULT_CONTROLLER_ANALOG_DEADZONE = 1000;
static constexpr uint16_t DEFAULT_CONTROLLER_ANALOG_SATURATE = 9500;

static const std::array<uint8_t, 10> DEFAULT_WEAPON_CHOICE_PREFERENCES({
	3, 4, 5, 7, 8, 6, 0, 2, 9, 1
});

static const std::vector<std::string> DEFAULT_MOUSE_BUTTON_ACTIONS({
	GameConfiguration::FIRE_ENTRY_NAME,
	GameConfiguration::STRAFE_ENTRY_NAME,
	GameConfiguration::MOVE_FORWARD_ENTRY_NAME,
});

static const std::vector<std::string> DEFAULT_EDUKE32_MOUSE_BUTTON_ACTIONS({
	GameConfiguration::FIRE_ENTRY_NAME,
	GameConfiguration::ALT_FIRE_ENTRY_NAME,
	GameConfiguration::MEDKIT_ENTRY_NAME,
	"",
	GameConfiguration::PREVIOUS_WEAPON_ENTRY_NAME,
	GameConfiguration::NEXT_WEAPON_ENTRY_NAME
});

static const std::array<std::string, 3> DEFAULT_MOUSE_BUTTON_CLICKED_ACTIONS({
	"",
	GameConfiguration::OPEN_ENTRY_NAME,
	""
});

static const std::array<std::string, 3> DEFAULT_EDUKE32_MOUSE_BUTTON_CLICKED_ACTIONS({
	"",
	"",
	""
});

static const std::array<std::string, 2> DEFAULT_MOUSE_ANALOG_AXES_ACTIONS({
	"analog_turning",
	"analog_moving"
});

static const std::array<std::pair<std::string, std::string>, 2> DEFAULT_MOUSE_DIGITAL_AXES_ACTIONS({
	std::make_pair("", ""),
	std::make_pair("", "")
});

static const std::array<std::pair<std::string, std::string>, 2> GAMEPAD_DIGITAL_AXES_ACTIONS({
	std::make_pair(GameConfiguration::TURN_LEFT_ENTRY_NAME,    GameConfiguration::TURN_RIGHT_ENTRY_NAME),
	std::make_pair(GameConfiguration::MOVE_FORWARD_ENTRY_NAME, GameConfiguration::MOVE_BACKWARD_ENTRY_NAME)
});

static const std::array<std::string, 4> DEFAULT_JOYSTICK_ANALOG_AXES_ACTIONS({
	"analog_turning",
	"analog_moving",
	"analog_strafing",
	""
});

static const std::array<std::pair<std::string, std::string>, 4> DEFAULT_JOYSTICK_DIGITAL_AXES_ACTIONS({
	std::make_pair("",    ""),
	std::make_pair("",    ""),
	std::make_pair("",    ""),
	std::make_pair("Run", "")
});

static const std::vector<std::pair<std::string, std::string>> DEFAULT_JOYSTICK_BUTTON_ACTIONS({
	{ GameConfiguration::FIRE_ENTRY_NAME,       "" },
	{ GameConfiguration::STRAFE_ENTRY_NAME,     GameConfiguration::INVENTORY_ENTRY_NAME },
	{ GameConfiguration::RUN_ENTRY_NAME,        GameConfiguration::JUMP_ENTRY_NAME },
	{ GameConfiguration::OPEN_ENTRY_NAME,       GameConfiguration::CROUCH_ENTRY_NAME },
	{ GameConfiguration::AIM_DOWN_ENTRY_NAME,   ""},
	{ GameConfiguration::LOOK_RIGHT_ENTRY_NAME, "" },
	{ GameConfiguration::AIM_UP_ENTRY_NAME,     "" },
	{ GameConfiguration::LOOK_LEFT_ENTRY_NAME,  "" }
});

static const std::vector<std::pair<std::string, std::string>> DEFAULT_JDFUKE3D_JOYSTICK_BUTTON_ACTIONS({
	DEFAULT_JOYSTICK_BUTTON_ACTIONS[0],
	DEFAULT_JOYSTICK_BUTTON_ACTIONS[1],
	DEFAULT_JOYSTICK_BUTTON_ACTIONS[2],
	DEFAULT_JOYSTICK_BUTTON_ACTIONS[3],
	{ "",                                       "" },
	{ "",                                       "" },
	{ JFDUKE3D_SHOW_MENU_ENTRY_NAME,            "" },
	{ "",                                       "" },
	{ "",                                       "" },
	{ "",                                       "" },
	{ "",                                       "" },
	{ GameConfiguration::AIM_UP_ENTRY_NAME,     "" },
	{ GameConfiguration::AIM_DOWN_ENTRY_NAME,   "" },
	{ GameConfiguration::LOOK_LEFT_ENTRY_NAME,  "" },
	{ GameConfiguration::LOOK_RIGHT_ENTRY_NAME, "" },
	{ "",                                       "" },
	{ "",                                       "" },
	{ "",                                       "" },
	{ "",                                       "" },
	{ "",                                       "" },
	{ "",                                       "" },
	{ "",                                       "" },
	{ "",                                       "" },
	{ "",                                       "" },
	{ "",                                       "" },
	{ "",                                       "" },
	{ "",                                       "" },
	{ "",                                       "" },
	{ "",                                       "" },
	{ "",                                       "" },
	{ "",                                       "" },
	{ "",                                       "" }
});

const std::array<std::string, 16> DEFAULT_CONTROLLER_BUTTON_ACTIONS({
	GameConfiguration::OPEN_ENTRY_NAME,
	GameConfiguration::INVENTORY_ENTRY_NAME,
	GameConfiguration::INVENTORY_ENTRY_NAME,
	GameConfiguration::QUICK_KICK_ENTRY_NAME,
	GameConfiguration::MAP_ENTRY_NAME,
	"",
	"",
	GameConfiguration::RUN_ENTRY_NAME,
	GameConfiguration::TOGGLE_CROUCH_ENTRY_NAME,
	GameConfiguration::CROUCH_ENTRY_NAME,
	GameConfiguration::ALT_FIRE_ENTRY_NAME,
	GameConfiguration::PREVIOUS_WEAPON_ENTRY_NAME,
	GameConfiguration::NEXT_WEAPON_ENTRY_NAME,
	GameConfiguration::INVENTORY_LEFT_ENTRY_NAME,
	GameConfiguration::INVENTORY_RIGHT_ENTRY_NAME,
	GameConfiguration::THIRD_PERSON_VIEW_ENTRY_NAME
});

static const std::array<std::string, 4> DEFAULT_CONTROLLER_ANALOG_AXES_ACTIONS({
	"analog_strafing",
	"analog_moving",
	"analog_turning",
	"analog_lookingupanddown"
});

static const std::array<std::pair<std::string, std::string>, 6> DEFAULT_CONTROLLER_DIGITAL_AXES_ACTIONS({
	std::make_pair("", ""),
	std::make_pair("", ""),
	std::make_pair("", ""),
	std::make_pair("", ""),
	std::make_pair("", GameConfiguration::JUMP_ENTRY_NAME),
	std::make_pair("", GameConfiguration::FIRE_ENTRY_NAME)
});

const std::array<std::string, 10> DEFAULT_EDUKE32_COMBAT_MACROS({
	"An inspiration for birth control.",
	"You're gonna die for that!",
	"It hurts to be you.",
	"Lucky son of a bitch.",
	"Hmmm... payback time.",
	"You bottom dwelling scum sucker.",
	"Damn, you're ugly.",
	"Ha ha ha... wasted!",
	"You suck!",
	"AARRRGHHHHH!!!"
});

bool GameConfiguration::determineGameVersion(bool & isBeta, bool & isRegularVersion, bool & isAtomicEdition, bool & isJFDuke3D, bool & isEDuke32, bool & isBelgian) {
	// JFDuke3D / eDuke32
	std::shared_ptr<Section> screenSetupSection(getSectionWithName(SCREEN_SETUP_SECTION_NAME));

	if(screenSetupSection != nullptr) {
		// Belgian Chocolate
		if(screenSetupSection->hasEntryWithName(BELGIAN_SHOW_FRAMES_PER_SECOND_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(BELGIAN_FULL_SCREEN_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(BELGIAN_EXTENDED_SCREEN_SIZE_ENTRY_NAME)) {

			std::shared_ptr<Section> miscSection(getSectionWithName(MISC_SECTION_NAME));

			if(miscSection != nullptr &&
			   miscSection->hasEntryWithName(BELGIAN_SHOW_CINEMATICS_ENTRY_NAME)) {

				isBelgian = true;

				return true;
			}
		}

		// eDuke32
		if(screenSetupSection->hasEntryWithName(SCREEN_BITS_PER_PIXEL_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(SCREEN_MAXIMUM_REFRESH_FREQUENCY_ENTRY_NAME)) {

			if(screenSetupSection->hasEntryWithName(SCREEN_EDUKE32_POLYMER_ENTRY_NAME)) {
				isEDuke32 = true;

				return true;
			}

			// JFDuke3D
			if(screenSetupSection->hasEntryWithName(SCREEN_JFDUKE3D_OPENGL_TEXTURE_MODE_ENTRY_NAME) &&
			   screenSetupSection->hasEntryWithName(SCREEN_JFDUKE3D_OPENGL_ANISOTROPY_ENTRY_NAME) &&
			   screenSetupSection->hasEntryWithName(SCREEN_JFDUKE3D_OPENGL_USE_TEXTURE_COMPRESSION_ENTRY_NAME) &&
			   screenSetupSection->hasEntryWithName(SCREEN_JFDUKE3D_OPENGL_USE_COMPRESSED_TEXTURE_CACHE_ENTRY_NAME)) {

				std::shared_ptr<Section> keyDefinitionsSection(getSectionWithName(KEY_DEFINITIONS_SECTION_NAME));

				if(keyDefinitionsSection != nullptr &&
				   keyDefinitionsSection->hasEntryWithName(JFDUKE3D_SHOW_MENU_ENTRY_NAME) &&
				   keyDefinitionsSection->hasEntryWithName(JFDUKE3D_SHOW_CONSOLE_ENTRY_NAME)) {

					std::shared_ptr<Section> controlsSection(getSectionWithName(CONTROLS_SECTION_NAME));

					if(controlsSection != nullptr &&
					   controlsSection->hasEntryWithName(CONTROLS_JFDUKE3D_USE_JOYSTICK_ENTRY_NAME) &&
					   controlsSection->hasEntryWithName(CONTROLS_JFDUKE3D_USE_MOUSE_ENTRY_NAME) &&
					   controlsSection->hasEntryWithName(CONTROLS_JFDUKE3D_RUN_KEY_BEHAVIOUR_ENTRY_NAME) &&
					   controlsSection->hasEntryWithName(CONTROLS_JFDUKE3D_WEAPON_SWITCH_MODE_ENTRY_NAME)) {

						std::shared_ptr<Section> miscSection(getSectionWithName(MISC_SECTION_NAME));

						if(miscSection != nullptr &&
						   miscSection->hasEntryWithName(MISC_JFDUKE3D_SHOW_LEVEL_STATISTICS_ENTRY_NAME) &&
						   miscSection->hasEntryWithName(MISC_JFDUKE3D_STATUS_BAR_SCALE_ENTRY_NAME) &&
						   miscSection->hasEntryWithName(MISC_JDFUKE3D_SHOW_OPPONENT_WEAPONS_ENTRY_NAME) &&
						   miscSection->hasEntryWithName(MISC_JFDUKE3D_USE_PRECACHE_ENTRY_NAME)) {

							isJFDuke3D = true;

							return true;
						}
					}
				}
			}
		}
	}

	std::shared_ptr<Section> setupSection(getSectionWithName(SETUP_SECTION_NAME));

	if(setupSection == nullptr) {
		return false;
	}

	// Beta 0.99
	std::shared_ptr<Entry> setupVersionEntry(setupSection->getEntryWithName(SETUP_VERSION_ENTRY_NAME));

	if(setupVersionEntry == nullptr || !setupVersionEntry->isString()) {
		isBeta = true;

		return true;
	}

	// 1.3D / 1.4 / 1.5
	if(Utilities::areStringsEqual(setupVersionEntry->getStringValue(), REGULAR_VERSION_SETUP_VERSION)) {
		isRegularVersion = true;
	}
	else if(Utilities::areStringsEqual(setupVersionEntry->getStringValue(), ATOMIC_EDITION_SETUP_VERSION)) {
		isAtomicEdition = true;
	}
	else {
		spdlog::warn("Unexpected setup version '{}', expected '{}' or '{}'. Assuming game configuration version is for Atomic Edition / Plutonium Pak.", setupVersionEntry->getStringValue(), REGULAR_VERSION_SETUP_VERSION, ATOMIC_EDITION_SETUP_VERSION);

		isAtomicEdition = true;
	}

	return true;
}

std::unique_ptr<GameConfiguration> GameConfiguration::generateDefaultGameConfiguration(const std::string & gameVersionID) {
	bool isBeta = Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_BETA_VERSION.getID());
	bool isRegularVersion = Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_REGULAR_VERSION.getID());
	bool isAtomicEdition = Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_PLUTONIUM_PAK.getID()) || Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_ATOMIC_EDITION.getID());
	bool isJFDuke3D = Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::JFDUKE3D.getID());
	bool isEDuke32 = Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::EDUKE32.getID());
	bool isBelgian = Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::BELGIAN_CHOCOLATE_DUKE3D.getID());

	// determine game version specific configuration features
	bool hasComments = !isJFDuke3D && !isEDuke32 && !isBelgian;
	bool hasSetupSection = !isBelgian;
	bool hasMiscSection = isJFDuke3D || isEDuke32 || isBelgian;
	bool hasDefaultMiscSectionEntries = hasMiscSection && (isJFDuke3D || isEDuke32 || isBelgian);
	bool hasHeadsUpDisplayEntries = hasDefaultMiscSectionEntries && !isEDuke32;
	bool hasControlsAutoAimEntry = isJFDuke3D;
	bool hasMiscAutoAimEntry = isBelgian;
	bool hasShowCinematicsEntry = isBelgian;
	bool hasWeaponVisibilityEntries = isBelgian;
	bool hasWeaponAutoSwitchEntry = isBelgian;
	bool hasWeaponChoicePreferences = isJFDuke3D || isBelgian;
	bool hasForceSetupEntry = isJFDuke3D || isEDuke32;
	bool hasModLoadingSupport = isEDuke32;
	bool hasSoundSection = !isEDuke32;
	bool hasLegacyAudioEntries = hasSoundSection && !isJFDuke3D && !isEDuke32 && !isBelgian;
	bool hasDefaultAudioEntries = hasSoundSection && (isJFDuke3D || isBelgian);
	bool hasAudioQualityEntries = !isBelgian;
	bool shouldAddDefaultScreenSize = !isEDuke32;
	bool hasAdvancedGraphicsEntries = isJFDuke3D || isEDuke32;
	bool hasPolymerSupport = hasAdvancedGraphicsEntries && isEDuke32;
	bool hasDefaultScreenEntries = isJFDuke3D || isEDuke32 || isBelgian;
	bool hasAllDefaultScreenEntries = hasDefaultScreenEntries && !isEDuke32;
	bool hasScreenModeEntry = !isBelgian;
	bool hasShowFramesPerSecondEntry = isBelgian;
	bool hasFullScreenEntry = isBelgian;
	bool hasExtendedScreenSizeEntry = isBelgian;
	bool hasKeyDefinitions = !isEDuke32;
	bool hasLegacyControllerEntries = !isEDuke32;
	bool hasJoystickPortEntry = hasLegacyControllerEntries && !isBelgian;
	bool hasMouseAnalogEntries = !isBelgian;
	bool hasGameMouseAimingEntry = isBelgian;
	bool hasMouseSensitivity = !isEDuke32 && !isBelgian;
	bool hasExternalFileNameEntry = !isJFDuke3D && !isEDuke32 && !isBelgian;
	bool hasRudderEntry = !isJFDuke3D && !isEDuke32 && !isBelgian;
	bool hasDefaultControls = isJFDuke3D || isBelgian;
	bool hasMouseAimingEntry = !isEDuke32;
	bool hasMouseAimFlippingEntry = !isBeta && !isRegularVersion && !isEDuke32;
	bool hasMouseButtonClickedEntries = !isBelgian;
	bool hasGamepadControls = !isJFDuke3D && !isEDuke32 && !isBelgian;
	bool hasJoystickControls = !isEDuke32 && !isBelgian;
	bool hasControllerControls = isEDuke32;
	bool hasNoramlizedKeyPadPrefix = isJFDuke3D || isBelgian;
	bool hasDialupNetworking = !isJFDuke3D && !isEDuke32 && !isBelgian;
	bool hasCombatMacros = !isJFDuke3D;
	bool hasRemoteRidiculeFileName = !isJFDuke3D;
	bool hasUpdatesSection = isEDuke32;

	// determine game version specific configuration parameters
	GameConfiguration::Style style = GameConfiguration::Style::Default;

	if(isJFDuke3D || isEDuke32 || isBelgian) {
		style |= GameConfiguration::Style::NewlineAfterSections;
	}

	std::string playerName(DEFAULT_PLAYER_NAME);

	if(isJFDuke3D) {
		playerName = "Duke";
	}
	else if(isEDuke32) {
		playerName = "";
	}

	Dimension resolution(DEFAULT_RESOLUTION);

	if(isJFDuke3D || isBelgian) {
		resolution = Dimension(640, 480);
	}

	uint64_t soundDevice = SOUND_DEVICE_UNSET;
	uint64_t musicDevice = MUSIC_DEVICE_UNSET;

	if(isJFDuke3D || isBelgian) {
		soundDevice = 0;
		musicDevice = 0;
	}

	uint8_t soundNumberOfVoices = 8;
	uint8_t soundNumberOfBits = 1;
	uint16_t soundMixRate = 11000;

	if(isJFDuke3D) {
		soundNumberOfVoices = 16;
		soundNumberOfBits = 16;
		soundMixRate = 44100;
	}

	uint8_t screenMode = 2;
	uint8_t screenBitsPerPixel = 1;
	uint8_t screenMaximumRefreshFrequency = 60;
	uint8_t screenGamma = 0;

	if(isJFDuke3D || isEDuke32) {
		screenMode = 1;
	}

	if(isJFDuke3D) {
		screenBitsPerPixel = 8;
	}
	else if(isEDuke32) {
		screenBitsPerPixel = 32;
		screenMaximumRefreshFrequency = 0;
	}

	if(isBelgian) {
		screenGamma = 16;
	}

	uint8_t autoAim = 0;
	bool runModeEnabled = false;
	bool crosshairEnabled = false;

	if(isJFDuke3D) {
		autoAim = 1;
	}
	else if(isBelgian) {
		autoAim = 2;
		runModeEnabled = true;
		crosshairEnabled = true;
	}

	uint8_t numberOfMouseButtons = 3;
	uint8_t numberOfClickableMouseButtons = numberOfMouseButtons;

	if(isJFDuke3D) {
		numberOfMouseButtons = 6;
		numberOfClickableMouseButtons = 4;
	}
	else if(isEDuke32) {
		numberOfMouseButtons = 10;
		numberOfClickableMouseButtons = 8;
	}
	else if(isBelgian) {
		numberOfMouseButtons = 7;
		numberOfClickableMouseButtons = 0;
	}

	uint8_t numberOfJoysticks = 4;

	if(isJFDuke3D) {
		numberOfJoysticks = 12;
	}

	uint8_t numberOfControllerButtons = 0;

	if(isEDuke32) {
		numberOfControllerButtons = 36;
	}

	uint8_t numberOfControllerThumbSticks = 0;

	if(isEDuke32) {
		numberOfControllerThumbSticks = 9;
	}

	size_t numberOfPhoneNumbers = 10;

	if(isAtomicEdition) {
		numberOfPhoneNumbers = 16;
	}

	// create configuration
	std::unique_ptr<GameConfiguration> gameConfiguration(std::make_unique<GameConfiguration>());
	gameConfiguration->setStyle(style);

	if(hasSetupSection) {
		// create setup section
		std::shared_ptr<Section> setupSection(std::make_shared<Section>(SETUP_SECTION_NAME, "", hasComments ? "Setup File for Duke Nukem 3D" : ""));
		gameConfiguration->addSection(setupSection);

		if(isRegularVersion) {
			setupSection->addStringEntry(SETUP_VERSION_ENTRY_NAME, REGULAR_VERSION_SETUP_VERSION);
		}
		else if(isAtomicEdition) {
			setupSection->addStringEntry(SETUP_VERSION_ENTRY_NAME, ATOMIC_EDITION_SETUP_VERSION);
		}

		if(isEDuke32) {
			setupSection->addIntegerEntry(SETUP_EDUKE32_CONFIGURATION_VERSION, 348);
		}

		if(hasForceSetupEntry) {
			setupSection->addIntegerEntry("ForceSetup", 1);
		}

		if(hasModLoadingSupport) {
			setupSection->addIntegerEntry("NoAutoLoad", 1);
			setupSection->addStringEntry("SelectedGRP", GroupGRP::DUKE_NUKEM_3D_GROUP_FILE_NAME);
			setupSection->addStringEntry("ModDir", "/");
		}
	}

	// create misc section
	if(hasMiscSection) {
		std::shared_ptr<Section> miscSection(std::make_shared<Section>(MISC_SECTION_NAME));
		gameConfiguration->addSection(miscSection);

		if(hasDefaultMiscSectionEntries) {
			miscSection->addIntegerEntry(MISC_EXECUTIONS_ENTRY_NAME, 0);

			if(hasHeadsUpDisplayEntries) {
				miscSection->addIntegerEntry(MISC_RUN_MODE_ENTRY_NAME, runModeEnabled ? 1 : 0);
				miscSection->addIntegerEntry(MISC_CROSSHAIRS_ENTRY_NAME, crosshairEnabled ? 1 : 0);
			}
		}

		if(hasShowCinematicsEntry) {
			miscSection->addIntegerEntry(BELGIAN_SHOW_CINEMATICS_ENTRY_NAME, 1);
		}

		if(hasWeaponVisibilityEntries) {
			miscSection->addIntegerEntry("HideWeapon", 0);
			miscSection->addIntegerEntry("ShowWeapon", 0);
		}

		if(hasWeaponAutoSwitchEntry) {
			miscSection->addIntegerEntry("WeaponAutoSwitch", 0);
		}

		if(hasMiscAutoAimEntry) {
			miscSection->addIntegerEntry(MISC_AUTO_AIM_ENTRY_NAME, autoAim);
		}

		if(isJFDuke3D) {
			miscSection->addIntegerEntry(MISC_JFDUKE3D_SHOW_LEVEL_STATISTICS_ENTRY_NAME, 0);
			miscSection->addIntegerEntry(MISC_JFDUKE3D_STATUS_BAR_SCALE_ENTRY_NAME, 8);
			miscSection->addIntegerEntry(MISC_JDFUKE3D_SHOW_OPPONENT_WEAPONS_ENTRY_NAME, 0);
			miscSection->addIntegerEntry(MISC_JFDUKE3D_USE_PRECACHE_ENTRY_NAME, 1);
		}

		if(hasWeaponChoicePreferences) {
			for(size_t i = 0; i < DEFAULT_WEAPON_CHOICE_PREFERENCES.size(); i++) {
				miscSection->addIntegerEntry(fmt::format("{}{}", WEAPON_CHOICE_PREFIX, i), DEFAULT_WEAPON_CHOICE_PREFERENCES[i]);
			}
		}
	}

	// create screen setup section
	std::shared_ptr<Section> screenSetupSection(std::make_shared<Section>(SCREEN_SETUP_SECTION_NAME, hasComments ? " \n " : ""));
	gameConfiguration->addSection(screenSetupSection);

	if(hasComments) {
		std::stringstream screenSetupSectionFollowingCommentsStream;
		screenSetupSectionFollowingCommentsStream << " \n";
		screenSetupSectionFollowingCommentsStream << " \n";
		screenSetupSectionFollowingCommentsStream << SCREEN_MODE_ENTRY_NAME << "\n";
		screenSetupSectionFollowingCommentsStream << " - Chained - 0\n";
		screenSetupSectionFollowingCommentsStream << " - Vesa 2.0 - 1\n";
		screenSetupSectionFollowingCommentsStream << " - Screen Buffered - 2\n";
		screenSetupSectionFollowingCommentsStream << " - Tseng optimized - 3\n";
		screenSetupSectionFollowingCommentsStream << " - Paradise optimized - 4\n";
		screenSetupSectionFollowingCommentsStream << " - S3 optimized - 5\n";
		screenSetupSectionFollowingCommentsStream << " - RedBlue Stereo - 7\n";
		screenSetupSectionFollowingCommentsStream << " - Crystal Eyes - 6\n";
		screenSetupSectionFollowingCommentsStream << " \n";
		screenSetupSectionFollowingCommentsStream << SCREEN_WIDTH_ENTRY_NAME << " passed to engine\n";
		screenSetupSectionFollowingCommentsStream << " \n";
		screenSetupSectionFollowingCommentsStream << SCREEN_HEIGHT_ENTRY_NAME << " passed to engine\n";
		screenSetupSectionFollowingCommentsStream << " \n";
		screenSetupSectionFollowingCommentsStream << " ";
		screenSetupSection->setFollowingComments(screenSetupSectionFollowingCommentsStream.str());
	}

	if(hasScreenModeEntry) {
		screenSetupSection->addIntegerEntry(SCREEN_MODE_ENTRY_NAME, screenMode);
	}

	if(hasDefaultScreenEntries) {
		if(hasAllDefaultScreenEntries) {
			screenSetupSection->addIntegerEntry(SCREEN_SHADOWS_ENTRY_NAME, 1);
		}

		screenSetupSection->addStringEntry(SCREEN_PASSWORD_ENTRY_NAME, "");

		if(hasAllDefaultScreenEntries) {
			screenSetupSection->addIntegerEntry(SCREEN_DETAIL_ENTRY_NAME, 1);
			screenSetupSection->addIntegerEntry(SCREEN_TILT_ENTRY_NAME, 1);
			screenSetupSection->addIntegerEntry(SCREEN_MESSAGES_ENTRY_NAME, 1);
		}

		screenSetupSection->addIntegerEntry(SCREEN_OUT_ENTRY_NAME, 0);
	}

	if(hasShowFramesPerSecondEntry) {
		screenSetupSection->addIntegerEntry(BELGIAN_SHOW_FRAMES_PER_SECOND_ENTRY_NAME, 0);
	}

	if(shouldAddDefaultScreenSize) {
		screenSetupSection->addIntegerEntry(SCREEN_WIDTH_ENTRY_NAME, resolution.w);
		screenSetupSection->addIntegerEntry(SCREEN_HEIGHT_ENTRY_NAME, resolution.h);
	}

	if(hasAdvancedGraphicsEntries) {
		if(hasPolymerSupport) {
			screenSetupSection->addIntegerEntry(SCREEN_EDUKE32_POLYMER_ENTRY_NAME, 0);
		}

		screenSetupSection->addIntegerEntry(SCREEN_BITS_PER_PIXEL_ENTRY_NAME, screenBitsPerPixel);

		if(isEDuke32) {
			screenSetupSection->addIntegerEntry("ScreenDisplay", 0);
		}

		screenSetupSection->addIntegerEntry(SCREEN_MAXIMUM_REFRESH_FREQUENCY_ENTRY_NAME, screenMaximumRefreshFrequency);
	}

	if(isJFDuke3D) {
		screenSetupSection->addIntegerEntry(SCREEN_JFDUKE3D_OPENGL_TEXTURE_MODE_ENTRY_NAME, 5);
		screenSetupSection->addIntegerEntry(SCREEN_JFDUKE3D_OPENGL_ANISOTROPY_ENTRY_NAME, 0);
		screenSetupSection->addIntegerEntry(SCREEN_JFDUKE3D_OPENGL_USE_TEXTURE_COMPRESSION_ENTRY_NAME, 1);
		screenSetupSection->addIntegerEntry(SCREEN_JFDUKE3D_OPENGL_USE_COMPRESSED_TEXTURE_CACHE_ENTRY_NAME, 1);
	}

	if(hasFullScreenEntry) {
		screenSetupSection->addIntegerEntry(BELGIAN_FULL_SCREEN_ENTRY_NAME, 0);
	}

	if(hasAllDefaultScreenEntries) {
		screenSetupSection->addIntegerEntry(SCREEN_SIZE_ENTRY_NAME, 8);
	}

	if(hasExtendedScreenSizeEntry) {
		screenSetupSection->addIntegerEntry(BELGIAN_EXTENDED_SCREEN_SIZE_ENTRY_NAME, 0);
	}

	if(hasAllDefaultScreenEntries) {
		screenSetupSection->addIntegerEntry(SCREEN_GAMMA_ENTRY_NAME, screenGamma);
	}

	if(hasUpdatesSection) {
		// create updates section
		std::shared_ptr<Section> updatesSection(std::make_shared<Section>("Updates"));
		gameConfiguration->addSection(updatesSection);

		updatesSection->addIntegerEntry("CheckForUpdates", 1);
	}

	if(hasSoundSection) {
		// create sound setup section
		std::shared_ptr<Section> soundSetupSection(std::make_shared<Section>(SOUND_SETUP_SECTION_NAME, hasComments ? " \n " : "", hasComments ? " \n " : ""));
		gameConfiguration->addSection(soundSetupSection);

		soundSetupSection->addIntegerEntry(SOUND_FX_DEVICE_ENTRY_NAME, soundDevice);
		soundSetupSection->addIntegerEntry(SOUND_MUSIC_DEVICE_ENTRY_NAME, musicDevice);
		soundSetupSection->addIntegerEntry(SOUND_FX_VOLUME_ENTRY_NAME, 220);
		soundSetupSection->addIntegerEntry(SOUND_MUSIC_VOLUME_ENTRY_NAME, 200);

		if(hasDefaultAudioEntries) {
			soundSetupSection->addIntegerEntry(SOUND_SOUND_TOGGLE_ENTRY_NAME, 1);
			soundSetupSection->addIntegerEntry(SOUND_VOICE_TOGGLE_ENTRY_NAME, 1);
			soundSetupSection->addIntegerEntry(SOUND_AMBIENCE_TOGGLE_ENTRY_NAME, 1);

			if(isBelgian) {
				soundSetupSection->addIntegerEntry(BELGIAN_OPPONENT_SOUND_TOGGLE_ENTRY_NAME, 1);
			}

			soundSetupSection->addIntegerEntry(SOUND_MUSIC_TOGGLE_ENTRY_NAME, 1);
		}

		if(hasAudioQualityEntries) {
			soundSetupSection->addIntegerEntry("NumVoices", soundNumberOfVoices);
			soundSetupSection->addIntegerEntry("NumChannels", 2);
			soundSetupSection->addIntegerEntry(SOUND_NUM_BITS_ENTRY_NAME, soundNumberOfBits);
			soundSetupSection->addIntegerEntry(SOUND_MIX_RATE_ENTRY_NAME, soundMixRate);
		}

		if(hasLegacyAudioEntries) {
			soundSetupSection->addHexadecimalEntryUsingDecimal("MidiPort", 0x330);
			soundSetupSection->addHexadecimalEntryUsingDecimal("BlasterAddress", 0x220);
			soundSetupSection->addIntegerEntry("BlasterType", 6);
			soundSetupSection->addIntegerEntry("BlasterInterrupt", 7);
			soundSetupSection->addIntegerEntry("BlasterDma8", 1);
			soundSetupSection->addIntegerEntry("BlasterDma16", 5);
			soundSetupSection->addHexadecimalEntryUsingDecimal("BlasterEmu", 0x620);
		}

		soundSetupSection->addIntegerEntry("ReverseStereo", 0);

		if(isJFDuke3D) {
			soundSetupSection->addStringEntry("MusicParams", "");
		}
	}

	// create key definitions section
	if(hasKeyDefinitions) {
		std::shared_ptr<Section> keyDefinitionsSection(std::make_shared<Section>(KEY_DEFINITIONS_SECTION_NAME, hasComments ? " \n " : "", hasComments ? " \n " : ""));
		gameConfiguration->addSection(keyDefinitionsSection);

		keyDefinitionsSection->addMultiStringEntry(MOVE_FORWARD_ENTRY_NAME, "Up", "Kpad8");
		keyDefinitionsSection->addMultiStringEntry(MOVE_BACKWARD_ENTRY_NAME, "Down", "Kpad2");
		keyDefinitionsSection->addMultiStringEntry(TURN_LEFT_ENTRY_NAME, "Left", "Kpad4");
		keyDefinitionsSection->addMultiStringEntry(TURN_RIGHT_ENTRY_NAME, "Right", hasNoramlizedKeyPadPrefix ? "Kpad6" : "KPad6");
		keyDefinitionsSection->addMultiStringEntry(STRAFE_ENTRY_NAME, "LAlt", "RAlt");
		keyDefinitionsSection->addMultiStringEntry(FIRE_ENTRY_NAME, "LCtrl", "RCtrl");
		keyDefinitionsSection->addMultiStringEntry(OPEN_ENTRY_NAME, "Space", "");
		keyDefinitionsSection->addMultiStringEntry(RUN_ENTRY_NAME, "LShift", "RShift");
		keyDefinitionsSection->addMultiStringEntry("AutoRun", "CapLck", "");
		keyDefinitionsSection->addMultiStringEntry(JUMP_ENTRY_NAME, "A", "/");
		keyDefinitionsSection->addMultiStringEntry(CROUCH_ENTRY_NAME, "Z", "");
		keyDefinitionsSection->addMultiStringEntry(LOOK_UP_ENTRY_NAME, "PgUp", "Kpad9");
		keyDefinitionsSection->addMultiStringEntry(LOOK_DOWN_ENTRY_NAME, "PgDn", "Kpad3");
		keyDefinitionsSection->addMultiStringEntry(LOOK_LEFT_ENTRY_NAME, "Insert", "Kpad0");
		keyDefinitionsSection->addMultiStringEntry(LOOK_RIGHT_ENTRY_NAME, "Delete", "Kpad.");
		keyDefinitionsSection->addMultiStringEntry(STRAFE_LEFT_ENTRY_NAME, ",", "");
		keyDefinitionsSection->addMultiStringEntry(STRAFE_RIGHT_ENTRY_NAME, ".", "");
		keyDefinitionsSection->addMultiStringEntry(AIM_UP_ENTRY_NAME, "Home", hasNoramlizedKeyPadPrefix ? "Kpad7" : "KPad7");
		keyDefinitionsSection->addMultiStringEntry(AIM_DOWN_ENTRY_NAME, "End", "Kpad1");

		for(size_t i = 1; i <= 10; i++) {
			keyDefinitionsSection->addMultiStringEntry(fmt::format("{}{}", WEAPON_KEY_DEFINITION_ENTRY_NAME_PREFIX, i), fmt::format("{}", i % 10) , "");
		}

		keyDefinitionsSection->addMultiStringEntry(INVENTORY_ENTRY_NAME, "Enter", "KpdEnt");
		keyDefinitionsSection->addMultiStringEntry(INVENTORY_LEFT_ENTRY_NAME, "[", "");
		keyDefinitionsSection->addMultiStringEntry(INVENTORY_RIGHT_ENTRY_NAME, "]", "");
		keyDefinitionsSection->addMultiStringEntry("Holo_Duke", "H", "");
		keyDefinitionsSection->addMultiStringEntry(JETPACK_ENTRY_NAME, "J", "");
		keyDefinitionsSection->addMultiStringEntry("NightVision", "N", "");
		keyDefinitionsSection->addMultiStringEntry(MEDKIT_ENTRY_NAME, "M", "");
		keyDefinitionsSection->addMultiStringEntry("TurnAround", "BakSpc", "");
		keyDefinitionsSection->addMultiStringEntry("SendMessage", "T", "");
		keyDefinitionsSection->addMultiStringEntry(MAP_ENTRY_NAME, "Tab", "");
		keyDefinitionsSection->addMultiStringEntry("Shrink_Screen", "-", "Kpad-");
		keyDefinitionsSection->addMultiStringEntry("Enlarge_Screen", "=", "Kpad+");
		keyDefinitionsSection->addMultiStringEntry("Center_View", hasNoramlizedKeyPadPrefix ? "Kpad5" : "KPad5", "");
		keyDefinitionsSection->addMultiStringEntry("Holster_Weapon", "ScrLck", "");
		keyDefinitionsSection->addMultiStringEntry(SHOW_OPPONENTS_WEAPON_ENTRY_NAME, "W", "");
		keyDefinitionsSection->addMultiStringEntry(MAP_FOLLOW_MODE_ENTRY_NAME, "F", "");
		keyDefinitionsSection->addMultiStringEntry("See_Coop_View", "K", "");
		keyDefinitionsSection->addMultiStringEntry("Mouse_Aiming", "U", "");
		keyDefinitionsSection->addMultiStringEntry("Toggle_Crosshair", "I", "");
		keyDefinitionsSection->addMultiStringEntry("Steroids", "R", "");
		keyDefinitionsSection->addMultiStringEntry(QUICK_KICK_ENTRY_NAME, isBelgian ? "C" : "`", "");
		keyDefinitionsSection->addMultiStringEntry(NEXT_WEAPON_ENTRY_NAME, "'", "");
		keyDefinitionsSection->addMultiStringEntry(PREVIOUS_WEAPON_ENTRY_NAME, ";", "");

		if(isJFDuke3D) {
			keyDefinitionsSection->addMultiStringEntry(JFDUKE3D_SHOW_MENU_ENTRY_NAME, "", "");
			keyDefinitionsSection->addMultiStringEntry(JFDUKE3D_SHOW_CONSOLE_ENTRY_NAME, "NumLck", "");
		}

		if(isBelgian) {
			keyDefinitionsSection->addMultiStringEntry(BELGIAN_HIDE_WEAPON_ENTRY_NAME, "S", "");
			keyDefinitionsSection->addMultiStringEntry(BELGIAN_AUTO_AIM_ENTRY_NAME, "V", "");
			keyDefinitionsSection->addMultiStringEntry(BELGIAN_CONSOLE_ENTRY_NAME, "`", "");
		}
	}

	// create controls section
	std::shared_ptr<Section> controlsSection(std::make_shared<Section>("Controls", hasComments ? " \n " : ""));
	gameConfiguration->addSection(controlsSection);

	if(hasComments) {
		std::stringstream controlsSectionFollowingCommentsStream;
		controlsSectionFollowingCommentsStream << " \n";
		controlsSectionFollowingCommentsStream << " \n";
		controlsSectionFollowingCommentsStream << CONTROLS_SECTION_NAME << '\n';
		controlsSectionFollowingCommentsStream << " \n";
		controlsSectionFollowingCommentsStream << CONTROLLER_TYPE_ENTRY_NAME << '\n';
		controlsSectionFollowingCommentsStream << " - Keyboard                  - 0\n";
		controlsSectionFollowingCommentsStream << " - Keyboard and Mouse        - 1\n";
		controlsSectionFollowingCommentsStream << " - Keyboard and Joystick     - 2\n";
		controlsSectionFollowingCommentsStream << " - Keyboard and Gamepad      - 4\n";
		controlsSectionFollowingCommentsStream << " - Keyboard and External     - 3\n";
		controlsSectionFollowingCommentsStream << " - Keyboard and FlightStick  - 5\n";
		controlsSectionFollowingCommentsStream << " - Keyboard and ThrustMaster - 6\n";
		controlsSectionFollowingCommentsStream << " \n";
		controlsSectionFollowingCommentsStream << ' ';
		controlsSection->setFollowingComments(controlsSectionFollowingCommentsStream.str());
	}

	if(isJFDuke3D) {
		controlsSection->addIntegerEntry(CONTROLS_JFDUKE3D_USE_JOYSTICK_ENTRY_NAME, 1);
		controlsSection->addIntegerEntry(CONTROLS_JFDUKE3D_USE_MOUSE_ENTRY_NAME, 1);
	}
	else if(hasLegacyControllerEntries) {
		controlsSection->addIntegerEntry(CONTROLLER_TYPE_ENTRY_NAME, 1);

		if(hasJoystickPortEntry) {
			controlsSection->addIntegerEntry("JoystickPort", 0);
		}
	}

	if(hasMouseSensitivity) {
		controlsSection->addIntegerEntry("MouseSensitivity", 32768);
	}

	if(hasExternalFileNameEntry) {
		controlsSection->addStringEntry("ExternalFilename", "EXTERNAL.EXE");
	}

	if(hasRudderEntry) {
		controlsSection->addIntegerEntry("EnableRudder", 0);
	}

	if(hasMouseAimingEntry) {
		controlsSection->addIntegerEntry(MOUSE_AIMING_ENTRY_NAME, 0);
	}

	if(hasGameMouseAimingEntry) {
		controlsSection->addIntegerEntry(GAME_MOUSE_AIMING_ENTRY_NAME, 0);
	}

	if(hasDefaultControls) {
		controlsSection->addIntegerEntry(AIMING_FLAG_ENTRY_NAME, 0);
	}

	if(hasMouseAimFlippingEntry) {
		controlsSection->addIntegerEntry(MOUSE_AIMING_FLIPPED_ENTRY_NAME, 0);
	}

	if(isJFDuke3D) {
		controlsSection->addIntegerEntry(CONTROLS_JFDUKE3D_RUN_KEY_BEHAVIOUR_ENTRY_NAME, 0);
	}

	if(hasControlsAutoAimEntry) {
		controlsSection->addIntegerEntry(CONTROLS_AUTO_AIM_ENTRY_NAME, autoAim);
	}

	if(isJFDuke3D) {
		controlsSection->addIntegerEntry(CONTROLS_JFDUKE3D_WEAPON_SWITCH_MODE_ENTRY_NAME, 3);
	}

	const std::vector<std::string> & mouseButtonActions = isEDuke32 ? DEFAULT_EDUKE32_MOUSE_BUTTON_ACTIONS : DEFAULT_MOUSE_BUTTON_ACTIONS;
	const std::array<std::string, 3> & mouseButtonClickedActions = isEDuke32 ? DEFAULT_EDUKE32_MOUSE_BUTTON_CLICKED_ACTIONS : DEFAULT_MOUSE_BUTTON_CLICKED_ACTIONS;

	for(size_t i = 0; i < numberOfMouseButtons; i++) {
		controlsSection->addStringEntry(fmt::format("{}{}{}", MOUSE_PREFIX, BUTTON_SUFFIX, i), i < mouseButtonActions.size() ? mouseButtonActions[i] : "");

		if(hasMouseButtonClickedEntries && i < numberOfClickableMouseButtons) {
			controlsSection->addStringEntry(fmt::format("{}{}{}{}", MOUSE_PREFIX, BUTTON_SUFFIX, CLICKED_SUFFIX, i), i < mouseButtonClickedActions.size() ? mouseButtonClickedActions[i] : "");
		}
	}

	if(hasJoystickControls) {
		const std::vector<std::pair<std::string, std::string>> * joystickButtonActionList = &DEFAULT_JOYSTICK_BUTTON_ACTIONS;

		if(isJFDuke3D) {
			joystickButtonActionList = &DEFAULT_JDFUKE3D_JOYSTICK_BUTTON_ACTIONS;
		}

		for(size_t i = 0; i < joystickButtonActionList->size(); i++) {
			const std::pair<std::string, std::string> & joystickButtonActions = (*joystickButtonActionList)[i];
			controlsSection->addStringEntry(fmt::format("{}{}{}", JOYSTICK_PREFIX, BUTTON_SUFFIX, i), joystickButtonActions.first);
			controlsSection->addStringEntry(fmt::format("{}{}{}{}", JOYSTICK_PREFIX, BUTTON_SUFFIX, CLICKED_SUFFIX, i), joystickButtonActions.second);
		}
	}

	for(size_t i = 0; i < 2; i++) {
		if(hasMouseAnalogEntries) {
			controlsSection->addStringEntry(fmt::format("{}{}{}{}", MOUSE_PREFIX, ANALOG_SUFFIX, AXES_SUFFIX, i), DEFAULT_MOUSE_ANALOG_AXES_ACTIONS[i]);
		}

		for(size_t j = 0; j < 2; j++) {
			std::string mouseDigitalAxesAction;

			if(i < DEFAULT_MOUSE_DIGITAL_AXES_ACTIONS.size()) {
				mouseDigitalAxesAction = j == 0 ? DEFAULT_MOUSE_DIGITAL_AXES_ACTIONS[i].first : DEFAULT_MOUSE_DIGITAL_AXES_ACTIONS[i].second;
			}

			controlsSection->addStringEntry(fmt::format("{}{}{}{}_{}", MOUSE_PREFIX, DIGITAL_SUFFIX, AXES_SUFFIX, i, j), mouseDigitalAxesAction);
		}

		if(hasMouseAnalogEntries) {
			controlsSection->addIntegerEntry(fmt::format("{}{}{}{}", MOUSE_PREFIX, ANALOG_SUFFIX, SCALE_SUFFIX, i), DEFAULT_ANALOG_SCALE);
		}
	}

	if(isBelgian) {
		for(uint8_t i = 0; i < 2; i++) {
			controlsSection->addIntegerEntry(fmt::format("{}{}_{}_{}", MOUSE_PREFIX, SENSITIVITY_SUFFIX, static_cast<char>('X' + i), RANCID_MEAT_SUFFIX), 16);
		}
	}

	if(hasJoystickControls) {
		for(size_t i = 0; i < numberOfJoysticks; i++) {
			controlsSection->addStringEntry(fmt::format("{}{}{}{}", JOYSTICK_PREFIX, ANALOG_SUFFIX, AXES_SUFFIX, i), i < DEFAULT_JOYSTICK_ANALOG_AXES_ACTIONS.size() ? DEFAULT_JOYSTICK_ANALOG_AXES_ACTIONS[i] : "");

			for(uint8_t j = 0; j < 2; j++) {
				std::string joystickDigitalAxesAction;

				if(i < DEFAULT_JOYSTICK_DIGITAL_AXES_ACTIONS.size()) {
					joystickDigitalAxesAction = j == 0 ? DEFAULT_JOYSTICK_DIGITAL_AXES_ACTIONS[i].first : DEFAULT_JOYSTICK_DIGITAL_AXES_ACTIONS[i].second;
				}

				controlsSection->addStringEntry(fmt::format("{}{}{}{}_{}", JOYSTICK_PREFIX, DIGITAL_SUFFIX, AXES_SUFFIX, i, j), joystickDigitalAxesAction);
			}

			controlsSection->addIntegerEntry(fmt::format("{}{}{}{}", JOYSTICK_PREFIX, ANALOG_SUFFIX, SCALE_SUFFIX, i), DEFAULT_ANALOG_SCALE);

			if(isJFDuke3D) {
				controlsSection->addIntegerEntry(fmt::format("{}{}{}{}", JOYSTICK_PREFIX, ANALOG_SUFFIX, DEADZONE_SUFFIX, i), DEFAULT_JFDUKE3D_JOYSTICK_ANALOG_DEADZONE);
				controlsSection->addIntegerEntry(fmt::format("{}{}{}{}", JOYSTICK_PREFIX, ANALOG_SUFFIX, SATURATE_SUFFIX, i), DEFAULT_JFDUKE3D_JOYSTICK_ANALOG_SATURATE);
			}
		}
	}

	if(hasGamepadControls) {
		for(size_t i = 0; i < GAMEPAD_DIGITAL_AXES_ACTIONS.size(); i++) {
			for(uint8_t j = 0; j < 2; j++) {
				controlsSection->addStringEntry(fmt::format("{}{}{}{}_{}", GAMEPAD_PREFIX, DIGITAL_SUFFIX, AXES_SUFFIX, i, j), j == 0 ? GAMEPAD_DIGITAL_AXES_ACTIONS[i].first : GAMEPAD_DIGITAL_AXES_ACTIONS[i].second);
			}
		}
	}

	if(hasControllerControls) {
		for(size_t i = 0; i < numberOfControllerButtons; i++) {
			controlsSection->addStringEntry(fmt::format("{}{}{}", CONTROLLER_PREFIX, BUTTON_SUFFIX, i), i < DEFAULT_CONTROLLER_BUTTON_ACTIONS.size() ? DEFAULT_CONTROLLER_BUTTON_ACTIONS[i] : "");
			controlsSection->addStringEntry(fmt::format("{}{}{}{}", CONTROLLER_PREFIX, BUTTON_SUFFIX, CLICKED_SUFFIX, i), "");
		}

		for(size_t i = 0; i < numberOfControllerThumbSticks; i++) {
			controlsSection->addStringEntry(fmt::format("{}{}{}{}", CONTROLLER_PREFIX, ANALOG_SUFFIX, AXES_SUFFIX, i), i < DEFAULT_CONTROLLER_ANALOG_AXES_ACTIONS.size() ? DEFAULT_CONTROLLER_ANALOG_AXES_ACTIONS[i] : "");

			for(uint8_t j = 0; j < 2; j++) {
				std::string controllerDigitalAxesAction;

				if(i < DEFAULT_CONTROLLER_DIGITAL_AXES_ACTIONS.size()) {
					controllerDigitalAxesAction = j == 0 ? DEFAULT_CONTROLLER_DIGITAL_AXES_ACTIONS[i].first : DEFAULT_CONTROLLER_DIGITAL_AXES_ACTIONS[i].second;
				}

				controlsSection->addStringEntry(fmt::format("{}{}{}{}_{}", CONTROLLER_PREFIX, DIGITAL_SUFFIX, AXES_SUFFIX, i, j), controllerDigitalAxesAction);
			}

			controlsSection->addIntegerEntry(fmt::format("{}{}{}{}", CONTROLLER_PREFIX, ANALOG_SUFFIX, SENSITIVITY_SUFFIX, i), DEFAULT_CONTROLLER_ANALOG_SENSITIVITY);
			controlsSection->addIntegerEntry(fmt::format("{}{}{}{}", CONTROLLER_PREFIX, ANALOG_SUFFIX, INVERT_SUFFIX, i), 0);
			controlsSection->addIntegerEntry(fmt::format("{}{}{}{}", CONTROLLER_PREFIX, ANALOG_SUFFIX, DEADZONE_SUFFIX, i), DEFAULT_CONTROLLER_ANALOG_DEADZONE);
			controlsSection->addIntegerEntry(fmt::format("{}{}{}{}", CONTROLLER_PREFIX, ANALOG_SUFFIX, SATURATE_SUFFIX, i), DEFAULT_CONTROLLER_ANALOG_SATURATE);
		}
	}

	// create communication setup section
	std::shared_ptr<Section> communicationSetupSection(std::make_shared<Section>("Comm Setup", hasComments ? " \n " : "", hasComments ? " \n " : ""));
	gameConfiguration->addSection(communicationSetupSection);

	if(hasDialupNetworking) {
		communicationSetupSection->addIntegerEntry("ComPort", 2);
		communicationSetupSection->addEmptyEntry("IrqNumber");
		communicationSetupSection->addEmptyEntry("UartAddress");
		communicationSetupSection->addIntegerEntry("PortSpeed", 9600);
		communicationSetupSection->addIntegerEntry("ToneDial", 1);
		communicationSetupSection->addEmptyEntry("SocketNumber");
		communicationSetupSection->addIntegerEntry("NumberPlayers", 2);
		communicationSetupSection->addStringEntry("ModemName", "");
		communicationSetupSection->addStringEntry("InitString", "ATZ");
		communicationSetupSection->addStringEntry("HangupString", "ATH0=0");
		communicationSetupSection->addStringEntry("DialoutString", "");
	}

	communicationSetupSection->addStringEntry("PlayerName", playerName);

	if(hasRemoteRidiculeFileName) {
		communicationSetupSection->addStringEntry("RTSName", "DUKE.RTS");
	}

	if(isAtomicEdition) {
		communicationSetupSection->addStringEntry("RTSPath", ".\\");
		communicationSetupSection->addStringEntry("UserPath", ".\\");
	}

	if(hasDialupNetworking) {
		communicationSetupSection->addStringEntry("PhoneNumber", "");
		communicationSetupSection->addIntegerEntry("ConnectType", 0);
	}

	if(hasCombatMacros) {
		for(size_t i = 0; i < DEFAULT_COMBAT_MACROS.size(); i++) {
			communicationSetupSection->addStringEntry(fmt::format("{}{}", COMBAT_MACRO_ENTRY_NAME_PREFIX, i), isEDuke32 ? DEFAULT_EDUKE32_COMBAT_MACROS[i] : DEFAULT_COMBAT_MACROS[i]);
		}
	}

	if(hasDialupNetworking) {
		for(size_t i = 0; i < numberOfPhoneNumbers; i++) {
			communicationSetupSection->addStringEntry(fmt::format("{}{}", PHONE_NAME_ENTRY_NAME_PREFIX, i), "");
			communicationSetupSection->addStringEntry(fmt::format("{}{}", PHONE_NUMBER_ENTRY_NAME_PREFIX, i), "");
		}
	}

	return gameConfiguration;
}

bool GameConfiguration::updateForDOSBox() {
	bool isBeta = false;
	bool isRegularVersion = false;
	bool isAtomicEdition = false;
	bool isJFDuke3D = false;
	bool isEDuke32 = false;
	bool isBelgian = false;

	if(!determineGameVersion(isBeta, isRegularVersion, isAtomicEdition, isJFDuke3D, isEDuke32, isBelgian)) {
		return false;
	}

	std::shared_ptr<Section> screenSetupSection(getSectionWithName(SCREEN_SETUP_SECTION_NAME));

	if(screenSetupSection == nullptr) {
		return false;
	}

	std::shared_ptr<Section> soundSetupSection(getSectionWithName(SOUND_SETUP_SECTION_NAME));

	if(soundSetupSection == nullptr) {
		return false;
	}

	if(!screenSetupSection->setEntryIntegerValue(SCREEN_MODE_ENTRY_NAME, 1, true)) { return false; }
	if(!screenSetupSection->setEntryIntegerValue(SCREEN_WIDTH_ENTRY_NAME, 800, true)) { return false; }
	if(!screenSetupSection->setEntryIntegerValue(SCREEN_HEIGHT_ENTRY_NAME, 600, true)) { return false; }
	if(!screenSetupSection->setEntryIntegerValue(SCREEN_SHADOWS_ENTRY_NAME, 1, true)) { return false; }

	if(isAtomicEdition) {
		if(!screenSetupSection->setEntryStringValue(SCREEN_PASSWORD_ENTRY_NAME, "", true)) { return false; }
	}
	else {
		if(!screenSetupSection->setEntryStringValue("Environment", "", true)) { return false; }
	}

	if(!screenSetupSection->setEntryIntegerValue(SCREEN_DETAIL_ENTRY_NAME, 1, true)) { return false; }
	if(!screenSetupSection->setEntryIntegerValue(SCREEN_TILT_ENTRY_NAME, 1, true)) { return false; }
	if(!screenSetupSection->setEntryIntegerValue(SCREEN_MESSAGES_ENTRY_NAME, 1, true)) { return false; }
	if(!screenSetupSection->setEntryIntegerValue(SCREEN_OUT_ENTRY_NAME, 0, true)) { return false; }
	if(isBeta) {
		if(!screenSetupSection->setEntryIntegerValue("LastIOSlot", 0, true)) { return false; }
	}
	if(!screenSetupSection->setEntryIntegerValue(SCREEN_SIZE_ENTRY_NAME, 4, true)) { return false; }
	if(!screenSetupSection->setEntryIntegerValue(SCREEN_GAMMA_ENTRY_NAME, 0, true)) { return false; }

	if(!soundSetupSection->setEntryIntegerValue(SOUND_FX_DEVICE_ENTRY_NAME, 0, true)) { return false; }
	if(!soundSetupSection->setEntryIntegerValue(SOUND_MUSIC_DEVICE_ENTRY_NAME, 0, true)) { return false; }
	if(!soundSetupSection->setEntryIntegerValue(SOUND_NUM_BITS_ENTRY_NAME, 16, true)) { return false; }
	if(!soundSetupSection->setEntryIntegerValue(SOUND_MIX_RATE_ENTRY_NAME, isAtomicEdition ? 44000 : 22000, true)) { return false; }
	if(!soundSetupSection->setEntryIntegerValue(SOUND_SOUND_TOGGLE_ENTRY_NAME, 1, true)) { return false; }
	if(!soundSetupSection->setEntryIntegerValue(SOUND_VOICE_TOGGLE_ENTRY_NAME, 1, true)) { return false; }
	if(!soundSetupSection->setEntryIntegerValue(SOUND_AMBIENCE_TOGGLE_ENTRY_NAME, 1, true)) { return false; }
	if(!soundSetupSection->setEntryIntegerValue(SOUND_MUSIC_TOGGLE_ENTRY_NAME, 1, true)) { return false; }

	return true;
}

bool GameConfiguration::updateWithBetterControls() {
	bool isBeta = false;
	bool isRegularVersion = false;
	bool isAtomicEdition = false;
	bool isJFDuke3D = false;
	bool isEDuke32 = false;
	bool isBelgian = false;

	if(!determineGameVersion(isBeta, isRegularVersion, isAtomicEdition, isJFDuke3D, isEDuke32, isBelgian)) {
		return false;
	}

	std::shared_ptr<Section> keyDefinitionsSection(getSectionWithName(KEY_DEFINITIONS_SECTION_NAME));

	if(keyDefinitionsSection != nullptr) {
		if(!keyDefinitionsSection->setEntryMultiStringValue(MOVE_FORWARD_ENTRY_NAME, "W", 0, true, true)) { return false; }
		if(!keyDefinitionsSection->setEntryMultiStringValue(MOVE_BACKWARD_ENTRY_NAME, "S", 0, true, true)) { return false; }
		if(!keyDefinitionsSection->setEntryMultiStringValue(TURN_LEFT_ENTRY_NAME, "", 0, true, true)) { return false; }
		if(!keyDefinitionsSection->setEntryMultiStringValue(TURN_RIGHT_ENTRY_NAME, "", 0, true, true)) { return false; }
		if(!keyDefinitionsSection->setEntryMultiStringValue(FIRE_ENTRY_NAME, "", 0, true, true)) { return false; }
		if(!keyDefinitionsSection->setEntryMultiStringValue(OPEN_ENTRY_NAME, "E", "F", true)) { return false; }
		if(!keyDefinitionsSection->setEntryMultiStringValue(JUMP_ENTRY_NAME, "Space", 0, true, true)) { return false; }
		if(!keyDefinitionsSection->setEntryMultiStringValue(CROUCH_ENTRY_NAME, "LCtrl", 1, true, true)) { return false; }
		if(!keyDefinitionsSection->setEntryMultiStringValue(STRAFE_LEFT_ENTRY_NAME, "A", 1, true, true)) { return false; }
		if(!keyDefinitionsSection->setEntryMultiStringValue(STRAFE_RIGHT_ENTRY_NAME, "D", 1, true, true)) { return false; }
	}

	std::shared_ptr<Section> controlsSection(getSectionWithName(CONTROLS_SECTION_NAME));

	if(controlsSection == nullptr) {
		return false;
	}

	std::shared_ptr<Entry> showOpponentsWeaponEntry(getEntryWithName(SHOW_OPPONENTS_WEAPON_ENTRY_NAME));

	if(showOpponentsWeaponEntry != nullptr) {
		if(Utilities::areStringsEqual(showOpponentsWeaponEntry->getMultiStringValue(0), "W")) {
			if(!showOpponentsWeaponEntry->setMultiStringValue("", 0, true)) { return false; }
		}
		else if(Utilities::areStringsEqual(showOpponentsWeaponEntry->getMultiStringValue(1), "W")) {
			if(!showOpponentsWeaponEntry->setMultiStringValue("", 1, true)) { return false; }
		}
	}
	else {
		if(!keyDefinitionsSection->setEntryMultiStringValue(SHOW_OPPONENTS_WEAPON_ENTRY_NAME, "", 0, true, true)) { return false; }
	}

	std::shared_ptr<Entry> mapFollowModeEntry(getEntryWithName(MAP_FOLLOW_MODE_ENTRY_NAME));

	if(mapFollowModeEntry != nullptr) {
		if(Utilities::areStringsEqual(mapFollowModeEntry->getMultiStringValue(0), "F")) {
			if(!mapFollowModeEntry->setMultiStringValue("", 0, true)) { return false; }
		}
		else if(Utilities::areStringsEqual(mapFollowModeEntry->getMultiStringValue(1), "F")) {
			if(!mapFollowModeEntry->setMultiStringValue("", 1, true)) { return false; }
		}
	}
	else {
		if(!keyDefinitionsSection->setEntryMultiStringValue(MAP_FOLLOW_MODE_ENTRY_NAME, "", 0, true, true)) { return false; }
	}

	if(isAtomicEdition) {
		if(!controlsSection->setEntryIntegerValue(MOUSE_AIMING_FLIPPED_ENTRY_NAME, 1, true)) { return false; }
	}

	std::string firstMouseButtonEntryName(fmt::format("{}{}{}", MOUSE_PREFIX, BUTTON_SUFFIX, 1));

	if(isAtomicEdition) {
		if(!controlsSection->setEntryStringValue(firstMouseButtonEntryName, QUICK_KICK_ENTRY_NAME, true)) { return false; }
	}
	else {
		if(!controlsSection->setEntryStringValue(firstMouseButtonEntryName, JETPACK_ENTRY_NAME, true)) { return false; }
	}

	if(!controlsSection->setEntryStringValue(fmt::format("{}{}{}{}", MOUSE_PREFIX, BUTTON_SUFFIX, CLICKED_SUFFIX, 1), "", true)) { return false; }
	if(!controlsSection->setEntryStringValue(fmt::format("{}{}{}", MOUSE_PREFIX, BUTTON_SUFFIX, 2), OPEN_ENTRY_NAME, true)) { return false; }

	if(isAtomicEdition) {
		if(!controlsSection->setEntryIntegerValue(GAME_MOUSE_AIMING_ENTRY_NAME, 1, true)) { return false; }
		if(!controlsSection->setEntryIntegerValue(AIMING_FLAG_ENTRY_NAME, 1, true)) { return false; }
	}

	std::shared_ptr<Section> miscSection(getSectionWithName(MISC_SECTION_NAME));

	if(miscSection == nullptr) {
		miscSection = std::shared_ptr<Section>(new Section(MISC_SECTION_NAME));
		addSection(miscSection);
	}

	if(!isBeta) {
		if(!miscSection->setEntryIntegerValue(MISC_RUN_MODE_ENTRY_NAME, 1, true)) { return false; }
		if(!miscSection->setEntryIntegerValue(MISC_CROSSHAIRS_ENTRY_NAME, 1, true)) { return false; }
	}

	return true;
}
