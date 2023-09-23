#include "GameConfiguration.h"

#include "Game/GameVersion.h"
#include "Game/File/Group/GRP/GroupGRP.h"

#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <sstream>

static const std::string SCREEN_BITS_PER_PIXEL_ENTRY_NAME("ScreenBPP");
static const std::string SCREEN_MAXIMUM_REFRESH_FREQUENCY_ENTRY_NAME("MaxRefreshFreq");
static const std::string SCREEN_OPENGL_TEXTURE_MODE_ENTRY_NAME("GLTextureMode");
static const std::string SCREEN_OPENGL_ANIMATION_SMOOTHING_ENTRY_NAME("GLAnimationSmoothing");
static const std::string SCREEN_OPENGL_USE_TEXTURE_COMPRESSION_ENTRY_NAME("GLUseTextureCompr");
static const std::string SCREEN_OPENGL_ANISOTROPY_ENTRY_NAME("GLAnisotropy");
static const std::string SCREEN_OPENGL_PRECACHE_ENTRY_NAME("GLUsePrecache");
static const std::string SCREEN_OPENGL_USE_COMPRESSED_TEXTURE_CACHE_ENTRY_NAME("GLUseCompressedTextureCache");
static const std::string SCREEN_OPENGL_USE_TEXTURE_CACHE_COMPRESSION_ENTRY_NAME("GLUseTextureCacheCompression");
static const std::string SCREEN_OPENGL_WIDE_SCREEN_ENTRY_NAME("GLWideScreen");
static const std::string SCREEN_OPENGL_FOV_SCREEN_ENTRY_NAME("GLFovscreen");
static const std::string SCREEN_OPENGL_VERTICAL_SYNC_ENTRY_NAME("GLVSync");
static const std::string SCREEN_USE_HIGH_TILE_ENTRY_NAME("UseHightile");
static const std::string SCREEN_EDUKE32_POLYMER_ENTRY_NAME("Polymer");
static const std::string SCREEN_AMBIENT_LIGHT_ENTRY_NAME("AmbientLight");
static const std::string SCREEN_USE_MODELS_ENTRY_NAME("UseModels");
static const std::string SCREEN_RESOLUTION_CONTROL_ENTRY_NAME("ResolutionControl");
static const std::string SCREEN_HUD_BACKGROUND_ENTRY_NAME("HUDBackground");
static const std::string SCREEN_FRAG_BAR_LAYOUT_ENTRY_NAME("FragBarLayout");
static const std::string SCREEN_STATUS_BAR_WIDTH_ENTRY_NAME("StatusBarWidth");
static const std::string SCREEN_STATUS_BAR_HEIGHT_ENTRY_NAME("StatusBarHeight");
static const std::string SCREEN_CROSSHAIR_WIDTH_ENTRY_NAME("CrosshairWidth");
static const std::string SCREEN_CROSSHAIR_HEIGHT_ENTRY_NAME("CrosshairHeight");
static const std::string SCREEN_FRAG_BAR_WIDTH_ENTRY_NAME("FragBarWidth");
static const std::string SCREEN_FRAG_BAR_HEIGHT_ENTRY_NAME("FragBarHeight");
static const std::string SCREEN_MESSAGES_WIDTH_ENTRY_NAME("MessagesWidth");
static const std::string SCREEN_MESSAGES_HEIGHT_ENTRY_NAME("MessagesHeight");
static const std::string SCREEN_MENU_WIDTH_ENTRY_NAME("MenuWidth");
static const std::string SCREEN_MENU_HEIGHT_ENTRY_NAME("MenuHeight");
static const std::string SCREEN_WIDESCREEN_ENTRY_NAME("Widescreen");
static const std::string SCREEN_WIDESCREEN_HUD_ENTRY_NAME("WidescreenHUD");
static const std::string SCREEN_COLOR_FIX_ENTRY_NAME("Colorfix");
static const std::string SOUND_RANDOM_MUSIC_ENTRY_NAME("RandomMusic");
static const std::string MISC_AUTO_AIM_ENTRY_NAME("AutoAim");
static const std::string MISC_SHOW_LEVEL_STATISTICS_ENTRY_NAME("ShowLevelStats");
static const std::string MISC_STATUS_BAR_SCALE_ENTRY_NAME("StatusBarScale");
static const std::string MISC_SHOW_OPPONENT_WEAPONS_ENTRY_NAME("ShowOpponentWeapons");
static const std::string MISC_USE_PRECACHE_ENTRY_NAME("UsePrecache");
static const std::string MISC_PREDICTION_DEBUG_ENTRY_NAME("PredictionDebug");
static const std::string MISC_WEAPON_ICONS_ENTRY_NAME("WeaponIcons");
static const std::string MISC_WEAPON_HIDE_ENTRY_NAME("WeaponHide");
static const std::string MISC_AUTO_SAVE_ENTRY_NAME("AutoSave");
static const std::string SETUP_EDUKE32_CONFIGURATION_VERSION("ConfigVersion");
static const std::string CONTROLS_USE_JOYSTICK_ENTRY_NAME("UseJoystick");
static const std::string CONTROLS_USE_MOUSE_ENTRY_NAME("UseMouse");
static const std::string CONTROLS_RUN_KEY_BEHAVIOUR_ENTRY_NAME("RunKeyBehaviour");
static const std::string CONTROLS_AUTO_AIM_ENTRY_NAME("AutoAim");
static const std::string CONTROLS_WEAPON_SWITCH_MODE_ENTRY_NAME("WeaponSwitchMode");
static const std::string CONTROLS_MOUSE_Y_LOCK_ENTRY_NAME("MouseYLock");
static const std::string CONTROLS_MOUSE_SCALE_X_ENTRY_NAME("MouseScaleX");
static const std::string CONTROLS_MOUSE_SCALE_Y_ENTRY_NAME("MouseScaleY");
static const std::string JFDUKE3D_SHOW_MENU_ENTRY_NAME("Show_Menu");
static const std::string SHOW_CONSOLE_ENTRY_NAME("Show_Console");
static const std::string PLAYER_COLOR_ENTRY_NAME("PlayerColor");
static const std::string KEY_DEFINITIONS_HIDE_WEAPON_ENTRY_NAME("Hide_Weapon");
static const std::string KEY_DEFINITIONS_AUTO_AIM_ENTRY_NAME("Auto_Aim");
static const std::string CONSOLE_ENTRY_NAME("Console");
static const std::string SHOW_FRAMES_PER_SECOND_ENTRY_NAME("ShowFPS");
static const std::string FULL_SCREEN_ENTRY_NAME("FullScreen");
static const std::string ALTERNATE_FULL_SCREEN_ENTRY_NAME("Fullscreen");
static const std::string EXTENDED_SCREEN_SIZE_ENTRY_NAME("ExtScreenSize");
static const std::string SHOW_CINEMATICS_ENTRY_NAME("ShowCinematics");
static const std::string OPPONENT_SOUND_TOGGLE_ENTRY_NAME("OpponentSoundToggle");
static const std::string RED_NUKEM_WINDOW_POSITIONING_ENTRY_NAME("WindowPositioning");
static const std::string RED_NUKEM_WINDOW_X_POSITION_ENTRY_NAME("WindowPosX");
static const std::string RED_NUKEM_WINDOW_Y_POSITION_ENTRY_NAME("WindowPosY");
static const std::string LOBBY_FILTER_SECTION_NAME("Lobby Filter");
static const std::string LOBBY_FILTER_GAME_MODE_ENTRY_NAME("GameMode");
static const std::string LOBBY_FILTER_SHOW_FULL_LOBBIES_ENTRY_NAME("ShowFullLobies");
static const std::string LOBBY_FILTER_MAPS_ENTRY_NAME("Maps");
static const std::string MULTIPLAYER_SECTION_NAME("Multiplayer");
static const std::string MULTIPLAYER_SHOW_INFO_ENTRY_NAME("ShowInfo");
static const std::string QUICK_LOAD_ENTRY_NAME("Quick_Load");
static const std::string QUICK_SAVE_ENTRY_NAME("Quick_Save");
static const std::string SAVE_MENU_ENTRY_NAME("Save_Menu");
static const std::string LOAD_MENU_ENTRY_NAME("Load_Menu");
static const std::string HELP_MENU_ENTRY_NAME("Help_Menu");
static const std::string SOUND_MENU_ENTRY_NAME("Sound_Menu");
static const std::string NEXT_TRACK_ENTRY_NAME("Next_Track");
static const std::string VIEW_MODE_ENTRY_NAME("View_Mode");
static const std::string VIDEO_MENU_ENTRY_NAME("Video_Menu");
static const std::string QUIT_GAME_ENTRY_NAME("Quit_Game");
static const std::string GAME_MENU_ENTRY_NAME("Game_Menu");

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
static const std::string ALTERNATE_DEADZONE_SUFFIX("Deadzone");
static const std::string SATURATE_SUFFIX("Saturate");
static const std::string INVERT_SUFFIX("Invert");
static const std::string HAT_SUFFIX("Hat");
static const std::string WEAPON_KEY_DEFINITION_ENTRY_NAME_PREFIX("Weapon_");
static const std::string COMBAT_MACRO_ENTRY_NAME_PREFIX("CommbatMacro#");
static const std::string PHONE_NAME_ENTRY_NAME_PREFIX("PhoneName#");
static const std::string PHONE_NUMBER_ENTRY_NAME_PREFIX("PhoneNumber#");
static const std::string WEAPON_CHOICE_PREFIX("WeaponChoice");
static const std::string RANCID_MEAT_SUFFIX("Rancid");
static const std::string IP_ADDRESS_ENTRY_NAME_PREFIX("IPAddress_");
static const std::string BOT_NAME_ENTRY_NAME_PREFIX("Botname_");
static const std::string BOT_NAME_NAME_PREFIX("Player_");

static constexpr uint16_t DEFAULT_JOYSTICK_ANALOG_DEADZONE = 1024;
static constexpr uint32_t DEFAULT_JOYSTICK_ANALOG_SATURATE = 31743;
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

static const std::vector<std::string> DEFAULT_RED_NUKEM_MOUSE_BUTTON_ACTIONS({
	GameConfiguration::FIRE_ENTRY_NAME,
	GameConfiguration::JETPACK_ENTRY_NAME,
	GameConfiguration::MEDKIT_ENTRY_NAME,
	"",
	GameConfiguration::PREVIOUS_WEAPON_ENTRY_NAME,
	GameConfiguration::NEXT_WEAPON_ENTRY_NAME
});

static const std::vector<std::string> DEFAULT_NETDUKE32_MOUSE_BUTTON_ACTIONS({
	GameConfiguration::FIRE_ENTRY_NAME,
	GameConfiguration::MEDKIT_ENTRY_NAME,
	GameConfiguration::JETPACK_ENTRY_NAME,
	"",
	GameConfiguration::PREVIOUS_WEAPON_ENTRY_NAME,
	GameConfiguration::NEXT_WEAPON_ENTRY_NAME
});

static const std::vector<std::string> DEFAULT_DUKE3DW_MOUSE_BUTTON_ACTIONS({
	GameConfiguration::FIRE_ENTRY_NAME,
	GameConfiguration::OPEN_ENTRY_NAME,
	GameConfiguration::MOVE_FORWARD_ENTRY_NAME,
	"",
	GameConfiguration::NEXT_WEAPON_ENTRY_NAME,
	GameConfiguration::PREVIOUS_WEAPON_ENTRY_NAME
});

static const std::vector<std::string> DEFAULT_DUKE3D_W32_MOUSE_BUTTON_ACTIONS({
	GameConfiguration::FIRE_ENTRY_NAME,
	GameConfiguration::OPEN_ENTRY_NAME,
	"",
	GameConfiguration::PREVIOUS_WEAPON_ENTRY_NAME,
	GameConfiguration::NEXT_WEAPON_ENTRY_NAME
});

