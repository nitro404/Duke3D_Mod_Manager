#include "GameConfiguration.h"

#include "Game/GameVersion.h"

#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <sstream>

std::unique_ptr<GameConfiguration> GameConfiguration::generateDefaultGameConfiguration(const std::string & gameName) {
	bool isRegularVersion = Utilities::areStringsEqual(gameName, GameVersion::ORIGINAL_REGULAR_VERSION.getName());
	bool isAtomicEdition = Utilities::areStringsEqual(gameName, GameVersion::ORIGINAL_ATOMIC_EDITION.getName());

	std::unique_ptr<GameConfiguration> gameConfiguration(std::make_unique<GameConfiguration>());

	// create setup section
	std::shared_ptr<Section> setupSection(std::make_shared<Section>(SETUP_SECTION_NAME, "", "Setup File for Duke Nukem 3D"));
	gameConfiguration->addSection(setupSection);

	if(isRegularVersion) {
		setupSection->addStringEntry(SETUP_VERSION_ENTRY_NAME, REGULAR_VERSION_SETUP_VERSION);
	}
	else if(isAtomicEdition) {
		setupSection->addStringEntry(SETUP_VERSION_ENTRY_NAME, ATOMIC_EDITION_SETUP_VERSION);
	}

	// create screen setup section
	std::shared_ptr<Section> screenSetupSection(std::make_shared<Section>(SCREEN_SETUP_SECTION_NAME, " \n "));
	gameConfiguration->addSection(screenSetupSection);

	std::stringstream screenSetupSectionFollowingCommentsStream;
	screenSetupSectionFollowingCommentsStream << " \n";
	screenSetupSectionFollowingCommentsStream << " \n";
	screenSetupSectionFollowingCommentsStream << "ScreenMode\n";
	screenSetupSectionFollowingCommentsStream << " - Chained - 0\n";
	screenSetupSectionFollowingCommentsStream << " - Vesa 2.0 - 1\n";
	screenSetupSectionFollowingCommentsStream << " - Screen Buffered - 2\n";
	screenSetupSectionFollowingCommentsStream << " - Tseng optimized - 3\n";
	screenSetupSectionFollowingCommentsStream << " - Paradise optimized - 4\n";
	screenSetupSectionFollowingCommentsStream << " - S3 optimized - 5\n";
	screenSetupSectionFollowingCommentsStream << " - RedBlue Stereo - 7\n";
	screenSetupSectionFollowingCommentsStream << " - Crystal Eyes - 6\n";
	screenSetupSectionFollowingCommentsStream << " \n";
	screenSetupSectionFollowingCommentsStream << "ScreenWidth passed to engine\n";
	screenSetupSectionFollowingCommentsStream << " \n";
	screenSetupSectionFollowingCommentsStream << "ScreenHeight passed to engine\n";
	screenSetupSectionFollowingCommentsStream << " \n";
	screenSetupSectionFollowingCommentsStream << " ";
	screenSetupSection->setFollowingComments(screenSetupSectionFollowingCommentsStream.str());

	screenSetupSection->addIntegerEntry(SCREEN_MODE_ENTRY_NAME, 2);
	screenSetupSection->addIntegerEntry(SCREEN_WIDTH_ENTRY_NAME, 320);
	screenSetupSection->addIntegerEntry(SCREEN_HEIGHT_ENTRY_NAME, 200);

	// create sound setup section
	std::shared_ptr<Section> soundSetupSection(std::make_shared<Section>(SOUND_SETUP_SECTION_NAME, " \n ", " \n "));
	gameConfiguration->addSection(soundSetupSection);

	soundSetupSection->addIntegerEntry(FX_DEVICE_ENTRY_NAME, 13);
	soundSetupSection->addIntegerEntry(MUSIC_DEVICE_ENTRY_NAME, 13);
	soundSetupSection->addIntegerEntry(FX_VOLUME_ENTRY_NAME, 220);
	soundSetupSection->addIntegerEntry(MUSIC_VOLUME_ENTRY_NAME, 200);
	soundSetupSection->addIntegerEntry("NumVoices", 8);
	soundSetupSection->addIntegerEntry("NumChannels", 2);
	soundSetupSection->addIntegerEntry(NUM_BITS_ENTRY_NAME, 1);
	soundSetupSection->addIntegerEntry(MIX_RATE_ENTRY_NAME, 11000);
	soundSetupSection->addHexadecimalEntryUsingDecimal("MidiPort", 0x330);
	soundSetupSection->addHexadecimalEntryUsingDecimal("BlasterAddress", 0x220);
	soundSetupSection->addIntegerEntry("BlasterType", 6);
	soundSetupSection->addIntegerEntry("BlasterInterrupt", 7);
	soundSetupSection->addIntegerEntry("BlasterDma8", 1);
	soundSetupSection->addIntegerEntry("BlasterDma16", 5);
	soundSetupSection->addHexadecimalEntryUsingDecimal("BlasterEmu", 0x620);
	soundSetupSection->addIntegerEntry("ReverseStereo", 0);

	// create key definitions section
	std::shared_ptr<Section> keyDefinitionsSection(std::make_shared<Section>("KeyDefinitions", " \n ", " \n "));
	gameConfiguration->addSection(keyDefinitionsSection);

	keyDefinitionsSection->addMultiStringEntry("Move_Forward", "Up", "Kpad8");
	keyDefinitionsSection->addMultiStringEntry("Move_Backward", "Down", "Kpad2");
	keyDefinitionsSection->addMultiStringEntry("Turn_Left", "Left", "Kpad4");
	keyDefinitionsSection->addMultiStringEntry("Turn_Right", "Right", "KPad6");
	keyDefinitionsSection->addMultiStringEntry("Strafe", "LAlt", "RAlt");
	keyDefinitionsSection->addMultiStringEntry("Fire", "LCtrl", "RCtrl");
	keyDefinitionsSection->addMultiStringEntry("Open", "Space", "");
	keyDefinitionsSection->addMultiStringEntry("Run", "LShift", "RShift");
	keyDefinitionsSection->addMultiStringEntry("AutoRun", "CapLck", "");
	keyDefinitionsSection->addMultiStringEntry("Jump", "A", "/");
	keyDefinitionsSection->addMultiStringEntry("Crouch", "Z", "");
	keyDefinitionsSection->addMultiStringEntry("Look_Up", "PgUp", "Kpad9");
	keyDefinitionsSection->addMultiStringEntry("Look_Down", "PgDn", "Kpad3");
	keyDefinitionsSection->addMultiStringEntry("Look_Left", "Insert", "Kpad0");
	keyDefinitionsSection->addMultiStringEntry("Look_Right", "Delete", "Kpad.");
	keyDefinitionsSection->addMultiStringEntry("Strafe_Left", ",", "");
	keyDefinitionsSection->addMultiStringEntry("Strafe_Right", ".", "");
	keyDefinitionsSection->addMultiStringEntry("Aim_Up", "Home", "KPad7");
	keyDefinitionsSection->addMultiStringEntry("Aim_Down", "End", "Kpad1");

	for(size_t i = 1; i <= 10; i++) {
		keyDefinitionsSection->addMultiStringEntry(fmt::format("{}{}", WEAPON_KEY_DEFINITION_ENTRY_NAME_PREFIX, i), fmt::format("{}", i % 10) , "");
	}

	keyDefinitionsSection->addMultiStringEntry("Inventory", "Enter", "KpdEnt");
	keyDefinitionsSection->addMultiStringEntry("Inventory_Left", "[", "");
	keyDefinitionsSection->addMultiStringEntry("Inventory_Right", "]", "");
	keyDefinitionsSection->addMultiStringEntry("Holo_Duke", "H", "");
	keyDefinitionsSection->addMultiStringEntry("Jetpack", "J", "");
	keyDefinitionsSection->addMultiStringEntry("NightVision", "N", "");
	keyDefinitionsSection->addMultiStringEntry("MedKit", "M", "");
	keyDefinitionsSection->addMultiStringEntry("TurnAround", "BakSpc", "");
	keyDefinitionsSection->addMultiStringEntry("SendMessage", "T", "");
	keyDefinitionsSection->addMultiStringEntry("Map", "Tab", "");
	keyDefinitionsSection->addMultiStringEntry("Shrink_Screen", "-", "Kpad-");
	keyDefinitionsSection->addMultiStringEntry("Enlarge_Screen", "=", "Kpad+");
	keyDefinitionsSection->addMultiStringEntry("Center_View", "KPad5", "");
	keyDefinitionsSection->addMultiStringEntry("Holster_Weapon", "ScrLck", "");
	keyDefinitionsSection->addMultiStringEntry("Show_Opponents_Weapon", "W", "");
	keyDefinitionsSection->addMultiStringEntry("Map_Follow_Mode", "F", "");
	keyDefinitionsSection->addMultiStringEntry("See_Coop_View", "K", "");
	keyDefinitionsSection->addMultiStringEntry("Mouse_Aiming", "U", "");
	keyDefinitionsSection->addMultiStringEntry("Toggle_Crosshair", "I", "");
	keyDefinitionsSection->addMultiStringEntry("Steroids", "R", "");
	keyDefinitionsSection->addMultiStringEntry("Quick_Kick", "`", "");
	keyDefinitionsSection->addMultiStringEntry("Next_Weapon", "'", "");
	keyDefinitionsSection->addMultiStringEntry("Previous_Weapon", ";", "");

	// create controls section
	std::shared_ptr<Section> controlsSection(std::make_shared<Section>("Controls", " \n "));
	gameConfiguration->addSection(controlsSection);

	std::stringstream controlsSectionFollowingCommentsStream;
	controlsSectionFollowingCommentsStream << " \n";
	controlsSectionFollowingCommentsStream << " \n";
	controlsSectionFollowingCommentsStream << "Controls\n";
	controlsSectionFollowingCommentsStream << " \n";
	controlsSectionFollowingCommentsStream << "ControllerType\n";
	controlsSectionFollowingCommentsStream << " - Keyboard                  - 0\n";
	controlsSectionFollowingCommentsStream << " - Keyboard and Mouse        - 1\n";
	controlsSectionFollowingCommentsStream << " - Keyboard and Joystick     - 2\n";
	controlsSectionFollowingCommentsStream << " - Keyboard and Gamepad      - 4\n";
	controlsSectionFollowingCommentsStream << " - Keyboard and External     - 3\n";
	controlsSectionFollowingCommentsStream << " - Keyboard and FlightStick  - 5\n";
	controlsSectionFollowingCommentsStream << " - Keyboard and ThrustMaster - 6\n";
	controlsSectionFollowingCommentsStream << " \n";
	controlsSectionFollowingCommentsStream << " ";
	controlsSection->setFollowingComments(controlsSectionFollowingCommentsStream.str());

	controlsSection->addIntegerEntry("ControllerType", 1);
	controlsSection->addIntegerEntry("JoystickPort", 0);
	controlsSection->addIntegerEntry("MouseSensitivity", 32768);
	controlsSection->addStringEntry("ExternalFilename", "EXTERNAL.EXE");
	controlsSection->addIntegerEntry("EnableRudder", 0);
	controlsSection->addIntegerEntry("MouseAiming", 0);

	if(isAtomicEdition) {
		controlsSection->addIntegerEntry("MouseAimingFlipped", 0);
	}

	controlsSection->addStringEntry("MouseButton0", "Fire");
	controlsSection->addStringEntry("MouseButtonClicked0", "");
	controlsSection->addStringEntry("MouseButton1", "Strafe");
	controlsSection->addStringEntry("MouseButtonClicked1", "Open");
	controlsSection->addStringEntry("MouseButton2", "Move_Forward");
	controlsSection->addStringEntry("MouseButtonClicked2", "");
	controlsSection->addStringEntry("JoystickButton0", "Fire");
	controlsSection->addStringEntry("JoystickButtonClicked0", "");
	controlsSection->addStringEntry("JoystickButton1", "Strafe");
	controlsSection->addStringEntry("JoystickButtonClicked1", "Inventory");
	controlsSection->addStringEntry("JoystickButton2", "Run");
	controlsSection->addStringEntry("JoystickButtonClicked2", "Jump");
	controlsSection->addStringEntry("JoystickButton3", "Open");
	controlsSection->addStringEntry("JoystickButtonClicked3", "Crouch");
	controlsSection->addStringEntry("JoystickButton4", "Aim_Down");
	controlsSection->addStringEntry("JoystickButtonClicked4", "");
	controlsSection->addStringEntry("JoystickButton5", "Look_Right");
	controlsSection->addStringEntry("JoystickButtonClicked5", "");
	controlsSection->addStringEntry("JoystickButton6", "Aim_Up");
	controlsSection->addStringEntry("JoystickButtonClicked6", "");
	controlsSection->addStringEntry("JoystickButton7", "Look_Left");
	controlsSection->addStringEntry("JoystickButtonClicked7", "");
	controlsSection->addStringEntry("MouseAnalogAxes0", "analog_turning");
	controlsSection->addStringEntry("MouseDigitalAxes0_0", "");
	controlsSection->addStringEntry("MouseDigitalAxes0_1", "");
	controlsSection->addIntegerEntry("MouseAnalogScale0", 65536);
	controlsSection->addStringEntry("MouseAnalogAxes1", "analog_moving");
	controlsSection->addStringEntry("MouseDigitalAxes1_0", "");
	controlsSection->addStringEntry("MouseDigitalAxes1_1", "");
	controlsSection->addIntegerEntry("MouseAnalogScale1", 65536);
	controlsSection->addStringEntry("JoystickAnalogAxes0", "analog_turning");
	controlsSection->addStringEntry("JoystickDigitalAxes0_0", "");
	controlsSection->addStringEntry("JoystickDigitalAxes0_1", "");
	controlsSection->addIntegerEntry("JoystickAnalogScale0", 65536);
	controlsSection->addStringEntry("JoystickAnalogAxes1", "analog_moving");
	controlsSection->addStringEntry("JoystickDigitalAxes1_0", "");
	controlsSection->addStringEntry("JoystickDigitalAxes1_1", "");
	controlsSection->addIntegerEntry("JoystickAnalogScale1", 65536);
	controlsSection->addStringEntry("JoystickAnalogAxes2", "analog_strafing");
	controlsSection->addStringEntry("JoystickDigitalAxes2_0", "");
	controlsSection->addStringEntry("JoystickDigitalAxes2_1", "");
	controlsSection->addIntegerEntry("JoystickAnalogScale2", 65536);
	controlsSection->addStringEntry("JoystickAnalogAxes3", "");
	controlsSection->addStringEntry("JoystickDigitalAxes3_0", "Run");
	controlsSection->addStringEntry("JoystickDigitalAxes3_1", "");
	controlsSection->addIntegerEntry("JoystickAnalogScale3", 65536);
	controlsSection->addStringEntry("GamePadDigitalAxes0_0", "Turn_Left");
	controlsSection->addStringEntry("GamePadDigitalAxes0_1", "Turn_Right");
	controlsSection->addStringEntry("GamePadDigitalAxes1_0", "Move_Forward");
	controlsSection->addStringEntry("GamePadDigitalAxes1_1", "Move_Backward");

	// create communication setup section
	std::shared_ptr<Section> communicationSetupSection(std::make_shared<Section>("Comm Setup", " \n ", " \n "));
	gameConfiguration->addSection(communicationSetupSection);

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
	communicationSetupSection->addStringEntry("PlayerName", "DUKE");
	communicationSetupSection->addStringEntry("RTSName", "DUKE.RTS");

	if(isAtomicEdition) {
		communicationSetupSection->addStringEntry("RTSPath", ".\\");
		communicationSetupSection->addStringEntry("UserPath", ".\\");
	}

	communicationSetupSection->addStringEntry("PhoneNumber", "");
	communicationSetupSection->addIntegerEntry("ConnectType", 0);

	for(size_t i = 0; i < DEFAULT_COMBAT_MACROS.size(); i++) {
		communicationSetupSection->addStringEntry(fmt::format("{}{}", COMBAT_MACRO_ENTRY_NAME_PREFIX, i), DEFAULT_COMBAT_MACROS[i]);
	}

	size_t numberOfPhoneNumbers = isAtomicEdition ? 16 : 10;

	for(size_t i = 0; i < numberOfPhoneNumbers; i++) {
		communicationSetupSection->addStringEntry(fmt::format("{}{}", PHONE_NAME_ENTRY_NAME_PREFIX, i), "");
		communicationSetupSection->addStringEntry(fmt::format("{}{}", PHONE_NUMBER_ENTRY_NAME_PREFIX, i), "");

	}

	return gameConfiguration;
}

