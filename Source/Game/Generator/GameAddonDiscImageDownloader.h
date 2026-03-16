#ifndef _GAME_ADDON_DISC_IMAGE_DOWNLOADER_H_
#define _GAME_ADDON_DISC_IMAGE_DOWNLOADER_H_

#include "GameAddon.h"

#include <Archive/Archive.h>
#include <Singleton/Singleton.h>

#include <cdio++/iso9660.hpp>

#include <memory>
#include <string>
#include <utility>

// TODO: move to download manager?

class GameAddonDiscImageDownloader final : public Singleton<GameAddonDiscImageDownloader> {
public:

private:
	GameAddonDiscImageDownloader();

	static std::string getGameAddonDownloadURL(GameAddon gameAddon);
	static std::unique_ptr<Archive> downloadGameAddonArchive(GameAddon gameAddon);
	static std::pair<std::unique_ptr<ISO9660::FS>, std::string> downloadGameAddonDiscImage(GameAddon gameAddon);

	GameAddonDiscImageDownloader(const GameAddonDiscImageDownloader &) = delete;
	const GameAddonDiscImageDownloader & operator = (const GameAddonDiscImageDownloader &) = delete;
};

#endif // _GAME_ADDON_DISC_IMAGE_DOWNLOADER_H_