static const std::vector<std::string> DEFAULT_RDUKE_MOUSE_BUTTON_ACTIONS({
	GameConfiguration::FIRE_ENTRY_NAME,
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

static const std::array<std::string, 3> NO_MOUSE_BUTTON_CLICKED_ACTIONS({
	"",
	"",
	""
});

static const std::array<std::string, 2> DEFAULT_MOUSE_ANALOG_AXES_ACTIONS({
	"analog_turning",
	"analog_moving"
});

static const std::array<std::string, 2> DEFAULT_NETDUKE32_MOUSE_ANALOG_AXES_ACTIONS({
	"analog_strafing",
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

static const std::vector<std::string> DEFAULT_JOYSTICK_ANALOG_AXES_ACTIONS({
	"analog_turning",
	"analog_moving",
	"analog_strafing",
	""
});

static const std::vector<std::string> DEFAULT_DUKE3D_W32_JOYSTICK_ANALOG_AXES_ACTIONS({
	"analog_turning",
	"analog_lookingupanddown",
	"analog_rolling",
	"analog_strafing",
	"analog_moving",
	"analog_rolling"
});

static const std::vector<std::pair<std::string, std::string>> DEFAULT_JOYSTICK_DIGITAL_AXES_ACTIONS({
	std::make_pair("",    ""),
	std::make_pair("",    ""),
	std::make_pair("",    ""),
	std::make_pair("Run", "")
});

static const std::vector<std::pair<std::string, std::string>> DEFAULT_DUKE3D_W32_JOYSTICK_DIGITAL_AXES_ACTIONS({
	std::make_pair("",    ""),
	std::make_pair("",    ""),
	std::make_pair("",    ""),
	std::make_pair("",    ""),
	std::make_pair("",    ""),
	std::make_pair("",    "")
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
	{ GameConfiguration::LOOK_RIGHT_ENTRY_NAME, "" }
});

static const std::vector<std::pair<std::string, std::string>> DEFAULT_DUKE3DW_JOYSTICK_BUTTON_ACTIONS({
	{ GameConfiguration::FIRE_ENTRY_NAME,       "" },
	{ GameConfiguration::STRAFE_ENTRY_NAME,     GameConfiguration::INVENTORY_ENTRY_NAME },
	{ GameConfiguration::RUN_ENTRY_NAME,        GameConfiguration::JUMP_ENTRY_NAME },
	{ GameConfiguration::OPEN_ENTRY_NAME,       GameConfiguration::CROUCH_ENTRY_NAME },
	{ "",                                       ""},
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
	{ GameConfiguration::AIM_DOWN_ENTRY_NAME,   "" },
	{ GameConfiguration::LOOK_RIGHT_ENTRY_NAME, "" },
	{ GameConfiguration::AIM_UP_ENTRY_NAME,     "" },
	{ GameConfiguration::LOOK_LEFT_ENTRY_NAME,  "" }
});

static const std::vector<std::pair<std::string, std::string>> DEFAULT_DUKE3D_W32_JOYSTICK_BUTTON_ACTIONS({
	{ GameConfiguration::FIRE_ENTRY_NAME,            "" },
	{ GameConfiguration::STRAFE_ENTRY_NAME,          "" },
	{ GameConfiguration::PREVIOUS_WEAPON_ENTRY_NAME, "" },
	{ GameConfiguration::NEXT_WEAPON_ENTRY_NAME,     "" },
	{ GameConfiguration::JUMP_ENTRY_NAME,            "" },
	{ GameConfiguration::OPEN_ENTRY_NAME,            "" },
	{ GameConfiguration::AIM_UP_ENTRY_NAME,          "" },
	{ GameConfiguration::LOOK_LEFT_ENTRY_NAME,       "" }
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

static const std::array<std::tuple<std::string, std::string, std::string>, 42> DEFAULT_KEY_DEFINITIONS({
	std::make_tuple(GameConfiguration::MOVE_FORWARD_ENTRY_NAME,          "Up",     "Kpad8"),
	std::make_tuple(GameConfiguration::MOVE_BACKWARD_ENTRY_NAME,         "Down",   "Kpad2"),
	std::make_tuple(GameConfiguration::TURN_LEFT_ENTRY_NAME,             "Left",   "Kpad4"),
	std::make_tuple(GameConfiguration::TURN_RIGHT_ENTRY_NAME,            "Right",  "KPad6"),
	std::make_tuple(GameConfiguration::STRAFE_ENTRY_NAME,                "LAlt",   "RAlt"),
	std::make_tuple(GameConfiguration::FIRE_ENTRY_NAME,                  "LCtrl",  "RCtrl"),
	std::make_tuple(GameConfiguration::OPEN_ENTRY_NAME,                  "Space",  ""),
	std::make_tuple(GameConfiguration::RUN_ENTRY_NAME,                   "LShift", "RShift"),
	std::make_tuple(GameConfiguration::AUTO_RUN_ENTRY_NAME,              "CapLck", ""),
	std::make_tuple(GameConfiguration::JUMP_ENTRY_NAME,                  "A",      "/"),
	std::make_tuple(GameConfiguration::CROUCH_ENTRY_NAME,                "Z",      ""),
	std::make_tuple(GameConfiguration::LOOK_UP_ENTRY_NAME,               "PgUp",   "Kpad9"),
	std::make_tuple(GameConfiguration::LOOK_DOWN_ENTRY_NAME,             "PgDn",   "Kpad3"),
	std::make_tuple(GameConfiguration::LOOK_LEFT_ENTRY_NAME,             "Insert", "Kpad0"),
	std::make_tuple(GameConfiguration::LOOK_RIGHT_ENTRY_NAME,            "Delete", "Kpad."),
	std::make_tuple(GameConfiguration::STRAFE_LEFT_ENTRY_NAME,           ",",      ""),
	std::make_tuple(GameConfiguration::STRAFE_RIGHT_ENTRY_NAME,          ".",      ""),
	std::make_tuple(GameConfiguration::AIM_UP_ENTRY_NAME,                "Home",   "KPad7"),
	std::make_tuple(GameConfiguration::AIM_DOWN_ENTRY_NAME,              "End",    "Kpad1"),
	std::make_tuple(GameConfiguration::INVENTORY_ENTRY_NAME,             "Enter",  "KpdEnt"),
	std::make_tuple(GameConfiguration::INVENTORY_LEFT_ENTRY_NAME,        "[",      ""),
	std::make_tuple(GameConfiguration::INVENTORY_RIGHT_ENTRY_NAME,       "]",      ""),
	std::make_tuple(GameConfiguration::HOLO_DUKE_ENTRY_NAME,             "H",      ""),
	std::make_tuple(GameConfiguration::JETPACK_ENTRY_NAME,               "J",      ""),
	std::make_tuple(GameConfiguration::NIGHT_VISION_ENTRY_NAME,          "N",      ""),
	std::make_tuple(GameConfiguration::MEDKIT_ENTRY_NAME,                "M",      ""),
	std::make_tuple(GameConfiguration::TURN_AROUND_ENTRY_NAME,           "BakSpc", ""),
	std::make_tuple(GameConfiguration::SEND_MESSAGE_ENTRY_NAME,          "T",      ""),
	std::make_tuple(GameConfiguration::MAP_ENTRY_NAME,                   "Tab",    ""),
	std::make_tuple(GameConfiguration::SHRINK_SCREEN_ENTRY_NAME,         "-",      "Kpad-"),
	std::make_tuple(GameConfiguration::ENLARGE_SCREEN_ENTRY_NAME,        "=",      "Kpad+"),
	std::make_tuple(GameConfiguration::CENTER_VIEW_ENTRY_NAME,           "KPad5",  ""),
	std::make_tuple(GameConfiguration::HOLSTER_WEAPON_ENTRY_NAME,        "ScrLck", ""),
	std::make_tuple(GameConfiguration::SHOW_OPPONENTS_WEAPON_ENTRY_NAME, "W",      ""),
	std::make_tuple(GameConfiguration::MAP_FOLLOW_MODE_ENTRY_NAME,       "F",      ""),
	std::make_tuple(GameConfiguration::SEE_COOP_VIEW_ENTRY_NAME,         "K",      ""),
	std::make_tuple(GameConfiguration::MOUSE_AIMING_ENTRY_NAME,          "U",      ""),
	std::make_tuple(GameConfiguration::TOGGLE_CROSSHAIR_ENTRY_NAME,      "I",      ""),
	std::make_tuple(GameConfiguration::STEROIDS_ENTRY_NAME,              "R",      ""),
	std::make_tuple(GameConfiguration::QUICK_KICK_ENTRY_NAME,            "`",      ""),
	std::make_tuple(GameConfiguration::NEXT_WEAPON_ENTRY_NAME,           "'",      ""),
	std::make_tuple(GameConfiguration::PREVIOUS_WEAPON_ENTRY_NAME,       ";",      "")
});

static const std::array<std::tuple<std::string, std::string, std::string>, 42> DEFAULT_PKDUKE3D_KEY_DEFINITIONS({
	std::make_tuple(GameConfiguration::MOVE_FORWARD_ENTRY_NAME,          "W",        "Up"),
	std::make_tuple(GameConfiguration::MOVE_BACKWARD_ENTRY_NAME,         "S",        "Down"),
	std::make_tuple(GameConfiguration::TURN_LEFT_ENTRY_NAME,             "None",     "Left"),
	std::make_tuple(GameConfiguration::TURN_RIGHT_ENTRY_NAME,            "None",     "Right"),
	std::make_tuple(GameConfiguration::STRAFE_ENTRY_NAME,                "LAlt",     "RAlt"),
	std::make_tuple(GameConfiguration::FIRE_ENTRY_NAME,                  "Mouse 1",  "None"),
	std::make_tuple(GameConfiguration::OPEN_ENTRY_NAME,                  "E",        "Mouse 3"),
	std::make_tuple(GameConfiguration::RUN_ENTRY_NAME,                   "LShift",   "RShift"),
	std::make_tuple(GameConfiguration::AUTO_RUN_ENTRY_NAME,              "CapsLock", "None"),
	std::make_tuple(GameConfiguration::JUMP_ENTRY_NAME,                  "Space",    "None"),
	std::make_tuple(GameConfiguration::CROUCH_ENTRY_NAME,                "LCtrl",    "None"),
	std::make_tuple(GameConfiguration::LOOK_UP_ENTRY_NAME,               "None",     "None"),
	std::make_tuple(GameConfiguration::LOOK_DOWN_ENTRY_NAME,             "None",     "None"),
	std::make_tuple(GameConfiguration::LOOK_LEFT_ENTRY_NAME,             "None",     "None"),
	std::make_tuple(GameConfiguration::LOOK_RIGHT_ENTRY_NAME,            "None",     "None"),
	std::make_tuple(GameConfiguration::STRAFE_LEFT_ENTRY_NAME,           "A",        "None"),
	std::make_tuple(GameConfiguration::STRAFE_RIGHT_ENTRY_NAME,          "D",        "None"),
	std::make_tuple(GameConfiguration::AIM_UP_ENTRY_NAME,                "Home",     "KPad 7"),
	std::make_tuple(GameConfiguration::AIM_DOWN_ENTRY_NAME,              "End",      "KPad 1"),
	std::make_tuple(GameConfiguration::INVENTORY_ENTRY_NAME,             "Enter",    "KP Enter"),
	std::make_tuple(GameConfiguration::INVENTORY_LEFT_ENTRY_NAME,        "[",        "None"),
	std::make_tuple(GameConfiguration::INVENTORY_RIGHT_ENTRY_NAME,       "]",        "None"),
	std::make_tuple(GameConfiguration::HOLO_DUKE_ENTRY_NAME,             "H",        "None"),
	std::make_tuple(GameConfiguration::JETPACK_ENTRY_NAME,               "J",        "None"),
	std::make_tuple(GameConfiguration::NIGHT_VISION_ENTRY_NAME,          "N",        "None"),
	std::make_tuple(GameConfiguration::MEDKIT_ENTRY_NAME,                "M",        "None"),
	std::make_tuple(GameConfiguration::TURN_AROUND_ENTRY_NAME,           "None",     "None"),
	std::make_tuple(GameConfiguration::SEND_MESSAGE_ENTRY_NAME,          "T",        "None"),
	std::make_tuple(GameConfiguration::MAP_ENTRY_NAME,                   "Tab",      "None"),
	std::make_tuple(GameConfiguration::SHRINK_SCREEN_ENTRY_NAME,         "-",        "None"),
	std::make_tuple(GameConfiguration::ENLARGE_SCREEN_ENTRY_NAME,        "=",        "None"),
	std::make_tuple(GameConfiguration::CENTER_VIEW_ENTRY_NAME,           "KPad 5",   "None"),
	std::make_tuple(GameConfiguration::HOLSTER_WEAPON_ENTRY_NAME,        "None",     "None"),
	std::make_tuple(GameConfiguration::SHOW_OPPONENTS_WEAPON_ENTRY_NAME, "None",     "None"),
	std::make_tuple(GameConfiguration::MAP_FOLLOW_MODE_ENTRY_NAME,       "F",        "None"),
	std::make_tuple(GameConfiguration::SEE_COOP_VIEW_ENTRY_NAME,         "None",     "None"),
	std::make_tuple(GameConfiguration::MOUSE_AIMING_ENTRY_NAME,          "U",        "None"),
	std::make_tuple(GameConfiguration::TOGGLE_CROSSHAIR_ENTRY_NAME,      "None",     "None"),
	std::make_tuple(GameConfiguration::STEROIDS_ENTRY_NAME,              "R",        "None"),
	std::make_tuple(GameConfiguration::QUICK_KICK_ENTRY_NAME,            ",",        "`"),
	std::make_tuple(GameConfiguration::NEXT_WEAPON_ENTRY_NAME,           "'",        "Wheel Up"),
	std::make_tuple(GameConfiguration::PREVIOUS_WEAPON_ENTRY_NAME,       ";",        "Wheel Dn")
});

static const std::array<std::tuple<std::string, std::string, std::string>, 42> DEFAULT_DUKE3DW_KEY_DEFINITIONS({
	std::make_tuple(GameConfiguration::MOVE_FORWARD_ENTRY_NAME,          "Up",     "W"),
	std::make_tuple(GameConfiguration::MOVE_BACKWARD_ENTRY_NAME,         "Down",   "S"),
	std::make_tuple(GameConfiguration::TURN_LEFT_ENTRY_NAME,             "Left",   ""),
	std::make_tuple(GameConfiguration::TURN_RIGHT_ENTRY_NAME,            "Right",  ""),
	std::make_tuple(GameConfiguration::STRAFE_ENTRY_NAME,                "",       "RAlt"),
	std::make_tuple(GameConfiguration::FIRE_ENTRY_NAME,                  "LCtrl",  "RCtrl"),
	std::make_tuple(GameConfiguration::OPEN_ENTRY_NAME,                  "E",      "Kpad0"),
	std::make_tuple(GameConfiguration::RUN_ENTRY_NAME,                   "LShift", "RShift"),
	std::make_tuple(GameConfiguration::AUTO_RUN_ENTRY_NAME,              "CapLck", ""),
	std::make_tuple(GameConfiguration::JUMP_ENTRY_NAME,                  "Space",  "LAlt"),
	std::make_tuple(GameConfiguration::CROUCH_ENTRY_NAME,                "C",      "Z"),
	std::make_tuple(GameConfiguration::LOOK_UP_ENTRY_NAME,               "PgUp",   "Kpad9"),
	std::make_tuple(GameConfiguration::LOOK_DOWN_ENTRY_NAME,             "PgDn",   "Kpad3"),
	std::make_tuple(GameConfiguration::LOOK_LEFT_ENTRY_NAME,             "Insert", ""),
	std::make_tuple(GameConfiguration::LOOK_RIGHT_ENTRY_NAME,            "Delete", "Kpad."),
	std::make_tuple(GameConfiguration::STRAFE_LEFT_ENTRY_NAME,           "A",      ""),
	std::make_tuple(GameConfiguration::STRAFE_RIGHT_ENTRY_NAME,          "D",      ""),
	std::make_tuple(GameConfiguration::AIM_UP_ENTRY_NAME,                "Home",   "KPad7"),
	std::make_tuple(GameConfiguration::AIM_DOWN_ENTRY_NAME,              "End",    "Kpad1"),
	std::make_tuple(GameConfiguration::INVENTORY_ENTRY_NAME,             "Enter",  "KpdEnt"),
	std::make_tuple(GameConfiguration::INVENTORY_LEFT_ENTRY_NAME,        "[",      ""),
	std::make_tuple(GameConfiguration::INVENTORY_RIGHT_ENTRY_NAME,       "]",      ""),
	std::make_tuple(GameConfiguration::HOLO_DUKE_ENTRY_NAME,             "H",      ""),
	std::make_tuple(GameConfiguration::JETPACK_ENTRY_NAME,               "J",      ""),
	std::make_tuple(GameConfiguration::NIGHT_VISION_ENTRY_NAME,          "N",      ""),
	std::make_tuple(GameConfiguration::MEDKIT_ENTRY_NAME,                "M",      ""),
	std::make_tuple(GameConfiguration::TURN_AROUND_ENTRY_NAME,           "BakSpc", ""),
	std::make_tuple(GameConfiguration::SEND_MESSAGE_ENTRY_NAME,          "T",      ""),
	std::make_tuple(GameConfiguration::MAP_ENTRY_NAME,                   "Tab",    ""),
	std::make_tuple(GameConfiguration::SHRINK_SCREEN_ENTRY_NAME,         "-",      "Kpad-"),
	std::make_tuple(GameConfiguration::ENLARGE_SCREEN_ENTRY_NAME,        "=",      "Kpad+"),
	std::make_tuple(GameConfiguration::CENTER_VIEW_ENTRY_NAME,           "KPad5",  ""),
	std::make_tuple(GameConfiguration::HOLSTER_WEAPON_ENTRY_NAME,        "ScrLck", ""),
	std::make_tuple(GameConfiguration::SHOW_OPPONENTS_WEAPON_ENTRY_NAME, "Y",      ""),
	std::make_tuple(GameConfiguration::MAP_FOLLOW_MODE_ENTRY_NAME,       "F",      ""),
	std::make_tuple(GameConfiguration::SEE_COOP_VIEW_ENTRY_NAME,         "K",      ""),
	std::make_tuple(GameConfiguration::MOUSE_AIMING_ENTRY_NAME,          "U",      ""),
	std::make_tuple(GameConfiguration::TOGGLE_CROSSHAIR_ENTRY_NAME,      "I",      ""),
	std::make_tuple(GameConfiguration::STEROIDS_ENTRY_NAME,              "R",      ""),
	std::make_tuple(GameConfiguration::QUICK_KICK_ENTRY_NAME,            "`",      ""),
	std::make_tuple(GameConfiguration::NEXT_WEAPON_ENTRY_NAME,           "'",      ""),
	std::make_tuple(GameConfiguration::PREVIOUS_WEAPON_ENTRY_NAME,       ";",      "")
});

static const std::array<std::tuple<std::string, std::string, std::string>, 42> DEFAULT_DUKE3D_W32_KEY_DEFINITIONS({
	std::make_tuple(GameConfiguration::MOVE_FORWARD_ENTRY_NAME,          "W",      "Kpad8"),
	std::make_tuple(GameConfiguration::MOVE_BACKWARD_ENTRY_NAME,         "S",      "Kpad2"),
	std::make_tuple(GameConfiguration::TURN_LEFT_ENTRY_NAME,             "Left",   "Kpad4"),
	std::make_tuple(GameConfiguration::TURN_RIGHT_ENTRY_NAME,            "Right",  "KPad6"),
	std::make_tuple(GameConfiguration::STRAFE_ENTRY_NAME,                "LAlt",   "RAlt"),
	std::make_tuple(GameConfiguration::FIRE_ENTRY_NAME,                  "LCtrl",  "RCtrl"),
	std::make_tuple(GameConfiguration::OPEN_ENTRY_NAME,                  "F",      ""),
	std::make_tuple(GameConfiguration::RUN_ENTRY_NAME,                   "LShift", "RShift"),
	std::make_tuple(GameConfiguration::AUTO_RUN_ENTRY_NAME,              "CapLck", "L"),
	std::make_tuple(GameConfiguration::JUMP_ENTRY_NAME,                  "Space",  "/"),
	std::make_tuple(GameConfiguration::CROUCH_ENTRY_NAME,                "Z",      ""),
	std::make_tuple(GameConfiguration::LOOK_UP_ENTRY_NAME,               "PgUp",   "Kpad9"),
	std::make_tuple(GameConfiguration::LOOK_DOWN_ENTRY_NAME,             "PgDn",   "Kpad3"),
	std::make_tuple(GameConfiguration::LOOK_LEFT_ENTRY_NAME,             "Insert", "Kpad0"),
	std::make_tuple(GameConfiguration::LOOK_RIGHT_ENTRY_NAME,            "Delete", "Kpad."),
	std::make_tuple(GameConfiguration::STRAFE_LEFT_ENTRY_NAME,           "A",      ""),
	std::make_tuple(GameConfiguration::STRAFE_RIGHT_ENTRY_NAME,          "D",      ""),
	std::make_tuple(GameConfiguration::AIM_UP_ENTRY_NAME,                "Home",   "KPad7"),
	std::make_tuple(GameConfiguration::AIM_DOWN_ENTRY_NAME,              "End",    "Kpad1"),
	std::make_tuple(GameConfiguration::INVENTORY_ENTRY_NAME,             "Enter",  "KpdEnt"),
	std::make_tuple(GameConfiguration::INVENTORY_LEFT_ENTRY_NAME,        "[",      ""),
	std::make_tuple(GameConfiguration::INVENTORY_RIGHT_ENTRY_NAME,       "]",      ""),
	std::make_tuple(GameConfiguration::HOLO_DUKE_ENTRY_NAME,             "H",      ""),
	std::make_tuple(GameConfiguration::JETPACK_ENTRY_NAME,               "J",      ""),
	std::make_tuple(GameConfiguration::NIGHT_VISION_ENTRY_NAME,          "N",      ""),
	std::make_tuple(GameConfiguration::MEDKIT_ENTRY_NAME,                "M",      ""),
	std::make_tuple(GameConfiguration::TURN_AROUND_ENTRY_NAME,           "BakSpc", ""),
	std::make_tuple(GameConfiguration::SEND_MESSAGE_ENTRY_NAME,          "T",      ""),
	std::make_tuple(GameConfiguration::MAP_ENTRY_NAME,                   "Tab",    ""),
	std::make_tuple(GameConfiguration::SHRINK_SCREEN_ENTRY_NAME,         "-",      "Kpad-"),
	std::make_tuple(GameConfiguration::ENLARGE_SCREEN_ENTRY_NAME,        "=",      "Kpad+"),
	std::make_tuple(GameConfiguration::CENTER_VIEW_ENTRY_NAME,           "KPad5",  ""),
	std::make_tuple(GameConfiguration::HOLSTER_WEAPON_ENTRY_NAME,        "ScrLck", ""),
	std::make_tuple(GameConfiguration::SHOW_OPPONENTS_WEAPON_ENTRY_NAME, "P",      ""),
	std::make_tuple(GameConfiguration::MAP_FOLLOW_MODE_ENTRY_NAME,       "",       ""),
	std::make_tuple(GameConfiguration::SEE_COOP_VIEW_ENTRY_NAME,         "K",      ""),
	std::make_tuple(GameConfiguration::MOUSE_AIMING_ENTRY_NAME,          "U",      ""),
	std::make_tuple(GameConfiguration::TOGGLE_CROSSHAIR_ENTRY_NAME,      "I",      ""),
	std::make_tuple(GameConfiguration::STEROIDS_ENTRY_NAME,              "R",      ""),
	std::make_tuple(GameConfiguration::QUICK_KICK_ENTRY_NAME,            "Q",      ""),
	std::make_tuple(GameConfiguration::NEXT_WEAPON_ENTRY_NAME,           "'",      ""),
	std::make_tuple(GameConfiguration::PREVIOUS_WEAPON_ENTRY_NAME,       ";",      "")
});

static const std::array<std::tuple<std::string, std::string, std::string>, 42> DEFAULT_RDUKE_KEY_DEFINITIONS({
	std::make_tuple(GameConfiguration::MOVE_FORWARD_ENTRY_NAME,          "W",      "Kpad8"),
	std::make_tuple(GameConfiguration::MOVE_BACKWARD_ENTRY_NAME,         "S",      "Kpad2"),
	std::make_tuple(GameConfiguration::TURN_LEFT_ENTRY_NAME,             "Left",   "Kpad4"),
	std::make_tuple(GameConfiguration::TURN_RIGHT_ENTRY_NAME,            "Right",  "KPad6"),
	std::make_tuple(GameConfiguration::STRAFE_ENTRY_NAME,                "LAlt",   "RAlt"),
	std::make_tuple(GameConfiguration::FIRE_ENTRY_NAME,                  "RCtrl",  ""),
	std::make_tuple(GameConfiguration::OPEN_ENTRY_NAME,                  "E",      ""),
	std::make_tuple(GameConfiguration::RUN_ENTRY_NAME,                   "LShift", "RShift"),
	std::make_tuple(GameConfiguration::AUTO_RUN_ENTRY_NAME,              "CapLck", ""),
	std::make_tuple(GameConfiguration::JUMP_ENTRY_NAME,                  "Space",  "/"),
	std::make_tuple(GameConfiguration::CROUCH_ENTRY_NAME,                "LCtrl",  ""),
	std::make_tuple(GameConfiguration::LOOK_UP_ENTRY_NAME,               "PgUp",   "Kpad9"),
	std::make_tuple(GameConfiguration::LOOK_DOWN_ENTRY_NAME,             "PgDn",   "Kpad3"),
	std::make_tuple(GameConfiguration::LOOK_LEFT_ENTRY_NAME,             "Insert", "Kpad0"),
	std::make_tuple(GameConfiguration::LOOK_RIGHT_ENTRY_NAME,            "Delete", "Kpad."),
	std::make_tuple(GameConfiguration::STRAFE_LEFT_ENTRY_NAME,           "A",      ""),
	std::make_tuple(GameConfiguration::STRAFE_RIGHT_ENTRY_NAME,          "D",      ""),
	std::make_tuple(GameConfiguration::AIM_UP_ENTRY_NAME,                "Home",   "KPad7"),
	std::make_tuple(GameConfiguration::AIM_DOWN_ENTRY_NAME,              "End",    "Kpad1"),
	std::make_tuple(GameConfiguration::INVENTORY_ENTRY_NAME,             "Enter",  "KpdEnt"),
	std::make_tuple(GameConfiguration::INVENTORY_LEFT_ENTRY_NAME,        "[",      ""),
	std::make_tuple(GameConfiguration::INVENTORY_RIGHT_ENTRY_NAME,       "]",      ""),
	std::make_tuple(GameConfiguration::HOLO_DUKE_ENTRY_NAME,             "H",      ""),
	std::make_tuple(GameConfiguration::JETPACK_ENTRY_NAME,               "J",      ""),
	std::make_tuple(GameConfiguration::NIGHT_VISION_ENTRY_NAME,          "N",      ""),
	std::make_tuple(GameConfiguration::MEDKIT_ENTRY_NAME,                "M",      ""),
	std::make_tuple(GameConfiguration::TURN_AROUND_ENTRY_NAME,           "BakSpc", ""),
	std::make_tuple(GameConfiguration::SEND_MESSAGE_ENTRY_NAME,          "T",      ""),
	std::make_tuple(GameConfiguration::MAP_ENTRY_NAME,                   "Tab",    ""),
	std::make_tuple(GameConfiguration::SHRINK_SCREEN_ENTRY_NAME,         "-",      "Kpad-"),
	std::make_tuple(GameConfiguration::ENLARGE_SCREEN_ENTRY_NAME,        "=",      "Kpad+"),
	std::make_tuple(GameConfiguration::CENTER_VIEW_ENTRY_NAME,           "KPad5",  ""),
	std::make_tuple(GameConfiguration::HOLSTER_WEAPON_ENTRY_NAME,        "ScrLck", ""),
	std::make_tuple(GameConfiguration::SHOW_OPPONENTS_WEAPON_ENTRY_NAME, "Y",      ""),
	std::make_tuple(GameConfiguration::MAP_FOLLOW_MODE_ENTRY_NAME,       "F",      ""),
	std::make_tuple(GameConfiguration::SEE_COOP_VIEW_ENTRY_NAME,         "K",      ""),
	std::make_tuple(GameConfiguration::MOUSE_AIMING_ENTRY_NAME,          "U",      ""),
	std::make_tuple(GameConfiguration::TOGGLE_CROSSHAIR_ENTRY_NAME,      "I",      ""),
	std::make_tuple(GameConfiguration::STEROIDS_ENTRY_NAME,              "R",      ""),
	std::make_tuple(GameConfiguration::QUICK_KICK_ENTRY_NAME,            "Q",      ""),
	std::make_tuple(GameConfiguration::NEXT_WEAPON_ENTRY_NAME,           "'",      ""),
	std::make_tuple(GameConfiguration::PREVIOUS_WEAPON_ENTRY_NAME,       ";",      "")
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

std::string GameConfiguration::getGameVersionIDFromType(GameVersionType gameVersionType) {
	switch(gameVersionType) {
		case GameVersionType::Beta:
			return GameVersion::ORIGINAL_BETA_VERSION.getID();

		case GameVersionType::RegularVersion:
			return GameVersion::ORIGINAL_REGULAR_VERSION.getID();

		case GameVersionType::AtomicEdition:
			return GameVersion::ORIGINAL_ATOMIC_EDITION.getID();

		case GameVersionType::JFDuke3D:
			return GameVersion::JFDUKE3D.getID();

		case GameVersionType::eDuke32:
			return GameVersion::EDUKE32.getID();

		case GameVersionType::NetDuke32:
			return GameVersion::NETDUKE32.getID();

		case GameVersionType::RedNukem:
			return GameVersion::RED_NUKEM.getID();

		case GameVersionType::BelgianChocolate:
			return GameVersion::BELGIAN_CHOCOLATE_DUKE3D.getID();

		case GameVersionType::Duke3dw:
			return GameVersion::DUKE3DW.getID();

		case GameVersionType::pkDuke3D:
			return GameVersion::PKDUKE3D.getID();

		case GameVersionType::xDuke:
			return GameVersion::XDUKE.getID();

		case GameVersionType::rDuke:
			return GameVersion::RDUKE.getID();

		case GameVersionType::Duke3d_w32:
			return GameVersion::DUKE3D_W32.getID();
	}

	return {};
}

std::optional<GameConfiguration::GameVersionType> GameConfiguration::getGameVersionTypeFromID(std::string_view gameVersionID) {
	if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_BETA_VERSION.getID())) {
		return GameVersionType::Beta;
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_REGULAR_VERSION.getID())) {
		return GameVersionType::RegularVersion;
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_ATOMIC_EDITION.getID()) ||
			Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::ORIGINAL_PLUTONIUM_PAK.getID())) {
		return GameVersionType::AtomicEdition;
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::JFDUKE3D.getID())) {
		return GameVersionType::JFDuke3D;
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::EDUKE32.getID())) {
		return GameVersionType::eDuke32;
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::NETDUKE32.getID())) {
		return GameVersionType::NetDuke32;
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::RED_NUKEM.getID())) {
		return GameVersionType::RedNukem;
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::BELGIAN_CHOCOLATE_DUKE3D.getID())) {
		return GameVersionType::BelgianChocolate;
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::DUKE3DW.getID())) {
		return GameVersionType::Duke3dw;
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::PKDUKE3D.getID())) {
		return GameVersionType::pkDuke3D;
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::XDUKE.getID())) {
		return GameVersionType::xDuke;
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::RDUKE.getID())) {
		return GameVersionType::rDuke;
	}
	else if(Utilities::areStringsEqualIgnoreCase(gameVersionID, GameVersion::DUKE3D_W32.getID())) {
		return GameVersionType::Duke3d_w32;
	}

	return {};
}

