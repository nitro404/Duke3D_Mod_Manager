#ifndef _MOD_BROWSER_PANEL_H_
#define _MOD_BROWSER_PANEL_H_

#include "Manager/ModManager.h"
#include "Mod/OrganizedModCollection.h"

#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/hyperlink.h>
#include <wx/srchctrl.h>

class GameVersion;
class Mod;
class ModAuthorInformation;
class ModManager;

class ModBrowserPanel final : public wxPanel,
							  public ModManager::Listener,
							  public OrganizedModCollection::Listener {
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
	void updateModSelection();
	void updateModInfo();
	void updateModGameType();
	void updatePreferredGameVersion();
	void updateDOSBoxServerSettings();
	void updateDOSBoxServerIPAddress();
	void updateDOSBoxServerPort();

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
	void onIPAddressTextChanged(wxCommandEvent & event);
	void onPortTextChanged(wxCommandEvent & event);
	void onModGameTypeSelected(wxCommandEvent & event);
	void onLaunchModButtonPressed(wxCommandEvent & event);

	// ModManager::Listener Virtuals
	virtual void modSelectionChanged(const std::shared_ptr<Mod> & mod, size_t modVersionIndex, size_t modVersionTypeIndex) override;
	virtual void gameTypeChanged(GameType gameType) override;
	virtual void preferredGameVersionChanged(const std::shared_ptr<GameVersion> & gameVersion) override;
	virtual void dosboxServerIPAddressChanged(const std::string & ipAddress) override;
	virtual void dosboxLocalServerPortChanged(uint16_t port) override;
	virtual void dosboxRemoteServerPortChanged(uint16_t port) override;

	// OrganizedModCollection::Listener Virtuals
	virtual void filterTypeChanged(OrganizedModCollection::FilterType filterType) override;
	virtual void sortOptionsChanged(OrganizedModCollection::SortType sortType, OrganizedModCollection::SortDirection sortDirection) override;
	virtual void selectedModChanged(const std::shared_ptr<Mod> & mod) override;
	virtual void selectedGameVersionChanged(const std::shared_ptr<GameVersion> & gameVersion) override;
	virtual void selectedTeamChanged(const std::shared_ptr<ModAuthorInformation> & team) override;
	virtual void selectedAuthorChanged(const std::shared_ptr<ModAuthorInformation> & author) override;
	virtual void organizedModCollectionChanged(const std::vector<std::shared_ptr<Mod>> & organizedMods) override;
	virtual void organizedModGameVersionCollectionChanged(const std::vector<std::shared_ptr<GameVersion>> & organizedMods) override;
	virtual void organizedModTeamCollectionChanged(const std::vector<std::shared_ptr<ModAuthorInformation>> & organizedMods) override;
	virtual void organizedModAuthorCollectionChanged(const std::vector<std::shared_ptr<ModAuthorInformation>> & organizedMods) override;

private:
	std::shared_ptr<ModManager> m_modManager;
	std::string m_searchQuery;
	std::vector<ModMatch> m_modMatches;
	wxSearchCtrl * m_modSearchTextField;
	wxButton * m_selectRandomModButton;
	wxComboBox * m_modListFilterTypeComboBox;
	wxComboBox * m_modListSortTypeComboBox;
	wxComboBox * m_modListSortDirectionComboBox;
	wxStaticText * m_modListLabel;
	wxListBox * m_modListBox;
	wxStaticText * m_modVersionListLabel;
	wxListBox * m_modVersionListBox;
	wxStaticText * m_modVersionTypeListLabel;
	wxListBox * m_modVersionTypeListBox;
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
	wxComboBox * m_modGameTypeComboBox;
	wxComboBox * m_modGameVersionComboBox;
	wxButton * m_launchModButton;
};

#endif // _MOD_BROWSER_PANEL_H_
