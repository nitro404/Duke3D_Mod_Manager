include_guard()

set(SOURCE_FILES
	Main.cpp
	Download/CachedFile.cpp
	Download/CachedPackageFile.cpp
	Download/DownloadCache.cpp
	Download/DownloadManager.cpp
	Game/GameLocator.cpp
	Game/GameVersion.cpp
	Game/GameVersionCollection.cpp
	Game/GameVersionCollectionBroadcaster.cpp
	Game/GameVersionCollectionListener.cpp
	Game/NoCDCracker.cpp
	Group/Group.cpp
	Group/GroupFile.cpp
	Manager/ModManager.cpp
	Manager/ModManagerCLI.cpp
	Manager/ModMatch.cpp
	Manager/SettingsManager.cpp
	Manager/Windows/ModManagerWindows.cpp
	Mod/ModIdentifier.cpp
	Mod/FavouriteModCollection.cpp
	Mod/Mod.cpp
	Mod/ModAuthorInformation.cpp
	Mod/ModCollection.cpp
	Mod/ModCollectionBroadcaster.cpp
	Mod/ModCollectionListener.cpp
	Mod/ModDownload.cpp
	Mod/ModFile.cpp
	Mod/ModGameVersion.cpp
	Mod/ModImage.cpp
	Mod/ModScreenshot.cpp
	Mod/ModTeam.cpp
	Mod/ModTeamMember.cpp
	Mod/ModVersion.cpp
	Mod/ModVersionType.cpp
	Mod/ModVideo.cpp
	Mod/OrganizedModCollection.cpp
)

set(SOURCE_FILES_WINDOWS
	Game/Windows/GameLocatorWindows.cpp
)

source_group(Resources                  REGULAR_EXPRESSION ".*\\.(rc)")
source_group(Resources\\Icon            REGULAR_EXPRESSION "Icon/.*\\.(rc)")
source_group(Resources\\Icon\\Windows   REGULAR_EXPRESSION "Icon/Windows.*\\.(rc)")
source_group(Source                     REGULAR_EXPRESSION ".*\\.(h|cpp)")
source_group(Source\\Download           REGULAR_EXPRESSION "Download/.*\\.(h|cpp)")
source_group(Source\\Game               REGULAR_EXPRESSION "Game/.*\\.(h|cpp)")
source_group(Source\\Game\\Windows      REGULAR_EXPRESSION "Game/Windows/.*\\.(h|cpp)")
source_group(Source\\Group              REGULAR_EXPRESSION "Group/.*\\.(h|cpp)")
source_group(Source\\Mod                REGULAR_EXPRESSION "Mod/.*\\.(h|cpp)")
source_group(Source\\Manager            REGULAR_EXPRESSION "Manager/.*\\.(h|cpp)")
source_group(Source\\Manager\\Windows   REGULAR_EXPRESSION "Manager/Windows/.*\\.(h|cpp)")