std::optional<GameConfiguration::SectionFeatures> GameConfiguration::getSectionFeaturesForGameVersion(std::string_view gameVersionID) {
	std::optional<GameVersionType> optionalGameVersionType(getGameVersionTypeFromID(gameVersionID));

	if(!optionalGameVersionType.has_value()) {
		return {};
	}

	return getSectionFeaturesForGameVersion(optionalGameVersionType.value());
}

GameConfiguration::SectionFeatures GameConfiguration::getSectionFeaturesForGameVersion(GameVersionType gameVersionType) {
	SectionFeatures sectionFeatures;

	switch(gameVersionType) {
		case GameVersionType::Beta: {
			sectionFeatures.misc &= ~(SectionFeatures::Misc::DoesSupportRunModeToggle |
									  SectionFeatures::Misc::DoesSupportCrosshairToggle);

			sectionFeatures.controls &= ~(SectionFeatures::Controls::HasMouseAimFlippedEntry |
										  SectionFeatures::Controls::DoesSupportQuickKick);

			sectionFeatures.screen |= SectionFeatures::Screen::HasLastInputOutputSlotEntry;
			break;
		}

		case GameVersionType::RegularVersion: {
			sectionFeatures.controls &= ~(SectionFeatures::Controls::HasMouseAimFlippedEntry |
										  SectionFeatures::Controls::DoesSupportQuickKick);
			break;
		}

		case GameVersionType::AtomicEdition: {
			break;
		}

		case GameVersionType::JFDuke3D: {
			sectionFeatures.general &= ~SectionFeatures::General::HasComments;

			sectionFeatures.setup |= SectionFeatures::Setup::HasForceSetupEntry;

			sectionFeatures.misc |= SectionFeatures::Misc::Populated |
									SectionFeatures::Misc::HasExtendedHeadsUpDisplayEntries |
									SectionFeatures::Misc::HasUsePrecacheEntry |
									SectionFeatures::Misc::HasWeaponChoicePreferences;

			sectionFeatures.controls |= SectionFeatures::Controls::HasAutoAimEntry |
										SectionFeatures::Controls::HasWeaponSwitchModeEntry |
										SectionFeatures::Controls::HasRunKeyBehaviourEntry |
										SectionFeatures::Controls::HasAimingFlagEntry |
										SectionFeatures::Controls::HasJoystickAnalogDeadZoneEntries |
										SectionFeatures::Controls::HasJoystickAnalogSaturateEntries |
										SectionFeatures::Controls::HasUseMouseAndJoystickEntries |
										SectionFeatures::Controls::HasUseMouseAndJoystickEntries;

			sectionFeatures.controls &= ~(SectionFeatures::Controls::HasExternalFileNameEntry |
										  SectionFeatures::Controls::HasRudderEntry |
										  SectionFeatures::Controls::HasGamepadControls);

			sectionFeatures.sound |= SectionFeatures::Sound::HasDefaultEntries;

			sectionFeatures.sound &= ~SectionFeatures::Sound::HasLegacyEntries;

			sectionFeatures.screen |= SectionFeatures::Screen::HasAdvancedGraphicsEntries |
									  SectionFeatures::Screen::HasDefaultEntries |
									  SectionFeatures::Screen::HasOpenGLTextureModeEntry |
									  SectionFeatures::Screen::HasOpenGLUseTextureCompressionEntry |
									  SectionFeatures::Screen::HasOpenGLAnisotropyEntry |
									  SectionFeatures::Screen::HasOpenGLUseCompressedTextureCacheEntry;


			sectionFeatures.keyDefinitions |= SectionFeatures::KeyDefinitions::HasShowMenuEntry |
											  SectionFeatures::KeyDefinitions::HasShowConsoleEntry |
											  SectionFeatures::KeyDefinitions::HasNormalizedKeyPadPrefix;

			sectionFeatures.communication &= ~(SectionFeatures::Communication::HasDialupNetworking |
											   SectionFeatures::Communication::HasCombatMacros |
											   SectionFeatures::Communication::HasRemoteRidiculeFileName);
			break;
		}

		case GameVersionType::eDuke32: {
			sectionFeatures.general &= ~SectionFeatures::General::HasComments;

			sectionFeatures.setup |= SectionFeatures::Setup::HasForceSetupEntry |
									 SectionFeatures::Setup::HasModLoadingSupport |
									 SectionFeatures::Setup::HasNoAutoLoadEntry;

			sectionFeatures.misc |= SectionFeatures::Misc::Populated;

			sectionFeatures.misc &= ~SectionFeatures::Misc::HasHeadsUpDisplayEntries;

			sectionFeatures.controls |= SectionFeatures::Controls::HasControllerControls |
										SectionFeatures::Controls::HasControllerAnalogSensitivityEntries;

			sectionFeatures.controls &= ~(SectionFeatures::Controls::HasLegacyControllerEntries |
										  SectionFeatures::Controls::HasMouseSensitivity |
										  SectionFeatures::Controls::HasExternalFileNameEntry |
										  SectionFeatures::Controls::HasRudderEntry |
										  SectionFeatures::Controls::HasMouseAimingEntry |
										  SectionFeatures::Controls::HasMouseAimFlippedEntry |
										  SectionFeatures::Controls::HasGamepadControls |
										  SectionFeatures::Controls::HasJoystickControls);

			sectionFeatures.sound &= ~(SectionFeatures::Sound::Supported |
									   SectionFeatures::Sound::HasLegacyEntries);

			sectionFeatures.screen |= SectionFeatures::Screen::HasAdvancedGraphicsEntries |
									  SectionFeatures::Screen::HasPolymerSupport |
									  SectionFeatures::Screen::HasDefaultEntries |
									  SectionFeatures::Screen::HasScreenDisplayEntry;

			sectionFeatures.screen &= ~(SectionFeatures::Screen::HasAllDefaultEntries |
										SectionFeatures::Screen::HasShadowsEntry);

			sectionFeatures.keyDefinitions &= ~SectionFeatures::KeyDefinitions::Supported;

			sectionFeatures.communication &= ~SectionFeatures::Communication::HasDialupNetworking;

			sectionFeatures.updates |= SectionFeatures::Updates::Supported;
			break;
		}

		case GameVersionType::NetDuke32: {
			sectionFeatures.general &= ~SectionFeatures::General::HasComments;

			sectionFeatures.setup |= SectionFeatures::Setup::HasForceSetupEntry |
									 SectionFeatures::Setup::HasModLoadingSupport;

			sectionFeatures.misc |= SectionFeatures::Misc::Populated |
									SectionFeatures::Misc::HasUsePrecacheEntry |
									SectionFeatures::Misc::HasPredictionDebugEntry;

			sectionFeatures.misc &= ~SectionFeatures::Misc::HasHeadsUpDisplayEntries;

			sectionFeatures.controls |= SectionFeatures::Controls::HasControllerControls |
										SectionFeatures::Controls::HasUseMouseAndJoystickEntries |
										SectionFeatures::Controls::HasControllerAnalogScaleEntries |
										SectionFeatures::Controls::HasControllerAnalogSensitivityEntries;

			sectionFeatures.controls &= ~(SectionFeatures::Controls::HasLegacyControllerEntries |
										  SectionFeatures::Controls::HasMouseAnalogScaleEntries |
										  SectionFeatures::Controls::HasMouseSensitivity |
										  SectionFeatures::Controls::HasExternalFileNameEntry |
										  SectionFeatures::Controls::HasRudderEntry |
										  SectionFeatures::Controls::HasMouseAimingEntry |
										  SectionFeatures::Controls::HasMouseAimFlippedEntry |
										  SectionFeatures::Controls::HasGamepadControls |
										  SectionFeatures::Controls::HasJoystickControls);

			sectionFeatures.sound &= ~(SectionFeatures::Sound::Supported |
									   SectionFeatures::Sound::HasLegacyEntries);

			sectionFeatures.screen |= SectionFeatures::Screen::HasAdvancedGraphicsEntries |
									  SectionFeatures::Screen::HasPolymerSupport |
									  SectionFeatures::Screen::HasAmbientLightEntry |
									  SectionFeatures::Screen::HasUseModelsEntry |
									  SectionFeatures::Screen::HasDefaultEntries |
									  SectionFeatures::Screen::HasOpenGLAnimationSmoothingEntry |
									  SectionFeatures::Screen::HasOpenGLTextureModeEntry |
									  SectionFeatures::Screen::HasOpenGLUseTextureCompressionEntry;

			sectionFeatures.screen &= ~SectionFeatures::Screen::HasShadowsEntry;

			sectionFeatures.keyDefinitions &= ~SectionFeatures::KeyDefinitions::Supported;

			sectionFeatures.communication &= ~SectionFeatures::Communication::HasDialupNetworking;
			break;
		}

		case GameVersionType::RedNukem: {
			sectionFeatures.general &= ~SectionFeatures::General::HasComments;

			sectionFeatures.setup |= SectionFeatures::Setup::HasForceSetupEntry |
									 SectionFeatures::Setup::HasModLoadingSupport |
									 SectionFeatures::Setup::HasNoAutoLoadEntry;

			sectionFeatures.misc |= SectionFeatures::Misc::Populated;

			sectionFeatures.misc &= ~SectionFeatures::Misc::HasHeadsUpDisplayEntries;

			sectionFeatures.controls &= ~(SectionFeatures::Controls::HasLegacyControllerEntries |
										  SectionFeatures::Controls::HasMouseAnalogScaleEntries |
										  SectionFeatures::Controls::HasMouseDigitalEntries |
										  SectionFeatures::Controls::HasMouseSensitivity |
										  SectionFeatures::Controls::HasExternalFileNameEntry |
										  SectionFeatures::Controls::HasRudderEntry |
										  SectionFeatures::Controls::HasMouseAimingEntry |
										  SectionFeatures::Controls::HasMouseAimFlippedEntry |
										  SectionFeatures::Controls::HasMouseButtonClickedEntries |
										  SectionFeatures::Controls::HasGamepadControls |
										  SectionFeatures::Controls::HasJoystickControls);

			sectionFeatures.sound &= ~(SectionFeatures::Sound::Supported |
									   SectionFeatures::Sound::HasLegacyEntries);

			sectionFeatures.screen |= SectionFeatures::Screen::HasAdvancedGraphicsEntries |
									  SectionFeatures::Screen::HasPolymerSupport |
									  SectionFeatures::Screen::HasDefaultEntries |
									  SectionFeatures::Screen::HasWindowPropertyEntries;

			sectionFeatures.screen &= ~(SectionFeatures::Screen::HasAllDefaultEntries |
										SectionFeatures::Screen::HasShadowsEntry);

			sectionFeatures.keyDefinitions &= ~SectionFeatures::KeyDefinitions::Supported;

			sectionFeatures.communication &= ~SectionFeatures::Communication::HasDialupNetworking;

			sectionFeatures.updates |= SectionFeatures::Updates::Supported;
			break;
		}

		case GameVersionType::BelgianChocolate: {
			sectionFeatures.general &= ~SectionFeatures::General::HasComments;

			sectionFeatures.setup &= ~SectionFeatures::Setup::Populated;

			sectionFeatures.misc |= SectionFeatures::Misc::Populated |
									SectionFeatures::Misc::HasAutoAimEntry |
									SectionFeatures::Misc::HasShowCinematicsEntry |
									SectionFeatures::Misc::HasWeaponVisibilityEntries |
									SectionFeatures::Misc::HasWeaponAutoSwitchEntry |
									SectionFeatures::Misc::HasWeaponChoicePreferences;

			sectionFeatures.controls |= SectionFeatures::Controls::HasRancidMeatMouseSensitivityEntries |
										SectionFeatures::Controls::HasGameMouseAimingEntry |
										SectionFeatures::Controls::HasAimingFlagEntry;

			sectionFeatures.controls &= ~(SectionFeatures::Controls::HasJoystickPortEntry |
										  SectionFeatures::Controls::HasMouseAnalogAxesEntries |
										  SectionFeatures::Controls::HasMouseAnalogScaleEntries |
										  SectionFeatures::Controls::HasMouseSensitivity |
										  SectionFeatures::Controls::HasExternalFileNameEntry |
										  SectionFeatures::Controls::HasRudderEntry |
										  SectionFeatures::Controls::HasMouseButtonClickedEntries |
										  SectionFeatures::Controls::HasGamepadControls |
										  SectionFeatures::Controls::HasJoystickControls);

			sectionFeatures.sound |= SectionFeatures::Sound::HasOpponentSoundToggleEntry |
									 SectionFeatures::Sound::HasDefaultEntries;

			sectionFeatures.sound &= ~(SectionFeatures::Sound::HasLegacyEntries |
									   SectionFeatures::Sound::HasQualityEntries);

			sectionFeatures.screen |= SectionFeatures::Screen::HasDefaultEntries |
									  SectionFeatures::Screen::HasShowFramesPerSecondEntry |
									  SectionFeatures::Screen::HasFullScreenEntry |
									  SectionFeatures::Screen::HasExtendedScreenSizeEntry;

			sectionFeatures.screen &= ~SectionFeatures::Screen::HasScreenModeEntry;

			sectionFeatures.keyDefinitions |= SectionFeatures::KeyDefinitions::HasConsoleEntry |
											  SectionFeatures::KeyDefinitions::HasHideWeaponEntry |
											  SectionFeatures::KeyDefinitions::HasAutoAimEntry |
											  SectionFeatures::KeyDefinitions::HasNormalizedKeyPadPrefix;

			sectionFeatures.communication &= ~SectionFeatures::Communication::HasDialupNetworking;
			break;
		}

		case GameVersionType::Duke3dw: {
			sectionFeatures.general &= ~SectionFeatures::General::HasComments;

			sectionFeatures.setup |= SectionFeatures::Setup::HasForceSetupEntry;

			sectionFeatures.misc |= SectionFeatures::Misc::Populated |
									SectionFeatures::Misc::HasExtendedHeadsUpDisplayEntries |
									SectionFeatures::Misc::HasWeaponIconsEntry |
									SectionFeatures::Misc::HasWeaponHideEntry |
									SectionFeatures::Misc::HasAutoSaveEntry |
									SectionFeatures::Misc::HasWeaponChoicePreferences;

			sectionFeatures.controls |= SectionFeatures::Controls::HasAutoAimEntry |
										SectionFeatures::Controls::HasWeaponSwitchModeEntry |
										SectionFeatures::Controls::HasRunKeyBehaviourEntry |
										SectionFeatures::Controls::HasAimingFlagEntry |
										SectionFeatures::Controls::HasJoystickAnalogDeadZoneEntries |
										SectionFeatures::Controls::HasJoystickAnalogSaturateEntries |
										SectionFeatures::Controls::HasUseMouseAndJoystickEntries |
										SectionFeatures::Controls::HasUseMouseAndJoystickEntries;

			sectionFeatures.controls &= ~(SectionFeatures::Controls::HasLegacyControllerEntries |
										  SectionFeatures::Controls::HasExternalFileNameEntry |
										  SectionFeatures::Controls::HasRudderEntry |
										  SectionFeatures::Controls::HasGamepadControls);

			sectionFeatures.sound |= SectionFeatures::Sound::HasRandomMusicEntry |
									 SectionFeatures::Sound::HasDefaultEntries;

			sectionFeatures.sound &= ~(SectionFeatures::Sound::HasLegacyEntries |
									   SectionFeatures::Sound::HasNumberOfChannelsEntry |
									   SectionFeatures::Sound::HasSoundDeviceEntry);

			sectionFeatures.screen |= SectionFeatures::Screen::HasAdvancedGraphicsEntries |
									  SectionFeatures::Screen::HasUseModelsEntry |
									  SectionFeatures::Screen::HasDefaultEntries |
									  SectionFeatures::Screen::HasOpenGLTextureModeEntry |
									  SectionFeatures::Screen::HasOpenGLUseTextureCompressionEntry |
									  SectionFeatures::Screen::HasOpenGLAnisotropyEntry |
									  SectionFeatures::Screen::HasOpenGLUsePreCacheEntry |
									  SectionFeatures::Screen::HasOpenGLUseCompressedTextureCacheEntry |
									  SectionFeatures::Screen::HasOpenGLUseTextureCacheCompressionEntry |
									  SectionFeatures::Screen::HasOpenGLWideScreenEntry |
									  SectionFeatures::Screen::HasOpenGLFovScreenEntry |
									  SectionFeatures::Screen::HasOpenGLVerticalSyncEntry |
									  SectionFeatures::Screen::HasOpenGLUseHighTileEntry;

			sectionFeatures.keyDefinitions |= SectionFeatures::KeyDefinitions::HasShowConsoleEntry |
											  SectionFeatures::KeyDefinitions::HasNormalizedKeyPadPrefix;


			sectionFeatures.communication |= SectionFeatures::Communication::HasPlayerColourEntry |
											 SectionFeatures::Communication::HasIPAddressEntries |
											 SectionFeatures::Communication::HasBotNameEntries;

			sectionFeatures.communication &= ~(SectionFeatures::Communication::HasDialupNetworking |
											   SectionFeatures::Communication::HasCombatMacros |
											   SectionFeatures::Communication::HasRemoteRidiculeFileName);
			break;
		}

		case GameVersionType::pkDuke3D: {
			sectionFeatures.general &= ~SectionFeatures::General::HasComments;

			sectionFeatures.setup |= SectionFeatures::Setup::HasForceSetupEntry;

			sectionFeatures.misc |= SectionFeatures::Misc::Populated |
									SectionFeatures::Misc::HasExtendedHeadsUpDisplayEntries |
									SectionFeatures::Misc::HasUsePrecacheEntry |
									SectionFeatures::Misc::HasWeaponChoicePreferences;

			sectionFeatures.controls |= SectionFeatures::Controls::HasAutoAimEntry |
										SectionFeatures::Controls::HasWeaponSwitchModeEntry |
										SectionFeatures::Controls::HasMouseYLockEntry |
										SectionFeatures::Controls::HasMouseScale |
										SectionFeatures::Controls::HasRunKeyBehaviourEntry |
										SectionFeatures::Controls::HasAimingFlagEntry |
										SectionFeatures::Controls::HasJoystickAnalogDeadZoneEntries |
										SectionFeatures::Controls::HasJoystickAnalogSaturateEntries |
										SectionFeatures::Controls::HasUseMouseAndJoystickEntries |
										SectionFeatures::Controls::HasUseMouseAndJoystickEntries;

			sectionFeatures.controls &= ~(SectionFeatures::Controls::HasExternalFileNameEntry |
										  SectionFeatures::Controls::HasRudderEntry |
										  SectionFeatures::Controls::HasGamepadControls);

			sectionFeatures.sound |= SectionFeatures::Sound::HasDefaultEntries;

			sectionFeatures.sound &= ~SectionFeatures::Sound::HasLegacyEntries;

			sectionFeatures.screen &= ~SectionFeatures::Screen::Populated;

			sectionFeatures.keyDefinitions |= SectionFeatures::KeyDefinitions::HasShowConsoleEntry |
											  SectionFeatures::KeyDefinitions::HasExtraEntries;

			sectionFeatures.communication &= ~(SectionFeatures::Communication::HasDialupNetworking |
											   SectionFeatures::Communication::HasCombatMacros |
											   SectionFeatures::Communication::HasRemoteRidiculeFileName);

			sectionFeatures.lobbyFilter |= SectionFeatures::LobbyFilter::Supported;

			sectionFeatures.multiplayer |= SectionFeatures::Multiplayer::Supported;
			break;
		}

		case GameVersionType::xDuke: {
			sectionFeatures.general &= ~SectionFeatures::General::HasComments;

			sectionFeatures.setup &= ~SectionFeatures::Setup::Populated;

			sectionFeatures.misc |= SectionFeatures::Misc::Populated |
									SectionFeatures::Misc::HasAutoAimEntry |
									SectionFeatures::Misc::HasShowCinematicsEntry |
									SectionFeatures::Misc::HasWeaponVisibilityEntries |
									SectionFeatures::Misc::HasWeaponAutoSwitchEntry |
									SectionFeatures::Misc::HasWeaponChoicePreferences;

			sectionFeatures.controls |= SectionFeatures::Controls::HasRancidMeatMouseSensitivityEntries |
										SectionFeatures::Controls::HasGameMouseAimingEntry |
										SectionFeatures::Controls::HasAimingFlagEntry;

			sectionFeatures.controls &= ~(SectionFeatures::Controls::HasJoystickPortEntry |
										  SectionFeatures::Controls::HasMouseAnalogAxesEntries |
										  SectionFeatures::Controls::HasMouseAnalogScaleEntries |
										  SectionFeatures::Controls::HasMouseSensitivity |
										  SectionFeatures::Controls::HasExternalFileNameEntry |
										  SectionFeatures::Controls::HasRudderEntry |
										  SectionFeatures::Controls::HasGamepadControls |
										  SectionFeatures::Controls::HasJoystickControls);

			sectionFeatures.sound |= SectionFeatures::Sound::HasOpponentSoundToggleEntry |
									 SectionFeatures::Sound::HasDefaultEntries;

			sectionFeatures.sound &= ~SectionFeatures::Sound::HasLegacyEntries;

			sectionFeatures.screen |= SectionFeatures::Screen::HasDefaultEntries |
									  SectionFeatures::Screen::HasCustomEntries |
									  SectionFeatures::Screen::HasShowFramesPerSecondEntry |
									  SectionFeatures::Screen::HasFullScreenEntry |
									  SectionFeatures::Screen::HasExtendedScreenSizeEntry;

			sectionFeatures.screen &= ~SectionFeatures::Screen::HasScreenModeEntry;

			sectionFeatures.keyDefinitions |= SectionFeatures::KeyDefinitions::HasConsoleEntry |
											  SectionFeatures::KeyDefinitions::HasHideWeaponEntry |
											  SectionFeatures::KeyDefinitions::HasAutoAimEntry |
											  SectionFeatures::KeyDefinitions::HasNormalizedKeyPadPrefix;

			sectionFeatures.communication &= ~SectionFeatures::Communication::HasDialupNetworking;
			break;
		}

		case GameVersionType::rDuke: {
			sectionFeatures.general &= ~SectionFeatures::General::HasComments;

			sectionFeatures.setup &= ~SectionFeatures::Setup::Populated;

			sectionFeatures.misc |= SectionFeatures::Misc::Populated |
									SectionFeatures::Misc::HasAutoAimEntry |
									SectionFeatures::Misc::HasShowCinematicsEntry |
									SectionFeatures::Misc::HasWeaponVisibilityEntries |
									SectionFeatures::Misc::HasWeaponAutoSwitchEntry |
									SectionFeatures::Misc::HasWeaponChoicePreferences;

			sectionFeatures.controls |= SectionFeatures::Controls::HasRancidMeatMouseSensitivityEntries |
										SectionFeatures::Controls::HasGameMouseAimingEntry |
										SectionFeatures::Controls::HasAimingFlagEntry;

			sectionFeatures.controls &= ~(SectionFeatures::Controls::HasJoystickPortEntry |
										  SectionFeatures::Controls::HasMouseAnalogAxesEntries |
										  SectionFeatures::Controls::HasMouseAnalogScaleEntries |
										  SectionFeatures::Controls::HasMouseSensitivity |
										  SectionFeatures::Controls::HasExternalFileNameEntry |
										  SectionFeatures::Controls::HasRudderEntry |
										  SectionFeatures::Controls::HasMouseButtonClickedEntries |
										  SectionFeatures::Controls::HasGamepadControls |
										  SectionFeatures::Controls::HasJoystickControls);

			sectionFeatures.sound |= SectionFeatures::Sound::HasOpponentSoundToggleEntry |
									 SectionFeatures::Sound::HasDefaultEntries;

			sectionFeatures.sound &= ~(SectionFeatures::Sound::HasLegacyEntries |
									   SectionFeatures::Sound::HasQualityEntries);

			sectionFeatures.screen |= SectionFeatures::Screen::HasDefaultEntries |
									  SectionFeatures::Screen::HasShowFramesPerSecondEntry |
									  SectionFeatures::Screen::HasFullScreenEntry |
									  SectionFeatures::Screen::HasWideScreenEntries |
									  SectionFeatures::Screen::HasColourFixEntry |
									  SectionFeatures::Screen::HasExtendedScreenSizeEntry;

			sectionFeatures.screen &= ~SectionFeatures::Screen::HasScreenModeEntry;

			sectionFeatures.keyDefinitions |= SectionFeatures::KeyDefinitions::HasConsoleEntry |
											  SectionFeatures::KeyDefinitions::HasHideWeaponEntry |
											  SectionFeatures::KeyDefinitions::HasAutoAimEntry |
											  SectionFeatures::KeyDefinitions::HasNormalizedKeyPadPrefix;

			sectionFeatures.communication &= ~SectionFeatures::Communication::HasDialupNetworking;
			break;
		}

		case GameVersionType::Duke3d_w32: {
			sectionFeatures.general |= SectionFeatures::General::DoesSupportFloatingPointEntryValues;

			sectionFeatures.general &= ~SectionFeatures::General::HasComments;

			sectionFeatures.misc |= SectionFeatures::Misc::Populated |
									SectionFeatures::Misc::HasWeaponChoicePreferences;

			sectionFeatures.controls |= SectionFeatures::Controls::HasGameMouseAimingEntry |
										SectionFeatures::Controls::HasAimingFlagEntry |
										SectionFeatures::Controls::HasJoystickAnalogDeadZoneEntries;

			sectionFeatures.controls &= ~(SectionFeatures::Controls::HasMouseAnalogAxesEntries |
										  SectionFeatures::Controls::HasMouseAnalogScaleEntries |
										  SectionFeatures::Controls::HasMouseDigitalEntries |
										  SectionFeatures::Controls::HasJoystickButtonClickedEntries);

			sectionFeatures.sound |= SectionFeatures::Sound::HasDefaultEntries;

			sectionFeatures.screen |= SectionFeatures::Screen::HasDefaultEntries |
									  SectionFeatures::Screen::HasEnvironmentEntry |
									  SectionFeatures::Screen::HasFullScreenEntry;

			sectionFeatures.keyDefinitions |= SectionFeatures::KeyDefinitions::HasConsoleEntry;

			sectionFeatures.communication &= ~(SectionFeatures::Communication::HasEmptyIRQNumber |
											   SectionFeatures::Communication::HasEmptyUARTAddress |
											   SectionFeatures::Communication::HasEmptySocketNumber);
			break;
		}
	}

	return sectionFeatures;
}

