include_guard()

set(MAIN_SOURCE_FILES
	DOSBox/DOSBoxManager.h
	DOSBox/DOSBoxManager.cpp
	DOSBox/DOSBoxVersion.h
	DOSBox/DOSBoxVersion.cpp
	DOSBox/DOSBoxVersionCollection.h
	DOSBox/DOSBoxVersionCollection.cpp
	DOSBox/Configuration/CommentCollection.h
	DOSBox/Configuration/CommentCollection.cpp
	DOSBox/Configuration/DOSBoxConfiguration.h
	DOSBox/Configuration/DOSBoxConfiguration.cpp
	DOSBox/Configuration/DOSBoxConfigurationEntry.cpp
	DOSBox/Configuration/DOSBoxConfigurationSection.cpp
	DOSBox/Download/DOSBoxDownload.h
	DOSBox/Download/DOSBoxDownload.cpp
	DOSBox/Download/DOSBoxDownloadCollection.h
	DOSBox/Download/DOSBoxDownloadCollection.cpp
	DOSBox/Download/DOSBoxDownloadFile.h
	DOSBox/Download/DOSBoxDownloadFile.cpp
	DOSBox/Download/DOSBoxDownloadVersion.h
	DOSBox/Download/DOSBoxDownloadVersion.cpp
	Download/CachedFile.h
	Download/CachedFile.cpp
	Download/CachedPackageFile.h
	Download/CachedPackageFile.cpp
	Download/DownloadCache.h
	Download/DownloadCache.cpp
	Download/DownloadManager.h
	Download/DownloadManager.cpp
	Environment.h
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
	Game/Configuration/GameConfiguration.h
	Game/Configuration/GameConfiguration.cpp
	Game/Configuration/GameConfigurationEntry.cpp
	Game/Configuration/GameConfigurationGenerator.cpp
	Game/Configuration/GameConfigurationSection.cpp
	Game/Download/GameDownload.h
	Game/Download/GameDownload.cpp
	Game/Download/GameDownloadCollection.h
	Game/Download/GameDownloadCollection.cpp
	Game/Download/GameDownloadFile.h
	Game/Download/GameDownloadFile.cpp
	Game/Download/GameDownloadVersion.h
	Game/Download/GameDownloadVersion.cpp
	Game/File/GameFile.h
	Game/File/GameFile.cpp
	Game/File/GameFileFactoryRegistry.h
	Game/File/GameFileFactoryRegistry.cpp
	Game/File/Animation/Animation.h
	Game/File/Animation/Animation.cpp
	Game/File/Animation/ANM/AnimationANM.h
	Game/File/Animation/ANM/AnimationANM.cpp
	Game/File/Animation/ANM/ColourCycleInfo.cpp
	Game/File/Animation/ANM/LargePageDescriptor.cpp
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
	Game/File/Group/GRP/GroupGRP.h
	Game/File/Group/GRP/GroupGRP.cpp
	Game/File/Group/SSI/GroupSSI.h
	Game/File/Group/SSI/GroupSSI.cpp
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
	Game/File/MIDI/MIDI.h
	Game/File/MIDI/MIDI.cpp
	Game/File/MIDI/MIDIByteBufferReadStream.h
	Game/File/MIDI/MIDIByteBufferReadStream.cpp
	Game/File/MIDI/MIDIByteBufferWriteStream.h
	Game/File/MIDI/MIDIByteBufferWriteStream.cpp
	Game/File/Palette/ColourTable.h
	Game/File/Palette/ColourTable.cpp
	Game/File/Palette/Palette.h
	Game/File/Palette/Palette.cpp
	Game/File/Palette/ACT/PaletteACT.h
	Game/File/Palette/ACT/PaletteACT.cpp
	Game/File/Palette/DAT/PaletteDAT.h
	Game/File/Palette/DAT/PaletteDAT.cpp
	Game/File/Palette/DAT/ShadeTable.cpp
	Game/File/Palette/DAT/SwapTable.cpp
	Game/File/Palette/DAT/TranslucencyTable.cpp
	Game/File/Palette/GPL/PaletteGPL.h
	Game/File/Palette/GPL/PaletteGPL.cpp
	Game/File/Palette/JASC/PaletteJASC.h
	Game/File/Palette/JASC/PaletteJASC.cpp
	Game/File/Palette/PAL/PalettePAL.h
	Game/File/Palette/PAL/PalettePAL.cpp
	Game/File/Sound/Sound.h
	Game/File/Sound/Sound.cpp
	Game/File/Sound/VOC/SoundVOC.h
	Game/File/Sound/VOC/SoundVOC.cpp
	Game/File/Sound/WAV/SoundWAV.h
	Game/File/Sound/WAV/SoundWAV.cpp
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
	Mod/ModDependency.h
	Mod/ModDependency.cpp
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
	GUI/Console/ConsolePanel.h
	GUI/Console/ConsolePanel.cpp
	GUI/Console/Logging/CustomLogTextControl.h
	GUI/Console/Logging/CustomLogTextControl.cpp
	GUI/Console/Logging/LogSinkWX.h
	GUI/Console/Logging/LogSinkWX.cpp
	GUI/Console/Logging/PreformattedLogFormatter.h
	GUI/Console/Logging/PreformattedLogFormatter.cpp
	GUI/DOSBox/DOSBoxManagerPanel.h
	GUI/DOSBox/DOSBoxManagerPanel.cpp
	GUI/DOSBox/DOSBoxSettingsPanel.h
	GUI/DOSBox/DOSBoxSettingsPanel.cpp
	GUI/DOSBox/DOSBoxVersionPanel.h
	GUI/DOSBox/DOSBoxVersionPanel.cpp
	GUI/DOSBox/Configuration/DOSBoxConfigurationPanel.h
	GUI/DOSBox/Configuration/DOSBoxConfigurationPanel.cpp
	GUI/Game/GameManagerPanel.h
	GUI/Game/GameManagerPanel.cpp
	GUI/Game/GameVersionPanel.h
	GUI/Game/GameVersionPanel.cpp
	GUI/Group/GroupEditorPanel.h
	GUI/Group/GroupEditorPanel.cpp
	GUI/Group/GroupPanel.h
	GUI/Group/GroupPanel.cpp
	GUI/Group/SSI/SunstormInteractiveMetadataPanel.h
	GUI/Group/SSI/SunstormInteractiveMetadataPanel.cpp
	GUI/Mod/ModBrowserPanel.h
	GUI/Mod/ModBrowserPanel.cpp
	GUI/Mod/Info/ModDependenciesPanel.h
	GUI/Mod/Info/ModDependenciesPanel.cpp
	GUI/Mod/Info/ModDependencyPanel.h
	GUI/Mod/Info/ModDependencyPanel.cpp
	GUI/Mod/Info/ModDownloadPanel.h
	GUI/Mod/Info/ModDownloadPanel.cpp
	GUI/Mod/Info/ModDownloadsPanel.h
	GUI/Mod/Info/ModDownloadsPanel.cpp
	GUI/Mod/Info/ModInfoPanel.h
	GUI/Mod/Info/ModInfoPanel.cpp
	GUI/Mod/Info/ModTeamMemberPanel.h
	GUI/Mod/Info/ModTeamMemberPanel.cpp
	GUI/Mod/Info/ModTeamMembersPanel.h
	GUI/Mod/Info/ModTeamMembersPanel.cpp
	GUI/Mod/Info/RelatedModsPanel.h
	GUI/Mod/Info/RelatedModsPanel.cpp
	GUI/Mod/Info/SimilarModsPanel.h
	GUI/Mod/Info/SimilarModsPanel.cpp
	GUI/Releases/ReleaseNotesPanel.h
	GUI/Releases/ReleaseNotesPanel.cpp
	GUI/Settings/SettingsManagerPanel.h
	GUI/Settings/SettingsManagerPanel.cpp
	GUI/MetadataPanel.h
	GUI/MetadataPanel.cpp
	GUI/ModManagerApplication.h
	GUI/ModManagerApplication.cpp
	GUI/ModManagerFrame.h
	GUI/ModManagerFrame.cpp
	GUI/ProcessRunningDialog.h
	GUI/ProcessRunningDialog.cpp
	GUI/SettingPanel.h
	GUI/SettingPanel.cpp
	GUI/WXUtilities.h
	GUI/WXUtilities.cpp
)

list(APPEND SOURCE_FILES ${MAIN_SOURCE_FILES} ${MAIN_SOURCE_FILES_${PLATFORM_UPPER}})

list(APPEND SOURCE_FILES ${GUI_SOURCE_FILES})

list(TRANSFORM SOURCE_FILES PREPEND "${_SOURCE_DIRECTORY}/")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/${_SOURCE_DIRECTORY}" PREFIX "Source Files" FILES ${SOURCE_FILES})
