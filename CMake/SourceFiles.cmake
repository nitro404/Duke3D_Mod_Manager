include_guard()

set(MAIN_SOURCE_FILES
	Configuration/GameConfiguration.h
	Configuration/GameConfiguration.cpp
	Configuration/GameConfigurationEntry.cpp
	Configuration/GameConfigurationGenerator.cpp
	Configuration/GameConfigurationSection.cpp
	DOSBox/DOSBoxDownload.h
	DOSBox/DOSBoxDownload.cpp
	DOSBox/DOSBoxDownloadCollection.h
	DOSBox/DOSBoxDownloadCollection.cpp
	DOSBox/DOSBoxDownloadFile.h
	DOSBox/DOSBoxDownloadFile.cpp
	DOSBox/DOSBoxDownloadVersion.h
	DOSBox/DOSBoxDownloadVersion.cpp
	DOSBox/DOSBoxManager.h
	DOSBox/DOSBoxManager.cpp
	DOSBox/DOSBoxVersion.h
	DOSBox/DOSBoxVersion.cpp
	DOSBox/DOSBoxVersionCollection.h
	DOSBox/DOSBoxVersionCollection.cpp
	Download/CachedFile.h
	Download/CachedFile.cpp
	Download/CachedPackageFile.h
	Download/CachedPackageFile.cpp
	Download/DownloadCache.h
	Download/DownloadCache.cpp
	Download/DownloadManager.h
	Download/DownloadManager.cpp
	Environment.h
	Game/GameDownload.h
	Game/GameDownload.cpp
	Game/GameDownloadCollection.h
	Game/GameDownloadCollection.cpp
	Game/GameDownloadFile.h
	Game/GameDownloadFile.cpp
	Game/GameDownloadVersion.h
	Game/GameDownloadVersion.cpp
	Game/GameLocator.h
	Game/GameLocator.cpp
	Game/GameManager.h
	Game/GameManager.cpp
	Game/GameType.h
	Game/GameVersion.h
	Game/GameVersion.cpp
	Game/GameVersionCollection.h
	Game/GameVersionCollection.cpp
	Game/NoCDCracker.h
	Game/NoCDCracker.cpp
	Game/File/GameFile.h
	Game/File/GameFile.cpp
	Game/File/GameFileFactoryRegistry.h
	Game/File/GameFileFactoryRegistry.cpp
	Game/File/Art/Art.h
	Game/File/Art/Art.cpp
	Game/File/Art/Tile.h
	Game/File/Art/Tile.cpp
	Game/File/Art/TileNames.cpp
	Game/File/Group/Group.h
	Game/File/Group/Group.cpp
	Game/File/Group/GroupFile.h
	Game/File/Group/GroupFile.cpp
	Game/File/Group/GroupUtilities.h
	Game/File/Map/BuildConstants.h
	Game/File/Map/Map.h
	Game/File/Map/Map.cpp
	Game/File/Map/Partition.h
	Game/File/Map/Partition.cpp
	Game/File/Map/PlayerSpawn.h
	Game/File/Map/PlayerSpawn.cpp
	Game/File/Map/SectorItem.h
	Game/File/Map/SectorItem.cpp
	Game/File/Map/Sector.h
	Game/File/Map/Sector.cpp
	Game/File/Map/Sprite.h
	Game/File/Map/Sprite.cpp
	Game/File/Map/TaggedItem.h
	Game/File/Map/TaggedItem.cpp
	Game/File/Map/TexturedItem.h
	Game/File/Map/TexturedItem.cpp
	Game/File/Map/Velocity3D.h
	Game/File/Map/Velocity3D.cpp
	Game/File/Map/Wall.h
	Game/File/Map/Wall.cpp
	Game/File/Palette/Palette.h
	Game/File/Palette/Palette.cpp
	Game/File/Palette/ACT/PaletteACT.h
	Game/File/Palette/ACT/PaletteACT.cpp
	Game/File/Palette/PAL/PalettePAL.h
	Game/File/Palette/PAL/PalettePAL.cpp
	Game/File/Zip/Zip.h
	Game/File/Zip/Zip.cpp
	Manager/InstalledModInfo.h
	Manager/InstalledModInfo.cpp
	Manager/ModManager.h
	Manager/ModManager.cpp
	Manager/ModMatch.h
	Manager/ModMatch.cpp
	Manager/SettingsManager.h
	Manager/SettingsManager.cpp
	Mod/FavouriteModCollection.h
	Mod/FavouriteModCollection.cpp
	Mod/Location.h
	Mod/Location.cpp
	Mod/Mod.h
	Mod/Mod.cpp
	Mod/ModAuthorInformation.h
	Mod/ModAuthorInformation.cpp
	Mod/ModCollection.h
	Mod/ModCollection.cpp
	Mod/ModDownload.h
	Mod/ModDownload.cpp
	Mod/ModFile.h
	Mod/ModFile.cpp
	Mod/ModGameVersion.h
	Mod/ModGameVersion.cpp
	Mod/ModIdentifier.h
	Mod/ModIdentifier.cpp
	Mod/ModImage.h
	Mod/ModImage.cpp
	Mod/ModScreenshot.h
	Mod/ModScreenshot.cpp
	Mod/ModTeam.h
	Mod/ModTeam.cpp
	Mod/ModTeamMember.h
	Mod/ModTeamMember.cpp
	Mod/ModVersion.h
	Mod/ModVersion.cpp
	Mod/ModVersionType.h
	Mod/ModVersionType.cpp
	Mod/ModVideo.h
	Mod/ModVideo.cpp
	Mod/OrganizedModCollection.h
	Mod/OrganizedModCollection.cpp
	Mod/StandAloneMod.h
	Mod/StandAloneMod.cpp
	Mod/StandAloneModCollection.h
	Mod/StandAloneModCollection.cpp
	Project.h
)