bool GameConfiguration::updateForDOSBox() {
	std::shared_ptr<Section> setupSection(getSectionWithName(SETUP_SECTION_NAME));

	if(setupSection == nullptr) {
		return false;
	}

	std::shared_ptr<Entry> setupVersionEntry(setupSection->getEntryWithName(SETUP_VERSION_ENTRY_NAME));

	if(setupVersionEntry == nullptr || !setupVersionEntry->isString()) {
		return false;
	}

	bool isRegularVersion = false;
	bool isAtomicEdition = false;

	if(Utilities::areStringsEqual(setupVersionEntry->getStringValue(), REGULAR_VERSION_SETUP_VERSION)) {
		isRegularVersion = true;
	}
	else if(Utilities::areStringsEqual(setupVersionEntry->getStringValue(), ATOMIC_EDITION_SETUP_VERSION)) {
		isAtomicEdition = true;
	}
	else {
		spdlog::warn("Unexpected setup version '{}', expected '{}' or '{}'. Assuming configuration file version is for Atomic Edition / Plutonium Pak.", setupVersionEntry->getStringValue(), REGULAR_VERSION_SETUP_VERSION, ATOMIC_EDITION_SETUP_VERSION);

		isAtomicEdition = true;
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
	if(!screenSetupSection->setEntryIntegerValue("Shadows", 1, true)) { return false; }

	if(isAtomicEdition) {
		if(!screenSetupSection->setEntryStringValue("Password", "", true)) { return false; }
	}
	else {
		if(!screenSetupSection->setEntryStringValue("Environment", "", true)) { return false; }
	}

	if(!screenSetupSection->setEntryIntegerValue("Detail", 1, true)) { return false; }
	if(!screenSetupSection->setEntryIntegerValue("Tilt", 1, true)) { return false; }
	if(!screenSetupSection->setEntryIntegerValue("Messages", 1, true)) { return false; }
	if(!screenSetupSection->setEntryIntegerValue("Out", 0, true)) { return false; }
	if(!screenSetupSection->setEntryIntegerValue("ScreenSize", 4, true)) { return false; }
	if(!screenSetupSection->setEntryIntegerValue("ScreenGamma", 0, true)) { return false; }

	if(!soundSetupSection->setEntryIntegerValue(FX_DEVICE_ENTRY_NAME, 0, true)) { return false; }
	if(!soundSetupSection->setEntryIntegerValue(MUSIC_DEVICE_ENTRY_NAME, 0, true)) { return false; }
	if(!soundSetupSection->setEntryIntegerValue(NUM_BITS_ENTRY_NAME, 16, true)) { return false; }
	if(!soundSetupSection->setEntryIntegerValue(MIX_RATE_ENTRY_NAME, isAtomicEdition ? 44000 : 22000, true)) { return false; }
	if(!soundSetupSection->setEntryIntegerValue("SoundToggle", 1, true)) { return false; }
	if(!soundSetupSection->setEntryIntegerValue("VoiceToggle", 1, true)) { return false; }
	if(!soundSetupSection->setEntryIntegerValue("AmbienceToggle", 1, true)) { return false; }
	if(!soundSetupSection->setEntryIntegerValue("MusicToggle", 1, true)) { return false; }

	return true;
}