std::optional<GameConfiguration::SectionFeatures> GameConfiguration::getSectionFeatures() const {
	std::optional<GameVersionType> optionalGameVersionType(determineGameVersionType());

	if(!optionalGameVersionType.has_value()) {
		return {};
	}

	return getSectionFeaturesForGameVersion(optionalGameVersionType.value());
}

std::optional<GameConfiguration::GameVersionType> GameConfiguration::determineGameVersionType() const {
	std::shared_ptr<Section> screenSetupSection(getSectionWithName(SCREEN_SETUP_SECTION_NAME));

	// xDuke / rDuke
	if(screenSetupSection != nullptr &&
	   screenSetupSection->hasEntryWithName(ALTERNATE_FULL_SCREEN_ENTRY_NAME)) {

		// rDuke
		if(screenSetupSection->hasEntryWithName(SCREEN_WIDESCREEN_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(SCREEN_WIDESCREEN_HUD_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(SCREEN_COLOR_FIX_ENTRY_NAME)) {

			return GameVersionType::rDuke;
		}

		// xDuke
		if(screenSetupSection->hasEntryWithName(SCREEN_RESOLUTION_CONTROL_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(SCREEN_HUD_BACKGROUND_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(SCREEN_FRAG_BAR_LAYOUT_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(SCREEN_STATUS_BAR_WIDTH_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(SCREEN_STATUS_BAR_HEIGHT_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(SCREEN_CROSSHAIR_WIDTH_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(SCREEN_CROSSHAIR_HEIGHT_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(SCREEN_FRAG_BAR_WIDTH_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(SCREEN_FRAG_BAR_HEIGHT_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(SCREEN_MESSAGES_WIDTH_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(SCREEN_MESSAGES_HEIGHT_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(SCREEN_MENU_WIDTH_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(SCREEN_MENU_HEIGHT_ENTRY_NAME)) {

			return GameVersionType::xDuke;
		}
	}

	std::shared_ptr<Section> multiplayerSection(getSectionWithName(MULTIPLAYER_SECTION_NAME));

	// pkDuke3D
	if(multiplayerSection != nullptr &&
	   multiplayerSection->hasEntryWithName(MULTIPLAYER_SHOW_INFO_ENTRY_NAME)) {

		std::shared_ptr<Section> controlsSection(getSectionWithName(GameConfiguration::CONTROLS_SECTION_NAME));

		if(controlsSection != nullptr &&
		   controlsSection->hasEntryWithName(CONTROLS_MOUSE_Y_LOCK_ENTRY_NAME)) {

			std::shared_ptr<Section> lobbyFilterSection(getSectionWithName(LOBBY_FILTER_SECTION_NAME));

			if(lobbyFilterSection != nullptr &&
			   lobbyFilterSection->hasEntryWithName(LOBBY_FILTER_GAME_MODE_ENTRY_NAME) &&
			   lobbyFilterSection->hasEntryWithName(LOBBY_FILTER_SHOW_FULL_LOBBIES_ENTRY_NAME) &&
			   lobbyFilterSection->hasEntryWithName(LOBBY_FILTER_MAPS_ENTRY_NAME)) {

				return GameVersionType::pkDuke3D;
			}
		}
	}

	// Duke3d_w32
	std::shared_ptr<Section> keyDefinitionsSection(getSectionWithName(KEY_DEFINITIONS_SECTION_NAME));

	if(keyDefinitionsSection != nullptr &&
	   keyDefinitionsSection->hasEntryWithName(CONSOLE_ENTRY_NAME)) {

		std::shared_ptr<Section> controlsSection(getSectionWithName(GameConfiguration::CONTROLS_SECTION_NAME));

		if(controlsSection != nullptr &&
		   controlsSection->hasEntryWithName(fmt::format("{}{}{}_{}", JOYSTICK_PREFIX, HAT_SUFFIX, 0, 0))) {

			std::shared_ptr<Section> setupSection(getSectionWithName(SETUP_SECTION_NAME));

			if(setupSection != nullptr) {
				std::shared_ptr<Entry> setupVersionEntry(setupSection->getEntryWithName(SETUP_VERSION_ENTRY_NAME));

				if(setupVersionEntry != nullptr &&
				   setupVersionEntry->isString() &&
				   Utilities::areStringsEqual(setupVersionEntry->getStringValue(), REGULAR_VERSION_SETUP_VERSION)) {

					return GameVersionType::Duke3d_w32;
				}
			}
		}
	}

	// NetDuke32 / Belgian Chocolate / eDuke32 / RedNukem / JFDuke3D / Duke3dw
	if(screenSetupSection != nullptr) {
		// NetDuke32
		if(screenSetupSection->hasEntryWithName(SCREEN_AMBIENT_LIGHT_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(SCREEN_OPENGL_ANIMATION_SMOOTHING_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(SCREEN_USE_MODELS_ENTRY_NAME)) {
			std::shared_ptr<Section> miscSection(getSectionWithName(MISC_SECTION_NAME));

			if(miscSection != nullptr &&
			   miscSection->hasEntryWithName(MISC_PREDICTION_DEBUG_ENTRY_NAME)) {

				return GameVersionType::NetDuke32;
			}
		}

		// Belgian Chocolate
		if(screenSetupSection->hasEntryWithName(FULL_SCREEN_ENTRY_NAME)) {

			std::shared_ptr<Section> miscSection(getSectionWithName(MISC_SECTION_NAME));

			if(miscSection != nullptr &&
			   miscSection->hasEntryWithName(SHOW_CINEMATICS_ENTRY_NAME)) {

				return GameVersionType::BelgianChocolate;
			}
		}

		// eDuke32 / RedNukem / JFDuke3D / Duke3dw
		if(screenSetupSection->hasEntryWithName(SCREEN_BITS_PER_PIXEL_ENTRY_NAME) &&
		   screenSetupSection->hasEntryWithName(SCREEN_MAXIMUM_REFRESH_FREQUENCY_ENTRY_NAME)) {

			if(screenSetupSection->hasEntryWithName(SCREEN_EDUKE32_POLYMER_ENTRY_NAME)) {
				// RedNukem
				if(screenSetupSection->hasEntryWithName(RED_NUKEM_WINDOW_POSITIONING_ENTRY_NAME)) {
					return GameVersionType::RedNukem;
				}

				// eDuke32
				return GameVersionType::eDuke32;
			}

			// JFDuke3D / Duke3dw
			if(screenSetupSection->hasEntryWithName(SCREEN_OPENGL_ANISOTROPY_ENTRY_NAME) &&
			   screenSetupSection->hasEntryWithName(SCREEN_OPENGL_USE_COMPRESSED_TEXTURE_CACHE_ENTRY_NAME)) {

				// Duke3dw
				if(screenSetupSection->hasEntryWithName(SCREEN_OPENGL_PRECACHE_ENTRY_NAME) &&
				   screenSetupSection->hasEntryWithName(SCREEN_OPENGL_USE_TEXTURE_CACHE_COMPRESSION_ENTRY_NAME) &&
				   screenSetupSection->hasEntryWithName(SCREEN_OPENGL_WIDE_SCREEN_ENTRY_NAME) &&
				   screenSetupSection->hasEntryWithName(SCREEN_OPENGL_FOV_SCREEN_ENTRY_NAME) &&
				   screenSetupSection->hasEntryWithName(SCREEN_OPENGL_VERTICAL_SYNC_ENTRY_NAME) &&
				   screenSetupSection->hasEntryWithName(SCREEN_USE_HIGH_TILE_ENTRY_NAME) &&
				   screenSetupSection->hasEntryWithName(SCREEN_USE_MODELS_ENTRY_NAME)) {

					std::shared_ptr<Section> soundSetupSection(getSectionWithName(SOUND_SETUP_SECTION_NAME));

					if(soundSetupSection != nullptr &&
					   soundSetupSection->hasEntryWithName(SOUND_RANDOM_MUSIC_ENTRY_NAME) &&
					   !soundSetupSection->hasEntryWithName(SOUND_FX_DEVICE_ENTRY_NAME)) {

						std::shared_ptr<Section> miscSection(getSectionWithName(MISC_SECTION_NAME));

						if(miscSection != nullptr &&
						   miscSection->hasEntryWithName(MISC_WEAPON_ICONS_ENTRY_NAME) &&
						   miscSection->hasEntryWithName(MISC_WEAPON_HIDE_ENTRY_NAME) &&
						   miscSection->hasEntryWithName(MISC_AUTO_SAVE_ENTRY_NAME)) {

							return GameVersionType::Duke3dw;
						}
					}
				}

				// JFDuke3D
				std::shared_ptr<Section> keyDefinitionsSection(getSectionWithName(KEY_DEFINITIONS_SECTION_NAME));

				if(keyDefinitionsSection != nullptr &&
				   keyDefinitionsSection->hasEntryWithName(JFDUKE3D_SHOW_MENU_ENTRY_NAME)) {

					return GameVersionType::JFDuke3D;
				}
			}
		}
	}

	std::shared_ptr<Section> setupSection(getSectionWithName(SETUP_SECTION_NAME));

	if(setupSection == nullptr) {
		return {};
	}

	// Beta 0.99
	std::shared_ptr<Entry> setupVersionEntry(setupSection->getEntryWithName(SETUP_VERSION_ENTRY_NAME));

	if(setupVersionEntry == nullptr || !setupVersionEntry->isString()) {
		return GameVersionType::Beta;
	}

	// Regular 1.3D
	if(Utilities::areStringsEqual(setupVersionEntry->getStringValue(), REGULAR_VERSION_SETUP_VERSION)) {
		return GameVersionType::RegularVersion;
	}

	// Plutonium Pak 1.4 / Atomic Edition 1.5
	if(!Utilities::areStringsEqual(setupVersionEntry->getStringValue(), ATOMIC_EDITION_SETUP_VERSION)) {
		spdlog::warn("Unexpected setup version '{}', expected '{}' or '{}'. Assuming game configuration version is for Atomic Edition / Plutonium Pak.", setupVersionEntry->getStringValue(), REGULAR_VERSION_SETUP_VERSION, ATOMIC_EDITION_SETUP_VERSION);
	}

	return GameVersionType::AtomicEdition;
}

std::unique_ptr<GameConfiguration> GameConfiguration::generateDefaultGameConfiguration(const std::string & gameVersionID) {
	std::optional<GameVersionType> optionalGameVersionType(getGameVersionTypeFromID(gameVersionID));

	if(!optionalGameVersionType.has_value()) {
		return nullptr;
	}

	GameVersionType gameVersionType = optionalGameVersionType.value();

	bool isBeta = false;
	bool isRegularVersion = false;
	bool isAtomicEdition = false;
	bool isJFDuke3D = false;
	bool isEDuke32 = false;
	bool isNetDuke32 = false;
	bool isRedNukem = false;
	bool isBelgianChocolate = false;
	bool isDuke3dw = false;
	bool isPKDuke3D = false;
	bool isXDuke = false;
	bool isRDuke = false;
	bool isDuke3d_w32 = false;

	switch(gameVersionType) {
		case GameVersionType::Beta:
			isBeta = true;
			break;

		case GameVersionType::RegularVersion:
			isRegularVersion = true;
			break;

		case GameVersionType::AtomicEdition:
			isAtomicEdition = true;
			break;

		case GameVersionType::JFDuke3D:
			isJFDuke3D = true;
			break;

		case GameVersionType::eDuke32:
			isEDuke32 = true;
			break;

		case GameVersionType::NetDuke32:
			isNetDuke32 = true;
			break;

		case GameVersionType::RedNukem:
			isRedNukem = true;
			break;

		case GameVersionType::BelgianChocolate:
			isBelgianChocolate = true;
			break;

		case GameVersionType::Duke3dw:
			isDuke3dw = true;
			break;

		case GameVersionType::pkDuke3D:
			isPKDuke3D = true;
			break;

		case GameVersionType::xDuke:
			isXDuke = true;
			break;

		case GameVersionType::rDuke:
			isRDuke = true;
			break;

		case GameVersionType::Duke3d_w32:
			isDuke3d_w32 = true;
			break;
	}

	bool isEDuke32Based = isEDuke32 || isNetDuke32 || isRedNukem;
	bool isMegatonBased = isPKDuke3D;
	bool isDuke3d_w32Based = isDuke3d_w32 || isXDuke;
	bool isXDukeBased = isXDuke || isRDuke;

	SectionFeatures sectionFeatures(getSectionFeaturesForGameVersion(gameVersionType));

	// determine game version specific configuration parameters
	bool hasComments = Any(sectionFeatures.general & SectionFeatures::General::HasComments);
	bool hasNormalizedKeyPadPrefix = Any(sectionFeatures.keyDefinitions & SectionFeatures::KeyDefinitions::HasNormalizedKeyPadPrefix);
	bool shouldAddDefaultScreenSize = !isEDuke32Based;
	GameConfiguration::Style style = GameConfiguration::Style::Default;

	if(isJFDuke3D || isEDuke32Based || isBelgianChocolate || isPKDuke3D || isDuke3dw || isDuke3d_w32Based || isXDukeBased) {
		style |= GameConfiguration::Style::NewlineAfterSections;
	}

	uint16_t configurationVersion = 0;

	if(isEDuke32) {
		configurationVersion = 348;
	}
	else if(isRedNukem) {
		configurationVersion = 336;
	}
	else if(isNetDuke32) {
		configurationVersion = 189;
	}

	std::string playerName(DEFAULT_PLAYER_NAME);

	if(isJFDuke3D) {
		playerName = "Duke";
	}
	else if(isNetDuke32 || isDuke3dw) {
		playerName = "Player";
	}
	else if(isEDuke32Based) {
		playerName = "";
	}
	else if(isPKDuke3D) {
		playerName = "NoSteam";
	}
	else if(isXDukeBased) {
		playerName = "XDUKE";
	}

	bool openGLUsePreCache = 0;
	uint8_t openGLTextureMode = 5;
	bool openGLAnisotropy = false;
	bool openGLUseCompressedTextureCache = true;
	bool openGLUseTextureCompression = true;
	bool openGLUseTextureCacheCompression = true;
	bool openGLWideScreen = false;
	uint8_t openGLFieldOfViewScreen = 8;
	bool openGLVerticalSync = false;
	bool useHighTile = true;
	Dimension resolution(DEFAULT_RESOLUTION);

	if(isJFDuke3D || isBelgianChocolate || isDuke3d_w32Based) {
		resolution = Dimension(640, 480);
	}
	else if(isDuke3dw) {
		openGLTextureMode = 3;
		openGLAnisotropy = true;
		openGLUseCompressedTextureCache = false;
		resolution = Dimension(1024, 768);
	}
	else if(isRDuke) {
		resolution = Dimension(800, 600);
	}

	uint64_t soundDevice = SOUND_DEVICE_UNSET;
	uint64_t musicDevice = MUSIC_DEVICE_UNSET;
	uint8_t soundNumberOfVoices = 8;
	uint8_t soundNumberOfBits = 1;
	uint16_t soundMixRate = 11000;
	bool randomMusic = false;
	uint16_t soundVolume = 220;
	uint16_t musicVolume = 200;
	uint8_t blasterType = 6;
	uint8_t blasterInterrupt = 7;

	if(isJFDuke3D || isBelgianChocolate || isPKDuke3D || isDuke3dw || isDuke3d_w32) {
		soundDevice = 0;
		musicDevice = 0;
	}
	else if(isXDukeBased) {
		soundDevice = 8;
		musicDevice = -1;
	}

	if(isJFDuke3D || isMegatonBased) {
		soundNumberOfVoices = 16;
		soundNumberOfBits = 16;
		soundMixRate = 44100;
	}
	else if(isDuke3dw) {
		soundNumberOfVoices = 32;
		soundNumberOfBits = 16;
		soundMixRate = 44100;
	}
	else if(isDuke3d_w32) {
		soundNumberOfVoices = 64;
		soundNumberOfBits = 16;
		soundMixRate = 48000;
		soundVolume = 188;
		musicVolume = 252;
		blasterType = 3;
		blasterInterrupt = 5;
	}
	else if(isXDuke) {
		soundNumberOfVoices = 32;
		soundMixRate = 44100;
		soundVolume = 256;
		musicVolume = 256;
		soundNumberOfBits = 16;
	}

	uint8_t screenMode = 2;
	uint8_t screenBitsPerPixel = 1;
	uint8_t screenMaximumRefreshFrequency = 60;
	uint8_t screenGamma = 0;
	uint8_t screenSize = 8;
	uint8_t extendedScreenSize = 0;
	bool fullScreen = false;

	if(isNetDuke32) {
		screenMode = 0;
	}
	else if(isJFDuke3D || isEDuke32Based || isDuke3dw || isDuke3d_w32) {
		screenMode = 1;
	}

	if(isJFDuke3D) {
		screenBitsPerPixel = 8;
	}
	else if(isEDuke32Based) {
		screenBitsPerPixel = 32;
		screenMaximumRefreshFrequency = 0;
	}
	else if(isDuke3dw) {
		screenBitsPerPixel = 32;
	}

	if(isBelgianChocolate) {
		screenGamma = 16;
	}
	else if(isNetDuke32) {
		screenGamma = 8;
		screenSize = 4;
	}
	else if(isDuke3dw) {
		screenGamma = 20;
	}
	else if(isDuke3d_w32) {
		screenGamma = 28;
	}
	else if(isXDuke) {
		screenGamma = 16;
		fullScreen = true;
	}
	else if(isRDuke) {
		screenSize = 4;
		extendedScreenSize = 1;
		fullScreen = true;
	}

	bool aimingFlag = false;
	bool gameMouseAiming = false;
	bool mouseAimingFlipped = false;
	uint8_t autoAim = 0;
	uint16_t statusBarScale = 8;
	bool weaponIcons = true;
	bool weaponHide = false;
	bool autoSave = true;
	bool runModeEnabled = false;
	bool crosshairEnabled = false;
	uint32_t mouseSensitivity = std::numeric_limits<int16_t>::max() + 1;
	uint32_t mouseAnalogScale = DEFAULT_ANALOG_SCALE;
	uint32_t mouseScale = 10;
	uint8_t rancidMeatXMouseSensitivity = 16;
	uint8_t rancidMeatYMouseSensitivity = 16;

	if(isJFDuke3D) {
		autoAim = 1;
	}
	else if(isBelgianChocolate) {
		autoAim = 2;
	}
	else if(isPKDuke3D) {
		mouseSensitivity = 36864;
		mouseAnalogScale = 0;
	}
	else if(isDuke3dw) {
		mouseAimingFlipped = true;
	}
	else if(isDuke3d_w32) {
		aimingFlag = true;
		gameMouseAiming = true;
		mouseAimingFlipped = true;
		mouseSensitivity = 19456;
	}
	else if(isXDuke) {
		autoAim = 2;
		rancidMeatXMouseSensitivity = 4;
		rancidMeatYMouseSensitivity = 8;
	}
	else if(isRDuke) {
		aimingFlag = true;
		mouseAimingFlipped = true;
		autoAim = 2;
	}

	if(isPKDuke3D || isDuke3dw) {
		aimingFlag = true;
		statusBarScale = 100;
	}

	if(isBelgianChocolate || isMegatonBased || isDuke3dw || isDuke3d_w32Based || isXDukeBased) {
		runModeEnabled = true;
		crosshairEnabled = true;
	}

	std::string unboundKeyValue;
	std::string hideWeaponKey("S");

	if(isMegatonBased) {
		unboundKeyValue = "None";
	}
	else if(isRDuke) {
		hideWeaponKey = "P";
	}

	uint8_t numberOfMouseButtons = 3;
	uint8_t numberOfClickableMouseButtons = numberOfMouseButtons;

	if(isJFDuke3D || isMegatonBased || isDuke3dw) {
		numberOfMouseButtons = 6;
		numberOfClickableMouseButtons = 4;
	}
	else if(isEDuke32 || isNetDuke32) {
		numberOfMouseButtons = 10;
		numberOfClickableMouseButtons = 8;
	}
	else if(isBelgianChocolate || isXDuke) {
		numberOfMouseButtons = 7;
		numberOfClickableMouseButtons = 0;
	}
	else if(isRedNukem) {
		numberOfMouseButtons = 6;
		numberOfClickableMouseButtons = 0;
	}
	else if(isDuke3d_w32) {
		numberOfMouseButtons = 5;
		numberOfClickableMouseButtons = 5;
	}
	else if(isRDuke) {
		numberOfMouseButtons = 7;
		numberOfClickableMouseButtons = 0;
	}

	bool useJoystick = true;
	bool useMouse = true;
	uint8_t numberOfJoystickButtons = 8;
	uint8_t numberOfJoysticks = 4;
	uint32_t joystickAnalogScale = DEFAULT_ANALOG_SCALE;
	uint32_t joystickAnalogDeadZone = DEFAULT_JOYSTICK_ANALOG_DEADZONE;
	uint32_t joystickAnalogSaturate = DEFAULT_JOYSTICK_ANALOG_SATURATE;

	if(isJFDuke3D) {
		numberOfJoystickButtons = 32;
		numberOfJoysticks = 12;
	}
	else if(isNetDuke32) {
		useJoystick = false;
	}
	else if(isMegatonBased || isDuke3dw) {
		useJoystick = false;
		numberOfJoystickButtons = 36;
		numberOfJoysticks = 8;

		if(isPKDuke3D) {
			joystickAnalogScale = 0;
			joystickAnalogDeadZone = 0;
			joystickAnalogSaturate = 0;
		}
		else if(isDuke3dw) {
			joystickAnalogDeadZone = DEFAULT_CONTROLLER_ANALOG_DEADZONE;
			joystickAnalogSaturate = DEFAULT_CONTROLLER_ANALOG_SATURATE;
		}
	}
	else if(isDuke3d_w32) {
		numberOfJoysticks = 6;
		joystickAnalogDeadZone = 5000;
	}

	uint8_t controllerType = 1;
	uint16_t controllerAnalogDeadZone = DEFAULT_CONTROLLER_ANALOG_DEADZONE;
	uint16_t controllerAnalogSaturate = DEFAULT_CONTROLLER_ANALOG_SATURATE;
	uint8_t numberOfControllerButtons = 0;
	uint8_t numberOfControllerThumbSticks = 0;

	if(isNetDuke32) {
		controllerAnalogDeadZone = 0;
		controllerAnalogSaturate = 0;
	}
	else if(isPKDuke3D) {
		numberOfControllerThumbSticks = 8;
	}
	else if(isDuke3d_w32) {
		controllerType = 7;
	}

	if(isEDuke32 || isNetDuke32) {
		numberOfControllerButtons = 36;
		numberOfControllerThumbSticks = 9;
	}

	uint8_t playerColor = 0;
	uint8_t numberOfIPAddressEntries = 8;
	uint8_t numberOfBotNameEntries = 8;
	size_t numberOfPhoneNumbers = 10;
	uint8_t irqNumber = 0;
	uint16_t uartAddress = std::numeric_limits<uint16_t>::max();
	uint16_t socketNumber = std::numeric_limits<uint16_t>::max();

	if(isAtomicEdition) {
		numberOfPhoneNumbers = 16;
	}

	std::string showConsoleKeyA;
	std::string showConsoleKeyB;

	if(isDuke3dw) {
		showConsoleKeyA = "`";
		showConsoleKeyB = "NumLck";
	}
	else {
		showConsoleKeyA = unboundKeyValue;
		showConsoleKeyB = unboundKeyValue;
	}

	// create configuration
	std::unique_ptr<GameConfiguration> gameConfiguration(std::make_unique<GameConfiguration>());
	gameConfiguration->setStyle(style);

	if(Any(sectionFeatures.setup & SectionFeatures::Setup::Supported) && Any(sectionFeatures.setup & SectionFeatures::Setup::Populated)) {
		// create setup section
		std::shared_ptr<Section> setupSection(std::make_shared<Section>(SETUP_SECTION_NAME, "", hasComments ? "Setup File for Duke Nukem 3D" : ""));
		gameConfiguration->addSection(setupSection);

		if(isRegularVersion || isDuke3d_w32) {
			setupSection->addStringEntry(SETUP_VERSION_ENTRY_NAME, REGULAR_VERSION_SETUP_VERSION);
		}
		else if(isAtomicEdition) {
			setupSection->addStringEntry(SETUP_VERSION_ENTRY_NAME, ATOMIC_EDITION_SETUP_VERSION);
		}

		if(isEDuke32Based) {
			setupSection->addIntegerEntry(SETUP_EDUKE32_CONFIGURATION_VERSION, configurationVersion);
		}

		if(Any(sectionFeatures.setup & SectionFeatures::Setup::HasForceSetupEntry)) {
			setupSection->addIntegerEntry("ForceSetup", 1);
		}

		if(Any(sectionFeatures.setup & SectionFeatures::Setup::HasModLoadingSupport)) {
			if(Any(sectionFeatures.setup & SectionFeatures::Setup::HasNoAutoLoadEntry)) {
				setupSection->addIntegerEntry("NoAutoLoad", 1);
			}

			setupSection->addStringEntry("SelectedGRP", GroupGRP::DUKE_NUKEM_3D_GROUP_FILE_NAME);
			setupSection->addStringEntry("ModDir", "/");
		}
	}

	// create misc section
	if(Any(sectionFeatures.misc & SectionFeatures::Misc::Supported) && Any(sectionFeatures.misc & SectionFeatures::Misc::Populated)) {
		std::shared_ptr<Section> miscSection(std::make_shared<Section>(MISC_SECTION_NAME));
		gameConfiguration->addSection(miscSection);

		miscSection->addIntegerEntry(MISC_EXECUTIONS_ENTRY_NAME, 0);

		if(Any(sectionFeatures.misc & SectionFeatures::Misc::HasHeadsUpDisplayEntries)) {
			miscSection->addIntegerEntry(MISC_RUN_MODE_ENTRY_NAME, runModeEnabled ? 1 : 0);
			miscSection->addIntegerEntry(MISC_CROSSHAIRS_ENTRY_NAME, crosshairEnabled ? 1 : 0);
		}

		if(Any(sectionFeatures.misc & SectionFeatures::Misc::HasShowCinematicsEntry)) {
			miscSection->addIntegerEntry(SHOW_CINEMATICS_ENTRY_NAME, 1);
		}

		if(Any(sectionFeatures.misc & SectionFeatures::Misc::HasWeaponVisibilityEntries)) {
			miscSection->addIntegerEntry("HideWeapon", 0);
			miscSection->addIntegerEntry("ShowWeapon", 0);
		}

		if(Any(sectionFeatures.misc & SectionFeatures::Misc::HasWeaponAutoSwitchEntry)) {
			miscSection->addIntegerEntry("WeaponAutoSwitch", 0);
		}

		if(Any(sectionFeatures.misc & SectionFeatures::Misc::HasAutoAimEntry)) {
			miscSection->addIntegerEntry(MISC_AUTO_AIM_ENTRY_NAME, autoAim);
		}

		if(Any(sectionFeatures.misc & SectionFeatures::Misc::HasExtendedHeadsUpDisplayEntries)) {
			miscSection->addIntegerEntry(MISC_SHOW_LEVEL_STATISTICS_ENTRY_NAME, 0);
			miscSection->addIntegerEntry(MISC_STATUS_BAR_SCALE_ENTRY_NAME, statusBarScale);
			miscSection->addIntegerEntry(MISC_SHOW_OPPONENT_WEAPONS_ENTRY_NAME, 0);
		}

		if(Any(sectionFeatures.misc & SectionFeatures::Misc::HasWeaponIconsEntry)) {
			miscSection->addIntegerEntry(MISC_WEAPON_ICONS_ENTRY_NAME, weaponIcons ? 1 : 0);
		}

		if(Any(sectionFeatures.misc & SectionFeatures::Misc::HasWeaponHideEntry)) {
			miscSection->addIntegerEntry(MISC_WEAPON_HIDE_ENTRY_NAME, weaponHide ? 1 : 0);
		}

		if(Any(sectionFeatures.misc & SectionFeatures::Misc::HasAutoSaveEntry)) {
			miscSection->addIntegerEntry(MISC_AUTO_SAVE_ENTRY_NAME, autoSave ? 1 : 0);
		}

		if(Any(sectionFeatures.misc & SectionFeatures::Misc::HasUsePrecacheEntry)) {
			miscSection->addIntegerEntry(MISC_USE_PRECACHE_ENTRY_NAME, 1);
		}

		if(Any(sectionFeatures.misc & SectionFeatures::Misc::HasPredictionDebugEntry)) {
			miscSection->addIntegerEntry(MISC_PREDICTION_DEBUG_ENTRY_NAME, 0);
		}

		if(Any(sectionFeatures.misc & SectionFeatures::Misc::HasWeaponChoicePreferences)) {
			for(size_t i = 0; i < DEFAULT_WEAPON_CHOICE_PREFERENCES.size(); i++) {
				miscSection->addIntegerEntry(fmt::format("{}{}", WEAPON_CHOICE_PREFIX, i), DEFAULT_WEAPON_CHOICE_PREFERENCES[i]);
			}
		}
	}

	if(Any(sectionFeatures.screen & SectionFeatures::Screen::Supported) && Any(sectionFeatures.screen & SectionFeatures::Screen::Populated)) {
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

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasScreenModeEntry)) {
			screenSetupSection->addIntegerEntry(SCREEN_MODE_ENTRY_NAME, screenMode);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasDefaultEntries)) {
			if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasShadowsEntry)) {
				screenSetupSection->addIntegerEntry(SCREEN_SHADOWS_ENTRY_NAME, 1);
			}

			screenSetupSection->addStringEntry(SCREEN_PASSWORD_ENTRY_NAME, "");

			if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasEnvironmentEntry)) {
				screenSetupSection->addStringEntry(SCREEN_ENVIRONMENT_ENTRY_NAME, "");
			}

			if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasAllDefaultEntries)) {
				screenSetupSection->addIntegerEntry(SCREEN_DETAIL_ENTRY_NAME, 1);
				screenSetupSection->addIntegerEntry(SCREEN_TILT_ENTRY_NAME, 1);
				screenSetupSection->addIntegerEntry(SCREEN_MESSAGES_ENTRY_NAME, 1);
			}

			screenSetupSection->addIntegerEntry(SCREEN_OUT_ENTRY_NAME, 0);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasUseModelsEntry)) {
			screenSetupSection->addIntegerEntry(SCREEN_USE_MODELS_ENTRY_NAME, 1);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasShowFramesPerSecondEntry)) {
			screenSetupSection->addIntegerEntry(SHOW_FRAMES_PER_SECOND_ENTRY_NAME, 0);
		}

		if(shouldAddDefaultScreenSize) {
			screenSetupSection->addIntegerEntry(SCREEN_WIDTH_ENTRY_NAME, resolution.w);
			screenSetupSection->addIntegerEntry(SCREEN_HEIGHT_ENTRY_NAME, resolution.h);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasWindowPropertyEntries)) {
			screenSetupSection->addIntegerEntry(RED_NUKEM_WINDOW_POSITIONING_ENTRY_NAME, 0);
			screenSetupSection->addIntegerEntry(RED_NUKEM_WINDOW_X_POSITION_ENTRY_NAME, -1);
			screenSetupSection->addIntegerEntry(RED_NUKEM_WINDOW_Y_POSITION_ENTRY_NAME, -1);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasAdvancedGraphicsEntries)) {
			if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasPolymerSupport)) {
				screenSetupSection->addIntegerEntry(SCREEN_EDUKE32_POLYMER_ENTRY_NAME, 0);
			}

			if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasAmbientLightEntry)) {
				screenSetupSection->addStringEntry(SCREEN_AMBIENT_LIGHT_ENTRY_NAME, "1.00");
			}

			screenSetupSection->addIntegerEntry(SCREEN_BITS_PER_PIXEL_ENTRY_NAME, screenBitsPerPixel);

			if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasScreenDisplayEntry)) {
				screenSetupSection->addIntegerEntry("ScreenDisplay", 0);
			}

			screenSetupSection->addIntegerEntry(SCREEN_MAXIMUM_REFRESH_FREQUENCY_ENTRY_NAME, screenMaximumRefreshFrequency);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasOpenGLAnimationSmoothingEntry)) {
			screenSetupSection->addIntegerEntry(SCREEN_OPENGL_ANIMATION_SMOOTHING_ENTRY_NAME, 1);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasOpenGLTextureModeEntry)) {
			screenSetupSection->addIntegerEntry(SCREEN_OPENGL_TEXTURE_MODE_ENTRY_NAME, openGLTextureMode);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasOpenGLAnisotropyEntry)) {
			screenSetupSection->addIntegerEntry(SCREEN_OPENGL_ANISOTROPY_ENTRY_NAME, openGLAnisotropy ? 1 : 0);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasOpenGLUsePreCacheEntry)) {
			screenSetupSection->addIntegerEntry(SCREEN_OPENGL_PRECACHE_ENTRY_NAME, openGLUsePreCache);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasOpenGLUseTextureCompressionEntry)) {
			screenSetupSection->addIntegerEntry(SCREEN_OPENGL_USE_TEXTURE_COMPRESSION_ENTRY_NAME, openGLUseTextureCompression ? 1 : 0);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasOpenGLUseCompressedTextureCacheEntry)) {
			screenSetupSection->addIntegerEntry(SCREEN_OPENGL_USE_COMPRESSED_TEXTURE_CACHE_ENTRY_NAME, openGLUseCompressedTextureCache ? 1 : 0);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasOpenGLUseTextureCacheCompressionEntry)) {
			screenSetupSection->addIntegerEntry(SCREEN_OPENGL_USE_TEXTURE_CACHE_COMPRESSION_ENTRY_NAME, openGLUseTextureCacheCompression ? 1 : 0);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasOpenGLWideScreenEntry)) {
			screenSetupSection->addIntegerEntry(SCREEN_OPENGL_WIDE_SCREEN_ENTRY_NAME, openGLWideScreen ? 1 : 0);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasOpenGLFovScreenEntry)) {
			screenSetupSection->addIntegerEntry(SCREEN_OPENGL_FOV_SCREEN_ENTRY_NAME, openGLFieldOfViewScreen);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasOpenGLVerticalSyncEntry)) {
			screenSetupSection->addIntegerEntry(SCREEN_OPENGL_VERTICAL_SYNC_ENTRY_NAME, openGLVerticalSync ? 1 : 0);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasOpenGLUseHighTileEntry)) {
			screenSetupSection->addIntegerEntry(SCREEN_USE_HIGH_TILE_ENTRY_NAME, useHighTile ? 1 : 0);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasFullScreenEntry)) {
			if(isBelgianChocolate) {
				screenSetupSection->addIntegerEntry(FULL_SCREEN_ENTRY_NAME, fullScreen ? 1 : 0);
			}
			else if(isDuke3d_w32 || isXDuke || isRDuke) {
				screenSetupSection->addIntegerEntry(ALTERNATE_FULL_SCREEN_ENTRY_NAME, fullScreen ? 1 : 0);
			}
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasWideScreenEntries)) {
			screenSetupSection->addIntegerEntry(SCREEN_WIDESCREEN_ENTRY_NAME, 1);
			screenSetupSection->addIntegerEntry(SCREEN_WIDESCREEN_HUD_ENTRY_NAME, 2);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasColourFixEntry)) {
			screenSetupSection->addIntegerEntry(SCREEN_COLOR_FIX_ENTRY_NAME, 0);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasAllDefaultEntries)) {
			screenSetupSection->addIntegerEntry(SCREEN_SIZE_ENTRY_NAME, screenSize);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasExtendedScreenSizeEntry)) {
			screenSetupSection->addIntegerEntry(EXTENDED_SCREEN_SIZE_ENTRY_NAME, extendedScreenSize);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasAllDefaultEntries)) {
			screenSetupSection->addIntegerEntry(SCREEN_GAMMA_ENTRY_NAME, screenGamma);
		}

		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasCustomEntries)) {
			screenSetupSection->addIntegerEntry(SCREEN_RESOLUTION_CONTROL_ENTRY_NAME, 0);
			screenSetupSection->addIntegerEntry(SCREEN_HUD_BACKGROUND_ENTRY_NAME, 1);
			screenSetupSection->addIntegerEntry(SCREEN_FRAG_BAR_LAYOUT_ENTRY_NAME, 0);
			screenSetupSection->addIntegerEntry(SCREEN_STATUS_BAR_WIDTH_ENTRY_NAME, 640);
			screenSetupSection->addIntegerEntry(SCREEN_STATUS_BAR_HEIGHT_ENTRY_NAME, 480);
			screenSetupSection->addIntegerEntry(SCREEN_CROSSHAIR_WIDTH_ENTRY_NAME, 640);
			screenSetupSection->addIntegerEntry(SCREEN_CROSSHAIR_HEIGHT_ENTRY_NAME, 480);
			screenSetupSection->addIntegerEntry(SCREEN_FRAG_BAR_WIDTH_ENTRY_NAME, 640);
			screenSetupSection->addIntegerEntry(SCREEN_FRAG_BAR_HEIGHT_ENTRY_NAME, 480);
			screenSetupSection->addIntegerEntry(SCREEN_MESSAGES_WIDTH_ENTRY_NAME, 640);
			screenSetupSection->addIntegerEntry(SCREEN_MESSAGES_HEIGHT_ENTRY_NAME, 480);
			screenSetupSection->addIntegerEntry(SCREEN_MENU_WIDTH_ENTRY_NAME, 640);
			screenSetupSection->addIntegerEntry(SCREEN_MENU_HEIGHT_ENTRY_NAME, 480);
		}
	}

	if(Any(sectionFeatures.updates & SectionFeatures::Updates::Supported) && Any(sectionFeatures.updates & SectionFeatures::Updates::Populated)) {
		// create updates section
		std::shared_ptr<Section> updatesSection(std::make_shared<Section>("Updates"));
		gameConfiguration->addSection(updatesSection);

		updatesSection->addIntegerEntry("CheckForUpdates", 1);
	}

	if(Any(sectionFeatures.sound & SectionFeatures::Sound::Supported) && Any(sectionFeatures.sound & SectionFeatures::Sound::Populated)) {
		// create sound setup section
		std::shared_ptr<Section> soundSetupSection(std::make_shared<Section>(SOUND_SETUP_SECTION_NAME, hasComments ? " \n " : "", hasComments ? " \n " : ""));
		gameConfiguration->addSection(soundSetupSection);

		if(Any(sectionFeatures.sound & SectionFeatures::Sound::HasSoundDeviceEntry)) {
			soundSetupSection->addIntegerEntry(SOUND_FX_DEVICE_ENTRY_NAME, soundDevice);
		}

		soundSetupSection->addIntegerEntry(SOUND_MUSIC_DEVICE_ENTRY_NAME, musicDevice);

		soundSetupSection->addIntegerEntry(SOUND_FX_VOLUME_ENTRY_NAME, soundVolume);
		soundSetupSection->addIntegerEntry(SOUND_MUSIC_VOLUME_ENTRY_NAME, musicVolume);

		if(Any(sectionFeatures.sound & SectionFeatures::Sound::HasDefaultEntries)) {
			soundSetupSection->addIntegerEntry(SOUND_SOUND_TOGGLE_ENTRY_NAME, 1);
			soundSetupSection->addIntegerEntry(SOUND_VOICE_TOGGLE_ENTRY_NAME, 1);
			soundSetupSection->addIntegerEntry(SOUND_AMBIENCE_TOGGLE_ENTRY_NAME, 1);

			if(Any(sectionFeatures.sound & SectionFeatures::Sound::HasOpponentSoundToggleEntry)) {
				soundSetupSection->addIntegerEntry(OPPONENT_SOUND_TOGGLE_ENTRY_NAME, 1);
			}

			soundSetupSection->addIntegerEntry(SOUND_MUSIC_TOGGLE_ENTRY_NAME, 1);
		}

		if(Any(sectionFeatures.sound & SectionFeatures::Sound::HasRandomMusicEntry)) {
			soundSetupSection->addIntegerEntry(SOUND_RANDOM_MUSIC_ENTRY_NAME, randomMusic ? 1 : 0);
		}

		if(Any(sectionFeatures.sound & SectionFeatures::Sound::HasQualityEntries)) {
			soundSetupSection->addIntegerEntry("NumVoices", soundNumberOfVoices);

			if(Any(sectionFeatures.sound & SectionFeatures::Sound::HasNumberOfChannelsEntry)) {
				soundSetupSection->addIntegerEntry("NumChannels", 2);
			}

			soundSetupSection->addIntegerEntry(SOUND_NUM_BITS_ENTRY_NAME, soundNumberOfBits);
			soundSetupSection->addIntegerEntry(SOUND_MIX_RATE_ENTRY_NAME, soundMixRate);
		}

		if(Any(sectionFeatures.sound & SectionFeatures::Sound::HasLegacyEntries)) {
			soundSetupSection->addHexadecimalEntryUsingDecimal("MidiPort", 0x330);
			soundSetupSection->addHexadecimalEntryUsingDecimal("BlasterAddress", 0x220);
			soundSetupSection->addIntegerEntry("BlasterType", blasterType);
			soundSetupSection->addIntegerEntry("BlasterInterrupt", blasterInterrupt);
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
	if(Any(sectionFeatures.keyDefinitions & SectionFeatures::KeyDefinitions::Supported) && Any(sectionFeatures.keyDefinitions & SectionFeatures::KeyDefinitions::Populated)) {
		const auto * keyDefinitions = &DEFAULT_KEY_DEFINITIONS;

		if(isNetDuke32) {
			configurationVersion = 189;
		}
		else if(isPKDuke3D) {
			keyDefinitions = &DEFAULT_PKDUKE3D_KEY_DEFINITIONS;
		}
		else if(isDuke3dw) {
			keyDefinitions = &DEFAULT_DUKE3DW_KEY_DEFINITIONS;
		}
		else if(isDuke3d_w32) {
			keyDefinitions = &DEFAULT_DUKE3D_W32_KEY_DEFINITIONS;
		}
		else if(isRDuke) {
			keyDefinitions = &DEFAULT_RDUKE_KEY_DEFINITIONS;
		}

		std::shared_ptr<Section> keyDefinitionsSection(std::make_shared<Section>(KEY_DEFINITIONS_SECTION_NAME, hasComments ? " \n " : "", hasComments ? " \n " : ""));
		gameConfiguration->addSection(keyDefinitionsSection);

		for(size_t i = 0; i < keyDefinitions->size(); i++) {
			const auto & keyDefinitionInfo = (*keyDefinitions)[i];

			keyDefinitionsSection->addMultiStringEntry(
				std::get<0>(keyDefinitionInfo),
				(isBelgianChocolate || isXDuke || isDuke3dw) && Utilities::areStringsEqualIgnoreCase(std::get<0>(keyDefinitionInfo), QUICK_KICK_ENTRY_NAME) ? (isBelgianChocolate || isXDuke ? "C" : "Q") : (std::get<1>(keyDefinitionInfo).empty() ? unboundKeyValue : (hasNormalizedKeyPadPrefix && Utilities::startsWith(std::get<1>(keyDefinitionInfo), "KP") ? "Kp" + std::get<1>(keyDefinitionInfo).substr(2) : std::get<1>(keyDefinitionInfo))),
				std::get<2>(keyDefinitionInfo).empty() ? unboundKeyValue : (hasNormalizedKeyPadPrefix && Utilities::startsWith(std::get<2>(keyDefinitionInfo), "KP") ? "Kp" + std::get<2>(keyDefinitionInfo).substr(2) : std::get<2>(keyDefinitionInfo))
			);

			// weapon key definitions are inserted in the middle of the normal key definitions
			if(i == 18) {
				for(size_t i = 1; i <= 10; i++) {
					keyDefinitionsSection->addMultiStringEntry(fmt::format("{}{}", WEAPON_KEY_DEFINITION_ENTRY_NAME_PREFIX, i), fmt::format("{}", i % 10), unboundKeyValue);
				}
			}
		}

		if(Any(sectionFeatures.keyDefinitions & SectionFeatures::KeyDefinitions::HasShowMenuEntry)) {
			keyDefinitionsSection->addMultiStringEntry(JFDUKE3D_SHOW_MENU_ENTRY_NAME, unboundKeyValue, unboundKeyValue);
		}

		if(Any(sectionFeatures.keyDefinitions & SectionFeatures::KeyDefinitions::HasShowConsoleEntry)) {
			keyDefinitionsSection->addMultiStringEntry(SHOW_CONSOLE_ENTRY_NAME, showConsoleKeyA, showConsoleKeyB);
		}

		if(Any(sectionFeatures.keyDefinitions & SectionFeatures::KeyDefinitions::HasExtraEntries)) {
			keyDefinitionsSection->addMultiStringEntry(QUICK_LOAD_ENTRY_NAME, "F9", unboundKeyValue);
			keyDefinitionsSection->addMultiStringEntry(QUICK_SAVE_ENTRY_NAME, "F6", unboundKeyValue);
			keyDefinitionsSection->addMultiStringEntry(SAVE_MENU_ENTRY_NAME, "F2", unboundKeyValue);
			keyDefinitionsSection->addMultiStringEntry(LOAD_MENU_ENTRY_NAME, "F3", unboundKeyValue);
			keyDefinitionsSection->addMultiStringEntry(HELP_MENU_ENTRY_NAME, "F1", unboundKeyValue);
			keyDefinitionsSection->addMultiStringEntry(SOUND_MENU_ENTRY_NAME, "F4", unboundKeyValue);
			keyDefinitionsSection->addMultiStringEntry(NEXT_TRACK_ENTRY_NAME, "F5", unboundKeyValue);
			keyDefinitionsSection->addMultiStringEntry(VIEW_MODE_ENTRY_NAME, "F7", unboundKeyValue);
			keyDefinitionsSection->addMultiStringEntry(VIDEO_MENU_ENTRY_NAME, "F11", unboundKeyValue);
			keyDefinitionsSection->addMultiStringEntry(QUIT_GAME_ENTRY_NAME, "F10", unboundKeyValue);
			keyDefinitionsSection->addMultiStringEntry(GAME_MENU_ENTRY_NAME, "F8", unboundKeyValue);
		}

		if(Any(sectionFeatures.keyDefinitions & SectionFeatures::KeyDefinitions::HasHideWeaponEntry)) {
			keyDefinitionsSection->addMultiStringEntry(KEY_DEFINITIONS_HIDE_WEAPON_ENTRY_NAME, hideWeaponKey, unboundKeyValue);
		}

		if(Any(sectionFeatures.keyDefinitions & SectionFeatures::KeyDefinitions::HasAutoAimEntry)) {
			keyDefinitionsSection->addMultiStringEntry(KEY_DEFINITIONS_AUTO_AIM_ENTRY_NAME, "V", unboundKeyValue);
		}

		if(Any(sectionFeatures.keyDefinitions & SectionFeatures::KeyDefinitions::HasConsoleEntry)) {
			keyDefinitionsSection->addMultiStringEntry(CONSOLE_ENTRY_NAME, "`", unboundKeyValue);
		}
	}

	// create controls section
	if(Any(sectionFeatures.controls & SectionFeatures::Controls::Supported) && Any(sectionFeatures.controls & SectionFeatures::Controls::Populated)) {
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

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasUseMouseAndJoystickEntries)) {
			controlsSection->addIntegerEntry(CONTROLS_USE_JOYSTICK_ENTRY_NAME, useJoystick ? 1 : 0);
			controlsSection->addIntegerEntry(CONTROLS_USE_MOUSE_ENTRY_NAME, useMouse ? 1 : 0);
		}
		else if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasLegacyControllerEntries)) {
			controlsSection->addIntegerEntry(CONTROLLER_TYPE_ENTRY_NAME, controllerType);

			if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasJoystickPortEntry)) {
				controlsSection->addIntegerEntry("JoystickPort", 0);
			}
		}

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasMouseSensitivity)) {
			controlsSection->addIntegerEntry("MouseSensitivity", mouseSensitivity);
		}

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasMouseScale)) {
			controlsSection->addIntegerEntry(CONTROLS_MOUSE_SCALE_X_ENTRY_NAME, mouseScale);
			controlsSection->addIntegerEntry(CONTROLS_MOUSE_SCALE_Y_ENTRY_NAME, mouseScale);
		}

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasExternalFileNameEntry)) {
			controlsSection->addStringEntry("ExternalFilename", "EXTERNAL.EXE");
		}

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasRudderEntry)) {
			controlsSection->addIntegerEntry("EnableRudder", 0);
		}

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasMouseYLockEntry)) {
			controlsSection->addIntegerEntry(CONTROLS_MOUSE_Y_LOCK_ENTRY_NAME, 0);
		}

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasMouseAimingEntry)) {
			controlsSection->addIntegerEntry(CONTROLS_MOUSE_AIMING_ENTRY_NAME, 0);
		}

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasGameMouseAimingEntry)) {
			controlsSection->addIntegerEntry(GAME_MOUSE_AIMING_ENTRY_NAME, gameMouseAiming ? 1 : 0);
		}

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasAimingFlagEntry)) {
			controlsSection->addIntegerEntry(AIMING_FLAG_ENTRY_NAME, aimingFlag ? 1 : 0);
		}

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasMouseAimFlippedEntry)) {
			controlsSection->addIntegerEntry(MOUSE_AIMING_FLIPPED_ENTRY_NAME, mouseAimingFlipped ? 1 : 0);
		}

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasRunKeyBehaviourEntry)) {
			controlsSection->addIntegerEntry(CONTROLS_RUN_KEY_BEHAVIOUR_ENTRY_NAME, 0);
		}

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasAutoAimEntry)) {
			controlsSection->addIntegerEntry(CONTROLS_AUTO_AIM_ENTRY_NAME, autoAim);
		}

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasWeaponSwitchModeEntry)) {
			controlsSection->addIntegerEntry(CONTROLS_WEAPON_SWITCH_MODE_ENTRY_NAME, 3);
		}

		const std::vector<std::string> * mouseButtonActions = &DEFAULT_MOUSE_BUTTON_ACTIONS;
		const std::array<std::string, 3> * mouseButtonClickedActions = &DEFAULT_MOUSE_BUTTON_CLICKED_ACTIONS;

		if(isEDuke32) {
			mouseButtonActions = &DEFAULT_EDUKE32_MOUSE_BUTTON_ACTIONS;
			mouseButtonClickedActions = &NO_MOUSE_BUTTON_CLICKED_ACTIONS;
		}
		else if(isRedNukem) {
			mouseButtonActions = &DEFAULT_RED_NUKEM_MOUSE_BUTTON_ACTIONS;
		}
		else if(isNetDuke32) {
			mouseButtonActions = &DEFAULT_NETDUKE32_MOUSE_BUTTON_ACTIONS;
		}
		else if(isDuke3dw) {
			mouseButtonActions = &DEFAULT_DUKE3DW_MOUSE_BUTTON_ACTIONS;
			mouseButtonClickedActions = &NO_MOUSE_BUTTON_CLICKED_ACTIONS;
		}
		else if(isDuke3d_w32) {
			mouseButtonActions = &DEFAULT_DUKE3D_W32_MOUSE_BUTTON_ACTIONS;
		}
		else if(isRDuke) {
			mouseButtonActions = &DEFAULT_RDUKE_MOUSE_BUTTON_ACTIONS;
		}

		for(size_t i = 0; i < numberOfMouseButtons; i++) {
			controlsSection->addStringEntry(fmt::format("{}{}{}", MOUSE_PREFIX, BUTTON_SUFFIX, i), i < mouseButtonActions->size() ? (*mouseButtonActions)[i] : "");

			if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasMouseButtonClickedEntries) && i < numberOfClickableMouseButtons) {
				controlsSection->addStringEntry(fmt::format("{}{}{}{}", MOUSE_PREFIX, BUTTON_SUFFIX, CLICKED_SUFFIX, i), i < mouseButtonClickedActions->size() ? (*mouseButtonClickedActions)[i] : "");
			}
		}

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasJoystickControls)) {
			const std::vector<std::pair<std::string, std::string>> * joystickButtonActionList = &DEFAULT_JOYSTICK_BUTTON_ACTIONS;

			if(isJFDuke3D) {
				joystickButtonActionList = &DEFAULT_JDFUKE3D_JOYSTICK_BUTTON_ACTIONS;
			}
			else if(isDuke3dw) {
				joystickButtonActionList = &DEFAULT_DUKE3DW_JOYSTICK_BUTTON_ACTIONS;
			}
			else if(isDuke3d_w32) {
				joystickButtonActionList = &DEFAULT_DUKE3D_W32_JOYSTICK_BUTTON_ACTIONS;
			}

			for(size_t i = 0; i < numberOfJoystickButtons; i++) {
				const std::pair<std::string, std::string> * joystickButtonActions = i < joystickButtonActionList->size() ? &(*joystickButtonActionList)[i] : nullptr;
				controlsSection->addStringEntry(fmt::format("{}{}{}", JOYSTICK_PREFIX, BUTTON_SUFFIX, i), joystickButtonActions != nullptr ? joystickButtonActions->first : "");

				if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasJoystickButtonClickedEntries)) {
					controlsSection->addStringEntry(fmt::format("{}{}{}{}", JOYSTICK_PREFIX, BUTTON_SUFFIX, CLICKED_SUFFIX, i), joystickButtonActions != nullptr ? joystickButtonActions->second : "");
				}
			}
		}

		for(size_t i = 0; i < 2; i++) {
			if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasMouseAnalogAxesEntries)) {
				controlsSection->addStringEntry(fmt::format("{}{}{}{}", MOUSE_PREFIX, ANALOG_SUFFIX, AXES_SUFFIX, i), isNetDuke32 ? DEFAULT_NETDUKE32_MOUSE_ANALOG_AXES_ACTIONS[i] : DEFAULT_MOUSE_ANALOG_AXES_ACTIONS[i]);
			}

			if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasMouseDigitalEntries)) {
				for(size_t j = 0; j < 2; j++) {
					std::string mouseDigitalAxesAction;

					if(i < DEFAULT_MOUSE_DIGITAL_AXES_ACTIONS.size()) {
						mouseDigitalAxesAction = j == 0 ? DEFAULT_MOUSE_DIGITAL_AXES_ACTIONS[i].first : DEFAULT_MOUSE_DIGITAL_AXES_ACTIONS[i].second;
					}

					controlsSection->addStringEntry(fmt::format("{}{}{}{}_{}", MOUSE_PREFIX, DIGITAL_SUFFIX, AXES_SUFFIX, i, j), mouseDigitalAxesAction);
				}
			}

			if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasMouseAnalogScaleEntries)) {
				controlsSection->addIntegerEntry(fmt::format("{}{}{}{}", MOUSE_PREFIX, ANALOG_SUFFIX, SCALE_SUFFIX, i), mouseAnalogScale);
			}
		}

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasRancidMeatMouseSensitivityEntries)) {
			for(uint8_t i = 0; i < 2; i++) {
				controlsSection->addIntegerEntry(fmt::format("{}{}_{}_{}", MOUSE_PREFIX, SENSITIVITY_SUFFIX, static_cast<char>('X' + i), RANCID_MEAT_SUFFIX), i == 0 ? rancidMeatXMouseSensitivity : rancidMeatYMouseSensitivity);
			}
		}

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasJoystickControls)) {
			for(size_t i = 0; i < numberOfJoysticks; i++) {
				const auto * joystickAnalogAxes = &DEFAULT_JOYSTICK_ANALOG_AXES_ACTIONS;
				const auto * joystickDigitalAxes = &DEFAULT_JOYSTICK_DIGITAL_AXES_ACTIONS;

				if(isDuke3d_w32) {
					joystickAnalogAxes = &DEFAULT_DUKE3D_W32_JOYSTICK_ANALOG_AXES_ACTIONS;
					joystickDigitalAxes = &DEFAULT_DUKE3D_W32_JOYSTICK_DIGITAL_AXES_ACTIONS;
				}

				controlsSection->addStringEntry(fmt::format("{}{}{}{}", JOYSTICK_PREFIX, ANALOG_SUFFIX, AXES_SUFFIX, i), i < joystickAnalogAxes->size() ? (*joystickAnalogAxes)[i] : "");

				for(uint8_t j = 0; j < 2; j++) {
					std::string joystickDigitalAxesAction;

					if(i < joystickDigitalAxes->size()) {
						joystickDigitalAxesAction = j == 0 ? (*joystickDigitalAxes)[i].first : (*joystickDigitalAxes)[i].second;
					}

					controlsSection->addStringEntry(fmt::format("{}{}{}{}_{}", JOYSTICK_PREFIX, DIGITAL_SUFFIX, AXES_SUFFIX, i, j), joystickDigitalAxesAction);
				}

				if(isDuke3d_w32) {
					controlsSection->addFloatEntry(fmt::format("{}{}{}{}", JOYSTICK_PREFIX, ANALOG_SUFFIX, SCALE_SUFFIX, i), i < 2 ? 0.167f : 0.25f);
				}
				else {
					controlsSection->addIntegerEntry(fmt::format("{}{}{}{}", JOYSTICK_PREFIX, ANALOG_SUFFIX, SCALE_SUFFIX, i), joystickAnalogScale);
				}

				if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasJoystickAnalogDeadZoneEntries)) {
					controlsSection->addIntegerEntry(fmt::format("{}{}{}{}", JOYSTICK_PREFIX, ANALOG_SUFFIX, isDuke3d_w32 ? ALTERNATE_DEADZONE_SUFFIX : DEADZONE_SUFFIX, i), joystickAnalogDeadZone);
				}

				if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasJoystickAnalogSaturateEntries)) {
					controlsSection->addIntegerEntry(fmt::format("{}{}{}{}", JOYSTICK_PREFIX, ANALOG_SUFFIX, SATURATE_SUFFIX, i), joystickAnalogSaturate);
				}
			}

			if(isDuke3d_w32) {
				controlsSection->addStringEntry(fmt::format("{}{}{}_{}", JOYSTICK_PREFIX, HAT_SUFFIX, 0, 0), NEXT_WEAPON_ENTRY_NAME);
				controlsSection->addStringEntry(fmt::format("{}{}{}_{}", JOYSTICK_PREFIX, HAT_SUFFIX, 0, 2), PREVIOUS_WEAPON_ENTRY_NAME);
			}
		}

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasGamepadControls)) {
			for(size_t i = 0; i < GAMEPAD_DIGITAL_AXES_ACTIONS.size(); i++) {
				for(uint8_t j = 0; j < 2; j++) {
					controlsSection->addStringEntry(fmt::format("{}{}{}{}_{}", GAMEPAD_PREFIX, DIGITAL_SUFFIX, AXES_SUFFIX, i, j), j == 0 ? GAMEPAD_DIGITAL_AXES_ACTIONS[i].first : GAMEPAD_DIGITAL_AXES_ACTIONS[i].second);
				}
			}
		}

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasControllerControls)) {
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

				if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasControllerAnalogSensitivityEntries)) {
					controlsSection->addIntegerEntry(fmt::format("{}{}{}{}", CONTROLLER_PREFIX, ANALOG_SUFFIX, SENSITIVITY_SUFFIX, i), DEFAULT_CONTROLLER_ANALOG_SENSITIVITY);
				}
				else if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasControllerAnalogScaleEntries)) {
					controlsSection->addIntegerEntry(fmt::format("{}{}{}{}", CONTROLLER_PREFIX, ANALOG_SUFFIX, SCALE_SUFFIX, i), 0);
				}

				controlsSection->addIntegerEntry(fmt::format("{}{}{}{}", CONTROLLER_PREFIX, ANALOG_SUFFIX, INVERT_SUFFIX, i), 0);
				controlsSection->addIntegerEntry(fmt::format("{}{}{}{}", CONTROLLER_PREFIX, ANALOG_SUFFIX, DEADZONE_SUFFIX, i), controllerAnalogDeadZone);
				controlsSection->addIntegerEntry(fmt::format("{}{}{}{}", CONTROLLER_PREFIX, ANALOG_SUFFIX, SATURATE_SUFFIX, i), controllerAnalogSaturate);
			}
		}
	}

	if(Any(sectionFeatures.lobbyFilter & SectionFeatures::LobbyFilter::Supported) && Any(sectionFeatures.lobbyFilter & SectionFeatures::LobbyFilter::Populated)) {
		std::shared_ptr<Section> lobbyFilterSection(std::make_shared<Section>(LOBBY_FILTER_SECTION_NAME));
		gameConfiguration->addSection(lobbyFilterSection);

		lobbyFilterSection->addIntegerEntry(LOBBY_FILTER_GAME_MODE_ENTRY_NAME, 3);
		lobbyFilterSection->addIntegerEntry(LOBBY_FILTER_SHOW_FULL_LOBBIES_ENTRY_NAME, 1);
		lobbyFilterSection->addIntegerEntry(LOBBY_FILTER_MAPS_ENTRY_NAME, 0);
	}

	if(Any(sectionFeatures.multiplayer & SectionFeatures::Multiplayer::Supported) && Any(sectionFeatures.multiplayer & SectionFeatures::Multiplayer::Populated)) {
		std::shared_ptr<Section> multiplayerSection(std::make_shared<Section>(MULTIPLAYER_SECTION_NAME));
		gameConfiguration->addSection(multiplayerSection);

		multiplayerSection->addIntegerEntry(MULTIPLAYER_SHOW_INFO_ENTRY_NAME, 1);
	}

	// create communication setup section
	std::shared_ptr<Section> communicationSetupSection(std::make_shared<Section>("Comm Setup", hasComments ? " \n " : "", hasComments ? " \n " : ""));
	gameConfiguration->addSection(communicationSetupSection);

	if(Any(sectionFeatures.communication & SectionFeatures::Communication::HasDialupNetworking)) {
		communicationSetupSection->addIntegerEntry("ComPort", 2);

		if(Any(sectionFeatures.communication & SectionFeatures::Communication::HasEmptyIRQNumber)) {
			communicationSetupSection->addEmptyEntry(COMMUNICATION_IRQ_NUMBER_ENTRY_NAME);
		}
		else {
			communicationSetupSection->addIntegerEntry(COMMUNICATION_IRQ_NUMBER_ENTRY_NAME, irqNumber);
		}

		if(Any(sectionFeatures.communication & SectionFeatures::Communication::HasEmptyUARTAddress)) {
			communicationSetupSection->addEmptyEntry(COMMUNICATION_UART_ADDRESS_ENTRY_NAME);
		}
		else {
			communicationSetupSection->addIntegerEntry(COMMUNICATION_UART_ADDRESS_ENTRY_NAME, uartAddress);
		}

		communicationSetupSection->addIntegerEntry("PortSpeed", 9600);
		communicationSetupSection->addIntegerEntry("ToneDial", 1);

		if(Any(sectionFeatures.communication & SectionFeatures::Communication::HasEmptySocketNumber)) {
			communicationSetupSection->addEmptyEntry(COMMUNICATION_SOCKET_NUMBER_ENTRY_NAME);
		}
		else {
			communicationSetupSection->addIntegerEntry(COMMUNICATION_SOCKET_NUMBER_ENTRY_NAME, socketNumber);
		}

		communicationSetupSection->addIntegerEntry("NumberPlayers", 2);
		communicationSetupSection->addStringEntry("ModemName", "");
		communicationSetupSection->addStringEntry("InitString", "ATZ");
		communicationSetupSection->addStringEntry("HangupString", "ATH0=0");
		communicationSetupSection->addStringEntry("DialoutString", "");
	}

	communicationSetupSection->addStringEntry("PlayerName", playerName);

	if(Any(sectionFeatures.communication & SectionFeatures::Communication::HasRemoteRidiculeFileName)) {
		communicationSetupSection->addStringEntry("RTSName", "DUKE.RTS");
	}

	if(isAtomicEdition) {
		communicationSetupSection->addStringEntry("RTSPath", ".\\");
		communicationSetupSection->addStringEntry("UserPath", ".\\");
	}

	if(Any(sectionFeatures.communication & SectionFeatures::Communication::HasDialupNetworking)) {
		communicationSetupSection->addStringEntry("PhoneNumber", "");
		communicationSetupSection->addIntegerEntry("ConnectType", 0);
	}

	if(Any(sectionFeatures.communication & SectionFeatures::Communication::HasCombatMacros)) {
		for(size_t i = 0; i < DEFAULT_COMBAT_MACROS.size(); i++) {
			communicationSetupSection->addStringEntry(fmt::format("{}{}", COMBAT_MACRO_ENTRY_NAME_PREFIX, i), isEDuke32 || isRedNukem ? DEFAULT_EDUKE32_COMBAT_MACROS[i] : DEFAULT_COMBAT_MACROS[i]);
		}
	}

	if(Any(sectionFeatures.communication & SectionFeatures::Communication::HasDialupNetworking)) {
		for(size_t i = 0; i < numberOfPhoneNumbers; i++) {
			communicationSetupSection->addStringEntry(fmt::format("{}{}", PHONE_NAME_ENTRY_NAME_PREFIX, i), "");
			communicationSetupSection->addStringEntry(fmt::format("{}{}", PHONE_NUMBER_ENTRY_NAME_PREFIX, i), "");
		}
	}

	if(Any(sectionFeatures.communication & SectionFeatures::Communication::HasPlayerColourEntry)) {
		communicationSetupSection->addIntegerEntry(PLAYER_COLOR_ENTRY_NAME, playerColor);
	}

	if(Any(sectionFeatures.communication & SectionFeatures::Communication::HasIPAddressEntries)) {
		for(uint8_t i = 0; i < numberOfIPAddressEntries; i++) {
			communicationSetupSection->addStringEntry(fmt::format("{}{}", IP_ADDRESS_ENTRY_NAME_PREFIX, i + 1), i == 0 ? "127.0.0.0" : "");
		}
	}

	if(Any(sectionFeatures.communication & SectionFeatures::Communication::HasBotNameEntries)) {
		for(uint8_t i = 0; i < numberOfBotNameEntries; i++) {
			communicationSetupSection->addStringEntry(fmt::format("{}{}", BOT_NAME_ENTRY_NAME_PREFIX, i + 1), fmt::format("{}{}", BOT_NAME_NAME_PREFIX, i + 1));
		}
	}

	return gameConfiguration;
}

