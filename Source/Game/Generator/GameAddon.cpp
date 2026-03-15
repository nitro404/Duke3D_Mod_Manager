#include <GameAddon.h>

extern std::string getGameAddonID(GameAddon gameAddon) {
	switch(gameAddon) {
		case DukeCaribbeanLifesABeach:
			return "duke_caribbean";
		case DukeItOutInDC:
			return "duke_it_out_in_dc";
		case NuclearWinter:
			return "nuclear_winter";
	}

	return "";
}

extern std::string getGameAddonName(GameAddon gameAddon) {
	switch(gameAddon) {
		case DukeCaribbeanLifesABeach:
			return "Duke Caribbean: Life's a Beach";
		case DukeItOutInDC:
			return "Duke It Out in D.C.";
		case NuclearWinter:
			return "Nuclear Winter";
	}

	return "";
}
