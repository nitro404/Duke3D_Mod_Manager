#include "GameAddon.h"

extern std::string getGameAddonID(GameAddon gameAddon) {
	switch(gameAddon) {
		case GameAddon::DukeCaribbeanLifesABeach:
			return "duke_caribbean";
		case GameAddon::DukeItOutInDC:
			return "duke_it_out_in_dc";
		case GameAddon::NuclearWinter:
			return "nuclear_winter";
	}

	return "";
}

extern std::string getGameAddonName(GameAddon gameAddon) {
	switch(gameAddon) {
		case GameAddon::DukeCaribbeanLifesABeach:
			return "Duke Caribbean: Life's a Beach";
		case GameAddon::DukeItOutInDC:
			return "Duke It Out in D.C.";
		case GameAddon::NuclearWinter:
			return "Nuclear Winter";
	}

	return "";
}