bool GameConfiguration::updateForDOSBox() {
	std::optional<GameVersionType> optionalGameVersionType(determineGameVersionType());

	if(!optionalGameVersionType.has_value()) {
		return false;
	}

	GameVersionType gameVersionType = optionalGameVersionType.value();

	SectionFeatures sectionFeatures(getSectionFeaturesForGameVersion(gameVersionType));

	uint16_t mixRate = 44000;

	if(gameVersionType == GameVersionType::RegularVersion || gameVersionType == GameVersionType::Beta) {
		mixRate = 22000;
	}

	std::shared_ptr<Section> screenSetupSection(getSectionWithName(SCREEN_SETUP_SECTION_NAME));

	if(screenSetupSection != nullptr) {
		if(!screenSetupSection->setEntryIntegerValue(SCREEN_MODE_ENTRY_NAME, 1, true)) { return false; }
		if(!screenSetupSection->setEntryIntegerValue(SCREEN_WIDTH_ENTRY_NAME, 800, true)) { return false; }
		if(!screenSetupSection->setEntryIntegerValue(SCREEN_HEIGHT_ENTRY_NAME, 600, true)) { return false; }
		if(!screenSetupSection->setEntryStringValue(SCREEN_PASSWORD_ENTRY_NAME, "", true)) { return false; }
		if(!screenSetupSection->setEntryStringValue(SCREEN_ENVIRONMENT_ENTRY_NAME, "", true)) { return false; }
		if(!screenSetupSection->setEntryIntegerValue(SCREEN_DETAIL_ENTRY_NAME, 1, true)) { return false; }
		if(!screenSetupSection->setEntryIntegerValue(SCREEN_TILT_ENTRY_NAME, 1, true)) { return false; }
		if(!screenSetupSection->setEntryIntegerValue(SCREEN_MESSAGES_ENTRY_NAME, 1, true)) { return false; }
		if(!screenSetupSection->setEntryIntegerValue(SCREEN_OUT_ENTRY_NAME, 0, true)) { return false; }
		if(Any(sectionFeatures.screen & SectionFeatures::Screen::HasLastInputOutputSlotEntry)) {
			if(!screenSetupSection->setEntryIntegerValue("LastIOSlot", 0, true)) { return false; }
		}
		if(!screenSetupSection->setEntryIntegerValue(SCREEN_SIZE_ENTRY_NAME, 4, true)) { return false; }
		if(!screenSetupSection->setEntryIntegerValue(SCREEN_GAMMA_ENTRY_NAME, 0, true)) { return false; }
	}

	std::shared_ptr<Section> soundSetupSection(getSectionWithName(SOUND_SETUP_SECTION_NAME));

	if(soundSetupSection != nullptr) {
		if(!soundSetupSection->setEntryIntegerValue(SOUND_FX_DEVICE_ENTRY_NAME, 0, true)) { return false; }
		if(!soundSetupSection->setEntryIntegerValue(SOUND_MUSIC_DEVICE_ENTRY_NAME, 0, true)) { return false; }
		if(!soundSetupSection->setEntryIntegerValue(SOUND_NUM_BITS_ENTRY_NAME, 16, true)) { return false; }
		if(!soundSetupSection->setEntryIntegerValue(SOUND_MIX_RATE_ENTRY_NAME, mixRate, true)) { return false; }
	}

	return true;
}

