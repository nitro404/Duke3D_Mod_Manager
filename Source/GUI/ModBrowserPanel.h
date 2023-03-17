#ifndef _MOD_BROWSER_PANEL_H_
#define _MOD_BROWSER_PANEL_H_

#include "Manager/ModManager.h"
#include "Mod/OrganizedModCollection.h"

#include <boost/signals2.hpp>
#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/hyperlink.h>
#include <wx/srchctrl.h>

class DOSBoxVersionCollection;
class GameProcessTerminatedEvent;
class GameVersion;
class GameVersionCollection;
class LaunchFailedEvent;
class Mod;
class ModAuthorInformation;
class ModManager;

class ModBrowserPanel final : public wxPanel,
							  public ModManager::Listener {
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

	void clear();
	void clearSearch();

	void onModSearchTextChanged(wxCommandEvent & event);
	void onModSearchCancelled(wxCommandEvent & event);
	void onSelectRandomModButtonPressed(wxCommandEvent & event);
	void onModListFilterTypeSelected(wxCommandEvent & event);
	void onModListSortTypeSelected(wxCommandEvent & event);
	void onModListSortDirectionSelected(wxCommandEvent & event);
	void onModSelected(wxCommandEvent & event);
	void onModVersionSelected(wxCommandEvent & event);
	void onModVersionTypeSelected(wxCommandEvent & event);
	void onPreferredDOSBoxVersionSelected(wxCommandEvent & event);
	void onModGameVersionSelected(wxCommandEvent & event);
	void onPreferredGameVersionSelected(wxCommandEvent & event);
	void onIPAddressTextChanged(wxCommandEvent & event);
	void onPortTextChanged(wxCommandEvent & event);
	void onModGameTypeSelected(wxCommandEvent & event);
	void onClearButtonPressed(wxCommandEvent & event);
	void onLaunchButtonPressed(wxCommandEvent & event);

	// ModManager::Listener Virtuals
	virtual void modSelectionChanged(const std::shared_ptr<Mod> & mod, size_t modVersionIndex, size_t modVersionTypeIndex, size_t modGameVersionIndex) override;
	virtual void gameTypeChanged(GameType gameType) override;
	virtual void preferredDOSBoxVersionChanged(const std::shared_ptr<DOSBoxVersion> & dosboxVersion) override;
	virtual void preferredGameVersionChanged(const std::shared_ptr<GameVersion> & gameVersion) override;
	virtual void dosboxServerIPAddressChanged(const std::string & ipAddress) override;
	virtual void dosboxLocalServerPortChanged(uint16_t port) override;
	virtual void dosboxRemoteServerPortChanged(uint16_t port) override;

private:
	void onLaunchError(std::string errorMessage);
	void onGameProcessTerminated(uint64_t nativeExitCode, bool forceTerminated);
	void onLaunchFailed(LaunchFailedEvent & launchFailedEvent);
	void onGameProcessEnded(GameProcessTerminatedEvent & gameProcessTerminatedEvent);
	void onFilterTypeChanged(OrganizedModCollection::FilterType filterType);
	void onSortOptionsChanged(OrganizedModCollection::SortType sortType, OrganizedModCollection::SortDirection sortDirection);
	void onSelectedModChanged(std::shared_ptr<Mod> mod);
	void onSelectedGameVersionChanged(std::shared_ptr<GameVersion> gameVersion);
	void onSelectedTeamChanged(std::shared_ptr<ModAuthorInformation> team);
	void onSelectedAuthorChanged(std::shared_ptr<ModAuthorInformation> author);
	void onOrganizedModCollectionChanged(const std::vector<std::shared_ptr<Mod>> & organizedMods);
	void onOrganizedModGameVersionCollectionChanged(const std::vector<std::shared_ptr<GameVersion>> & organizedMods);
	void onOrganizedModTeamCollectionChanged(const std::vector<std::shared_ptr<ModAuthorInformation>> & organizedMods);
	void onOrganizedModAuthorCollectionChanged(const std::vector<std::shared_ptr<ModAuthorInformation>> & organizedMods);
	void onDOSBoxVersionCollectionSizeChanged(DOSBoxVersionCollection & dosboxVersionCollection);
	void onDOSBoxVersionCollectionItemModified(DOSBoxVersionCollection & dosboxVersionCollection, DOSBoxVersion & dosboxVersion);
	void onGameVersionCollectionSizeChanged(GameVersionCollection & gameVersionCollection);
	void onGameVersionCollectionItemModified(GameVersionCollection & gameVersionCollection, GameVersion & gameVersion);

	std::shared_ptr<ModManager> m_modManager;
	std::shared_ptr<GameVersion> m_activeGameVersion;
	std::future<bool> m_runSelectedModFuture;
	boost::signals2::connection m_launchErrorConnection;
	boost::signals2::connection m_gameProcessTerminatedConnection;
	boost::signals2::connection m_filterTypeChangedConnection;
	boost::signals2::connection m_sortOptionsChangedConnection;
	boost::signals2::connection m_selectedModChangedConnection;
	boost::signals2::connection m_selectedGameVersionChangedConnection;
	boost::signals2::connection m_selectedTeamChangedConnection;
	boost::signals2::connection m_selectedAuthorChangedConnection;
	boost::signals2::connection m_organizedModCollectionChangedConnection;
	boost::signals2::connection m_organizedModGameVersionCollectionChangedConnection;
	boost::signals2::connection m_organizedModTeamCollectionChangedConnection;
	boost::signals2::connection m_organizedModAuthorCollectionChangedConnection;
	boost::signals2::connection m_dosboxVersionCollectionSizeChangedConnection;
	boost::signals2::connection m_dosboxVersionCollectionItemModifiedConnection;
	boost::signals2::connection m_gameVersionCollectionSizeChangedConnection;
	boost::signals2::connection m_gameVersionCollectionItemModifiedConnection;
	std::string m_searchQuery;
	std::vector<ModMatch> m_modMatches;
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
	wxStaticBox * m_modInfoBox;
	wxScrolledWindow * m_modInfoPanel;
	wxStaticText * m_modNameText;
	wxStaticText * m_modTypeText;
	wxStaticText * m_initialReleaseDateText;
	wxStaticText * m_latestReleaseDateLabel;
	wxStaticText * m_latestReleaseDateText;
	wxStaticText * m_supportedGameVersionsText;
	wxStaticText * m_modWebsiteHyperlinkLabel;
	wxHyperlinkCtrl * m_modWebsiteHyperlink;
	wxStaticText * m_notesLabel;
	wxStaticText * m_notesText;
	wxStaticText * m_teamWebsiteHyperlinkLabel;
	wxHyperlinkCtrl * m_teamWebsiteHyperlink;
	wxStaticText * m_teamEmailHyperlinkLabel;
	wxHyperlinkCtrl * m_teamEmailHyperlink;
	wxStaticText * m_teamLocationLabel;
	wxStaticText * m_teamLocationText;
	wxStaticText * m_teamMembersLabel;
	wxStaticText * m_teamMembersText;
	wxPanel * m_gameOptionsPanel;
	wxTextCtrl * m_ipAddressTextField;
	wxStaticText * m_portLabel;
	wxTextCtrl * m_portTextField;
	wxComboBox * m_preferredDOSBoxVersionComboBox;
	wxComboBox * m_modGameTypeComboBox;
	wxComboBox * m_preferredGameVersionComboBox;
	wxButton * m_launchButton;
};

#endif // _MOD_BROWSER_PANEL_H_
