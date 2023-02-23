include_guard()

set(MAIN_SOURCE_FILES
	Configuration/GameConfiguration.cpp
	Configuration/GameConfigurationEntry.cpp
	Configuration/GameConfigurationGenerator.cpp
	Configuration/GameConfigurationSection.cpp
	DOSBox/DOSBoxDownload.cpp
	DOSBox/DOSBoxDownloadCollection.cpp
	DOSBox/DOSBoxDownloadFile.cpp
	DOSBox/DOSBoxDownloadVersion.cpp
	DOSBox/DOSBoxManager.cpp
	DOSBox/DOSBoxVersion.cpp
	DOSBox/DOSBoxVersionCollection.cpp
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
	Manager/InstalledModInfo.cpp
	Manager/ModManager.cpp
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

set(MAIN_HEADER_FILES
	Configuration/GameConfiguration.h
	DOSBox/DOSBoxDownload.h
	DOSBox/DOSBoxDownloadCollection.h
	DOSBox/DOSBoxDownloadFile.h
	DOSBox/DOSBoxDownloadVersion.h
	DOSBox/DOSBoxManager.h
	DOSBox/DOSBoxVersion.h
	DOSBox/DOSBoxVersionCollection.h
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
	Manager/InstalledModInfo.h
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

set(MAIN_SOURCE_FILES_WINDOWS
	Game/Windows/GameLocatorWindows.cpp
)

set(GUI_SOURCE_FILES
	GUI/ConsolePanel.cpp
	GUI/DOSBoxManagerPanel.cpp
	GUI/DOSBoxSettingsPanel.cpp
	GUI/DOSBoxVersionPanel.cpp
	GUI/GameManagerPanel.cpp
	GUI/GameVersionPanel.cpp
	GUI/GroupEditorPanel.cpp
	GUI/GroupPanel.cpp
	GUI/Logging/CustomLogTextControl.cpp
	GUI/Logging/LogSinkWX.cpp
	GUI/Logging/PreformattedLogFormatter.cpp
	GUI/ModBrowserPanel.cpp
	GUI/ModManagerApplication.cpp
	GUI/ModManagerFrame.cpp
	GUI/SettingPanel.cpp
	GUI/SettingsManagerPanel.cpp
	GUI/WXUtilities.cpp
)

set(GUI_HEADER_FILES
	GUI/ConsolePanel.h
	GUI/DOSBoxManagerPanel.h
	GUI/DOSBoxSettingsPanel.h
	GUI/DOSBoxVersionPanel.h
	GUI/GameManagerPanel.h
	GUI/GameVersionPanel.h
	GUI/GroupEditorPanel.h
	GUI/GroupPanel.h
	GUI/Logging/CustomLogTextControl.h
	GUI/Logging/LogSinkWX.h
	GUI/Logging/PreformattedLogFormatter.h
	GUI/ModBrowserPanel.h
	GUI/ModManagerApplication.h
	GUI/ModManagerFrame.h
	GUI/SettingPanel.h
	GUI/SettingsManagerPanel.h
	GUI/WXUtilities.h
)

set(CLI_SOURCE_FILES
	CLI/CLIMain.cpp
	CLI/ModManagerCLI.cpp
)

set(CLI_HEADER_FILES
	CLI/ModManagerCLI.h
)

list(APPEND SOURCE_FILES ${MAIN_SOURCE_FILES} ${MAIN_SOURCE_FILES_${PLATFORM_UPPER}})
list(APPEND HEADER_FILES ${MAIN_HEADER_FILES} ${MAIN_HEADER_FILES_${PLATFORM_UPPER}})

if(GUI_ENABLED)
	list(APPEND SOURCE_FILES ${GUI_SOURCE_FILES})
	list(APPEND HEADER_FILES ${GUI_HEADER_FILES})
else()
	list(APPEND SOURCE_FILES ${CLI_SOURCE_FILES})
	list(APPEND HEADER_FILES ${CLI_HEADER_FILES})
endif()

list(TRANSFORM SOURCE_FILES PREPEND "${_SOURCE_DIRECTORY}/")
list(TRANSFORM HEADER_FILES PREPEND "${_SOURCE_DIRECTORY}/")

list(APPEND ALL_FILES ${HEADER_FILES} ${SOURCE_FILES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ALL_FILES})