bool GameConfiguration::updateWithBetterSettings() {
	std::optional<GameVersionType> optionalGameVersionType(determineGameVersionType());

	if(!optionalGameVersionType.has_value()) {
		return false;
	}

	GameVersionType gameVersionType = optionalGameVersionType.value();

	SectionFeatures sectionFeatures(getSectionFeaturesForGameVersion(gameVersionType));

	std::shared_ptr<Section> miscSection(getSectionWithName(MISC_SECTION_NAME));

	if(miscSection == nullptr) {
		miscSection = std::shared_ptr<Section>(new Section(MISC_SECTION_NAME));
		addSection(miscSection);
	}

	if(Any(sectionFeatures.misc & SectionFeatures::Misc::DoesSupportRunModeToggle)) {
		if(!miscSection->setEntryIntegerValue(MISC_RUN_MODE_ENTRY_NAME, 1, true)) { return false; }

	}

	if(Any(sectionFeatures.misc & SectionFeatures::Misc::DoesSupportCrosshairToggle)) {
		if(!miscSection->setEntryIntegerValue(MISC_CROSSHAIRS_ENTRY_NAME, 1, true)) { return false; }
	}

	std::shared_ptr<Section> screenSetupSection(getSectionWithName(SCREEN_SETUP_SECTION_NAME));

	if(screenSetupSection != nullptr) {
		if(!screenSetupSection->setEntryIntegerValue(SCREEN_SHADOWS_ENTRY_NAME, 1, true)) { return false; }
	}

	std::shared_ptr<Section> soundSetupSection(getSectionWithName(SOUND_SETUP_SECTION_NAME));

	if(soundSetupSection != nullptr) {
		if(!soundSetupSection->setEntryIntegerValue(SOUND_SOUND_TOGGLE_ENTRY_NAME, 1, true)) { return false; }
		if(!soundSetupSection->setEntryIntegerValue(SOUND_VOICE_TOGGLE_ENTRY_NAME, 1, true)) { return false; }
		if(!soundSetupSection->setEntryIntegerValue(SOUND_AMBIENCE_TOGGLE_ENTRY_NAME, 1, true)) { return false; }
		if(!soundSetupSection->setEntryIntegerValue(SOUND_MUSIC_TOGGLE_ENTRY_NAME, 1, true)) { return false; }
	}

	return true;
}

