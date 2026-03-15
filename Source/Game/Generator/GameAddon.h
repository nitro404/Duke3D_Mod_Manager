#ifndef _GAME_ADDON_H_
#define _GAME_ADDON_H_

enum class GameAddon : uint8_t {
	DukeCaribbeanLifesABeach,
	DukeItOutInDC,
	NuclearWinter
};

extern std::string getGameAddonID(GameAddon gameAddon);
extern std::string getGameAddonName(GameAddon gameAddon);

#endif // _GAME_ADDON_H_
