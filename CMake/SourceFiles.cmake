include_guard()

set(SOURCE_FILES
	Main.cpp
	Configuration/GameConfiguration.cpp
	Configuration/GameConfigurationEntry.cpp
	Configuration/GameConfigurationSection.cpp
	Download/CachedFile.cpp
	Download/CachedPackageFile.cpp
	Download/DownloadCache.cpp
	Download/DownloadManager.cpp
	Game/GameDownload.cpp
	Game/GameDownloadCollection.cpp
	Game/GameDownloadCollectionBroadcaster.cpp
	Game/GameDownloadCollectionListener.cpp
	Game/GameDownloadFile.cpp
	Game/GameDownloadVersion.cpp
	Game/GameLocator.cpp
	Game/GameManager.cpp
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
	Mod/FavouriteModCollection.cpp
	Mod/Mod.cpp
	Mod/ModAuthorInformation.cpp
	Mod/ModCollection.cpp
	Mod/ModCollectionBroadcaster.cpp
	Mod/ModCollectionListener.cpp
	Mod/ModDownload.cpp
	Mod/ModFile.cpp
	Mod/ModGameVersion.cpp
	Mod/ModIdentifier.cpp
	Mod/ModImage.cpp
	Mod/ModScreenshot.cpp
	Mod/ModTeam.cpp
	Mod/ModTeamMember.cpp
	Mod/ModVersion.cpp
	Mod/ModVersionType.cpp
	Mod/ModVideo.cpp
	Mod/OrganizedModCollection.cpp
)

set(HEADER_FILES
	Configuration/GameConfiguration.h
	Download/CachedFile.h
	Download/CachedPackageFile.h
	Download/DownloadCache.h
	Download/DownloadManager.h
	Environment.h
	Project.h
	Game/GameDownload.h
	Game/GameDownloadCollection.h
	Game/GameDownloadCollectionBroadcaster.h
	Game/GameDownloadCollectionListener.h
	Game/GameDownloadFile.h
	Game/GameDownloadVersion.h
	Game/GameLocator.h
	Game/GameManager.h
	Game/GameType.h
	Game/GameVersion.h
	Game/GameVersionCollection.h
	Game/GameVersionCollectionBroadcaster.h
	Game/GameVersionCollectionListener.h
	Game/NoCDCracker.h
	Group/Group.h
	Group/GroupFile.h
	Group/GroupUtilities.h
	Manager/ModManager.h
	Manager/ModMatch.h
	Manager/SettingsManager.h
	Mod/FavouriteModCollection.h
	Mod/Mod.h
	Mod/ModAuthorInformation.h
	Mod/ModCollection.h
	Mod/ModCollectionBroadcaster.h
	Mod/ModCollectionListener.h
	Mod/ModDownload.h
	Mod/ModFile.h
	Mod/ModGameVersion.h
	Mod/ModIdentifier.h
	Mod/ModImage.h
	Mod/ModScreenshot.h
	Mod/ModTeam.h
	Mod/ModTeamMember.h
	Mod/ModVersion.h
	Mod/ModVersionType.h
	Mod/ModVideo.h
	Mod/OrganizedModCollection.h
)

set(SOURCE_FILES_WINDOWS
	Game/Windows/GameLocatorWindows.cpp
)

list(APPEND SOURCE_FILES ${SOURCE_FILES_${PLATFORM_UPPER}})
list(APPEND HEADER_FILES ${HEADER_FILES_${PLATFORM_UPPER}})

list(TRANSFORM SOURCE_FILES PREPEND "${_SOURCE_DIRECTORY}/")
list(TRANSFORM HEADER_FILES PREPEND "${_SOURCE_DIRECTORY}/")

list(APPEND ALL_FILES ${HEADER_FILES} ${SOURCE_FILES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ALL_FILES})
