#ifndef _MOD_BROWSER_PANEL_H_
#define _MOD_BROWSER_PANEL_H_

#include "Game/GameType.h"
#include "Mod/OrganizedModCollection.h"

#include <Signal/SignalConnectionGroup.h>

#include <boost/signals2.hpp>
#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/hyperlink.h>
#include <wx/progdlg.h>
#include <wx/srchctrl.h>

#include <future>

class DOSBoxVersion;
class DOSBoxVersionCollection;
class GameProcessTerminatedEvent;
class ProcessRunningDialog;
class GameVersion;
class GameVersionCollection;
class LaunchFailedEvent;
class Mod;
class ModAuthorInformation;
class ModGameVersion;
class ModInfoPanel;
class ModInstallProgressEvent;
class ModInstallDoneEvent;
class ModManager;
class ModMatch;

class ModBrowserPanel final : public wxPanel {
public:
	ModBrowserPanel(std::shared_ptr<ModManager> modManager, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~ModBrowserPanel();

	void update();
	void updateModListFilterType();
	void updateModListSortOptions();
	void updateModListSortType();
	void updateModListSortDirection();
	void updateModList();
	void updateModVersionList();
	void updateModVersionTypeList();
	void updateModGameVersionList();
	void updateModSelection();
	void updateModInfo();
	void updatePreferredDOSBoxVersion();
	void updatePreferredDOSBoxVersionList();
	void updateModGameType();
	void updatePreferredGameVersion();
	void updatePreferredGameVersionList();
	void updateDOSBoxServerSettings();
	void updateDOSBoxServerIPAddress();
	void updateDOSBoxServerPort();
	void updateUninstallButton();

	void clear();
	void clearSearch();
	void clearSearchResults();


	bool installStandAloneMod(std::shared_ptr<ModGameVersion> standAloneModGameVersion);
	bool launchGame();

private:
	void onModSearchTextChanged(wxCommandEvent & event);
	void onModSearchCancelled(wxCommandEvent & event);
	void onSelectRandomModButtonPressed(wxCommandEvent & event);
	void onModListFilterTypeSelected(wxCommandEvent & event);
	void onModListSortTypeSelected(wxCommandEvent & event);
	void onModListSortDirectionSelected(wxCommandEvent & event);
	void onModSelected(wxCommandEvent & event);
	void onModVersionSelected(wxCommandEvent & event);
	void onModVersionTypeSelected(wxCommandEvent & event);
	void onModGameVersionSelected(wxCommandEvent & event);
	void onModListRightClicked(wxMouseEvent & event);
	void onModVersionListRightClicked(wxMouseEvent & event);
	void onModVersionTypeListRightClicked(wxMouseEvent & event);
	void onModPopupMenuItemPressed(wxCommandEvent & event);
	void onModVersionPopupMenuItemPressed(wxCommandEvent & event);
	void onModVersionTypePopupMenuItemPressed(wxCommandEvent & event);
	void onPreferredDOSBoxVersionSelected(wxCommandEvent & event);
	void onPreferredGameVersionSelected(wxCommandEvent & event);
	void onIPAddressTextChanged(wxCommandEvent & event);
	void onPortTextChanged(wxCommandEvent & event);
	void onModGameTypeSelected(wxCommandEvent & event);
	void onClearButtonPressed(wxCommandEvent & event);
	void onUninstallButtonPressed(wxCommandEvent & event);
	void onLaunchButtonPressed(wxCommandEvent & event);
	void onLaunched();
	void onLaunchStatus(const std::string & statusMessage);
	void onLaunchError(const std::string & errorMessage);
	void onGameProcessTerminated(uint64_t nativeExitCode, bool forceTerminated);
	void onLaunchFailed(LaunchFailedEvent & launchFailedEvent);
	void onGameProcessEnded(GameProcessTerminatedEvent & gameProcessTerminatedEvent);
	void onModInstallProgress(ModInstallProgressEvent & event);
	void onModInstallDone(ModInstallDoneEvent & event);
	void onModSelectionChanged(std::shared_ptr<Mod> mod, size_t modVersionIndex, size_t modVersionTypeIndex, size_t modGameVersionIndex);
	void onGameTypeChanged(GameType gameType);
	void onPreferredDOSBoxVersionChanged(std::shared_ptr<DOSBoxVersion> dosboxVersion);
	void onPreferredGameVersionChanged(std::shared_ptr<GameVersion> gameVersion);
	void onDOSBoxServerIPAddressChanged(std::string ipAddress);
	void onDOSBoxLocalServerPortChanged(uint16_t port);
	void onDOSBoxRemoteServerPortChanged(uint16_t port);
	void onFilterTypeChanged(OrganizedModCollection::FilterType filterType);
	void onSortOptionsChanged(OrganizedModCollection::SortType sortType, OrganizedModCollection::SortDirection sortDirection);
	void onSelectedModChanged(std::shared_ptr<Mod> mod);
	void onSelectedFavouriteModChanged(std::shared_ptr<ModIdentifier> favouriteMod);
	void onSelectedGameVersionChanged(std::shared_ptr<GameVersion> gameVersion);
	void onSelectedTeamChanged(std::shared_ptr<ModAuthorInformation> team);
	void onSelectedAuthorChanged(std::shared_ptr<ModAuthorInformation> author);
	void onOrganizedModCollectionChanged(const std::vector<std::shared_ptr<Mod>> & organizedMods);
	void onOrganizedFavouriteModCollectionChanged(const std::vector<std::shared_ptr<ModIdentifier>> & organizedFavouriteMods);
	void onOrganizedModGameVersionCollectionChanged(const std::vector<std::shared_ptr<GameVersion>> & organizedGameVersions);
	void onOrganizedModTeamCollectionChanged(const std::vector<std::shared_ptr<ModAuthorInformation>> & organizedTeams);
	void onOrganizedModAuthorCollectionChanged(const std::vector<std::shared_ptr<ModAuthorInformation>> & organizedAuthors);
	void onDOSBoxVersionCollectionSizeChanged(DOSBoxVersionCollection & dosboxVersionCollection);
	void onDOSBoxVersionCollectionItemModified(DOSBoxVersionCollection & dosboxVersionCollection, DOSBoxVersion & dosboxVersion);
	void onGameVersionCollectionSizeChanged(GameVersionCollection & gameVersionCollection);
	void onGameVersionCollectionItemModified(GameVersionCollection & gameVersionCollection, GameVersion & gameVersion);
	void onModSelectionRequested(const std::string & modID);
	void onModTeamSelectionRequested(const std::string & modTeamName);
	void onModTeamMemberSelectionRequested(const std::string & modTeamMemberName);
	void onModVersionTypeSelectionRequested(const std::string & modID, const std::string & modVersion, const std::string & modVersionType);

	std::shared_ptr<ModManager> m_modManager;
	std::shared_ptr<GameVersion> m_activeGameVersion;
	std::future<bool> m_runSelectedModFuture;
	boost::signals2::connection m_launchedConnection;
	boost::signals2::connection m_launchStatusConnection;
	boost::signals2::connection m_launchErrorConnection;
	boost::signals2::connection m_gameProcessTerminatedConnection;
	boost::signals2::connection m_modSelectionChangedConnection;
	boost::signals2::connection m_gameTypeChangedConnection;
	boost::signals2::connection m_preferredDOSBoxVersionChangedConnection;
	boost::signals2::connection m_preferredGameVersionChangedConnection;
	boost::signals2::connection m_dosboxServerIPAddressChangedConnection;
	boost::signals2::connection m_dosboxLocalServerPortChangedConnection;
	boost::signals2::connection m_dosboxRemoteServerPortChangedConnection;
	boost::signals2::connection m_filterTypeChangedConnection;
	boost::signals2::connection m_sortOptionsChangedConnection;
	boost::signals2::connection m_selectedModChangedConnection;
	boost::signals2::connection m_selectedFavouriteModChangedConnection;
	boost::signals2::connection m_selectedGameVersionChangedConnection;
	boost::signals2::connection m_selectedTeamChangedConnection;
	boost::signals2::connection m_selectedAuthorChangedConnection;
	boost::signals2::connection m_organizedModCollectionChangedConnection;
	boost::signals2::connection m_organizedFavouriteModCollectionChangedConnection;
	boost::signals2::connection m_organizedModGameVersionCollectionChangedConnection;
	boost::signals2::connection m_organizedModTeamCollectionChangedConnection;
	boost::signals2::connection m_organizedModAuthorCollectionChangedConnection;
	boost::signals2::connection m_dosboxVersionCollectionSizeChangedConnection;
	boost::signals2::connection m_dosboxVersionCollectionItemModifiedConnection;
	boost::signals2::connection m_gameVersionCollectionSizeChangedConnection;
	boost::signals2::connection m_gameVersionCollectionItemModifiedConnection;
	SignalConnectionGroup m_modInfoPanelSignalConnectionGroup;
	std::string m_searchQuery;
	std::vector<ModMatch> m_modMatches;
	std::vector<std::shared_ptr<ModIdentifier>> m_favouriteModMatches;
	std::vector<std::shared_ptr<GameVersion>> m_gameVersionMatches;
	std::vector<std::shared_ptr<ModAuthorInformation>> m_modAuthorMatches;
	wxSearchCtrl * m_modSearchTextField;
	wxButton * m_selectRandomModButton;
	wxComboBox * m_modListFilterTypeComboBox;
	wxComboBox * m_modListSortTypeComboBox;
	wxComboBox * m_modListSortDirectionComboBox;
	wxButton * m_clearButton;
	wxStaticText * m_modListLabel;
	wxListBox * m_modListBox;
	wxStaticText * m_modVersionListLabel;
	wxListBox * m_modVersionListBox;
	wxStaticText * m_modVersionTypeListLabel;
	wxListBox * m_modVersionTypeListBox;
	wxStaticText * m_modGameVersionListLabel;
	wxListBox * m_modGameVersionListBox;
	int m_modPopupMenuItemIndex;
	std::unique_ptr<wxMenu> m_modListPopupMenu;
	wxMenuItem * m_modListAddFavouriteMenuItem;
	wxMenuItem * m_modListRemoveFavouriteMenuItem;
	int m_modVersionPopupMenuItemIndex;
	std::unique_ptr<wxMenu> m_modVersionListPopupMenu;
	wxMenuItem * m_modVersionListAddFavouriteMenuItem;
	int m_modVersionTypePopupMenuItemIndex;
	std::unique_ptr<wxMenu> m_modVersionTypeListPopupMenu;
	wxMenuItem * m_modVersionTypeListAddFavouriteMenuItem;
	wxStaticBox * m_modInfoBox;
	ModInfoPanel * m_modInfoPanel;
	wxPanel * m_gameOptionsPanel;
	wxTextCtrl * m_ipAddressTextField;
	wxStaticText * m_portLabel;
	wxTextCtrl * m_portTextField;
	wxComboBox * m_preferredDOSBoxVersionComboBox;
	wxComboBox * m_modGameTypeComboBox;
	wxComboBox * m_preferredGameVersionComboBox;
	wxButton * m_uninstallButton;
	wxButton * m_launchButton;
	ProcessRunningDialog * m_gameRunningDialog;
	std::future<bool> m_installModFuture;
	std::atomic<bool> m_modInstallationCancelled;
	wxProgressDialog * m_installModProgressDialog;

	ModBrowserPanel(const ModBrowserPanel &) = delete;
	const ModBrowserPanel & operator = (const ModBrowserPanel &) = delete;
};

#endif // _MOD_BROWSER_PANEL_H_