set(MAIN_SOURCE_FILES_WINDOWS
	Game/Windows/GameLocatorWindows.cpp
	Manager/Windows/ModManagerWindows.cpp
)

set(GUI_SOURCE_FILES
	GUI/ConsolePanel.h
	GUI/ConsolePanel.cpp
	GUI/DOSBoxManagerPanel.h
	GUI/DOSBoxManagerPanel.cpp
	GUI/DOSBoxSettingsPanel.h
	GUI/DOSBoxSettingsPanel.cpp
	GUI/DOSBoxVersionPanel.h
	GUI/DOSBoxVersionPanel.cpp
	GUI/GameManagerPanel.h
	GUI/GameManagerPanel.cpp
	GUI/GameVersionPanel.h
	GUI/GameVersionPanel.cpp
	GUI/GroupEditorPanel.h
	GUI/GroupEditorPanel.cpp
	GUI/GroupPanel.h
	GUI/GroupPanel.cpp
	GUI/Logging/CustomLogTextControl.h
	GUI/Logging/CustomLogTextControl.cpp
	GUI/Logging/LogSinkWX.h
	GUI/Logging/LogSinkWX.cpp
	GUI/Logging/PreformattedLogFormatter.h
	GUI/Logging/PreformattedLogFormatter.cpp
	GUI/MetadataPanel.h
	GUI/MetadataPanel.cpp
	GUI/ModBrowserPanel.h
	GUI/ModBrowserPanel.cpp
	GUI/ModDownloadPanel.h
	GUI/ModDownloadPanel.cpp
	GUI/ModDownloadsPanel.h
	GUI/ModDownloadsPanel.cpp
	GUI/ModInfoPanel.h
	GUI/ModInfoPanel.cpp
	GUI/ModManagerApplication.h
	GUI/ModManagerApplication.cpp
	GUI/ModManagerFrame.h
	GUI/ModManagerFrame.cpp
	GUI/ModTeamMemberPanel.h
	GUI/ModTeamMemberPanel.cpp
	GUI/ModTeamMembersPanel.h
	GUI/ModTeamMembersPanel.cpp
	GUI/ProcessRunningDialog.h
	GUI/ProcessRunningDialog.cpp
	GUI/ReleaseNotesPanel.h
	GUI/ReleaseNotesPanel.cpp
	GUI/SettingPanel.h
	GUI/SettingPanel.cpp
	GUI/SettingsManagerPanel.h
	GUI/SettingsManagerPanel.cpp
	GUI/WXUtilities.h
	GUI/WXUtilities.cpp
)

list(APPEND SOURCE_FILES ${MAIN_SOURCE_FILES} ${MAIN_SOURCE_FILES_${PLATFORM_UPPER}})

list(APPEND SOURCE_FILES ${GUI_SOURCE_FILES})

list(TRANSFORM SOURCE_FILES PREPEND "${_SOURCE_DIRECTORY}/")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