bool GameConfiguration::updateWithBetterControls() {
	std::optional<GameVersionType> optionalGameVersionType(determineGameVersionType());

	if(!optionalGameVersionType.has_value()) {
		return false;
	}

	GameVersionType gameVersionType = optionalGameVersionType.value();

	SectionFeatures sectionFeatures(getSectionFeaturesForGameVersion(gameVersionType));

	std::shared_ptr<Section> keyDefinitionsSection(getSectionWithName(KEY_DEFINITIONS_SECTION_NAME));

	if(keyDefinitionsSection != nullptr) {
		if(!keyDefinitionsSection->setEntryMultiStringValue(MOVE_FORWARD_ENTRY_NAME, "W", 0, true, true)) { return false; }
		if(!keyDefinitionsSection->setEntryMultiStringValue(MOVE_BACKWARD_ENTRY_NAME, "S", 0, true, true)) { return false; }
		if(!keyDefinitionsSection->setEntryMultiStringValue(FIRE_ENTRY_NAME, "", 0, true, true)) { return false; }
		if(!keyDefinitionsSection->setEntryMultiStringValue(OPEN_ENTRY_NAME, "E", "F", true)) { return false; }
		if(!keyDefinitionsSection->setEntryMultiStringValue(JUMP_ENTRY_NAME, "Space", 0, true, true)) { return false; }
		if(!keyDefinitionsSection->setEntryMultiStringValue(CROUCH_ENTRY_NAME, "LCtrl", 1, true, true)) { return false; }
		if(!keyDefinitionsSection->setEntryMultiStringValue(STRAFE_LEFT_ENTRY_NAME, "A", 1, true, true)) { return false; }
		if(!keyDefinitionsSection->setEntryMultiStringValue(STRAFE_RIGHT_ENTRY_NAME, "D", 1, true, true)) { return false; }

		std::shared_ptr<Entry> hideWeaponEntry(keyDefinitionsSection->getEntryWithName(KEY_DEFINITIONS_HIDE_WEAPON_ENTRY_NAME));

		if(hideWeaponEntry != nullptr) {
			if(!hideWeaponEntry->setMultiStringValue("", 0, true)) { return false; }
		}
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
	else if(keyDefinitionsSection != nullptr) {
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
	else if(keyDefinitionsSection != nullptr) {
		if(!keyDefinitionsSection->setEntryMultiStringValue(MAP_FOLLOW_MODE_ENTRY_NAME, "", 0, true, true)) { return false; }
	}

	std::shared_ptr<Section> controlsSection(getSectionWithName(CONTROLS_SECTION_NAME));

	if(controlsSection != nullptr) {
		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasMouseAimFlippedEntry)) {
			if(!controlsSection->setEntryIntegerValue(MOUSE_AIMING_FLIPPED_ENTRY_NAME, 1, true)) { return false; }
		}

		std::string firstMouseButtonEntryName(fmt::format("{}{}{}", MOUSE_PREFIX, BUTTON_SUFFIX, 1));

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::DoesSupportQuickKick)) {
			if(!controlsSection->setEntryStringValue(firstMouseButtonEntryName, QUICK_KICK_ENTRY_NAME, true)) { return false; }
		}
		else {
			if(!controlsSection->setEntryStringValue(firstMouseButtonEntryName, JETPACK_ENTRY_NAME, true)) { return false; }
		}

		if(!controlsSection->setEntryStringValue(fmt::format("{}{}{}{}", MOUSE_PREFIX, BUTTON_SUFFIX, CLICKED_SUFFIX, 1), "", true)) { return false; }
		if(!controlsSection->setEntryStringValue(fmt::format("{}{}{}", MOUSE_PREFIX, BUTTON_SUFFIX, 2), OPEN_ENTRY_NAME, true)) { return false; }

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasGameMouseAimingEntry)) {
			if(!controlsSection->setEntryIntegerValue(GAME_MOUSE_AIMING_ENTRY_NAME, 1, true)) { return false; }
		}

		if(Any(sectionFeatures.controls & SectionFeatures::Controls::HasAimingFlagEntry)) {
			if(!controlsSection->setEntryIntegerValue(AIMING_FLAG_ENTRY_NAME, 1, true)) { return false; }
		}
	}

	return true;
}
