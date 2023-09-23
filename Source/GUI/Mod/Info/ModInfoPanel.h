#ifndef _MOD_INFO_PANEL_H_
#define _MOD_INFO_PANEL_H_

#include <boost/signals2.hpp>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/hyperlink.h>

#include <memory>

class Mod;
class ModCollection;
class ModDependenciesPanel;
class ModDownloadsPanel;
class ModTeamMembersPanel;
class ModVersionType;
class GameVersionCollection;
class RelatedModsPanel;
class SimilarModsPanel;

class ModInfoPanel final : public wxScrolledWindow {
public:
	ModInfoPanel(std::shared_ptr<ModCollection> mods, std::shared_ptr<GameVersionCollection> gameVersions, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~ModInfoPanel();

	void setMod(std::shared_ptr<Mod> mod);
	void setModVersionType(std::shared_ptr<ModVersionType> modVersionType);

	boost::signals2::signal<void (std::string /* modID */)> modSelectionRequested;
	boost::signals2::signal<void (std::string /* modTeamName */)> modTeamSelectionRequested;
	boost::signals2::signal<void (std::string /* modTeamMemberName */)> modTeamMemberSelectionRequested;
	boost::signals2::signal<void (std::string /* modID */, std::string /* modVersion */, std::string /* modVersionType */)> modVersionTypeSelectionRequested;

private:
	void onModAliasDeepLinkClicked(wxHyperlinkEvent & event);
	void onModTeamNameDeepLinkClicked(wxHyperlinkEvent & event);
	void onModWebsiteHyperlinkClicked(wxHyperlinkEvent & event);
	void onModRepositoryHyperlinkClicked(wxHyperlinkEvent & event);
	void onTeamWebsiteHyperlinkClicked(wxHyperlinkEvent & event);
	void onTeamEmailHyperlinkClicked(wxHyperlinkEvent & event);
	void onTeamTwitterHyperlinkClicked(wxHyperlinkEvent & event);
	void onTeamDiscordHyperlinkClicked(wxHyperlinkEvent & event);
	void onRelatedModSelectionRequested(const std::string & relatedModID);
	void onSimilarModSelectionRequested(const std::string & similarModID);
	void onModTeamMemberSelectionRequested(const std::string & modTeamMemberName);
	void onModVersionTypeSelectionRequested(const std::string & modID, const std::string & modVersion, const std::string & modVersionType);

	std::shared_ptr<Mod> m_mod;
	std::shared_ptr<ModVersionType> m_modVersionType;
	std::shared_ptr<ModCollection> m_mods;
	std::shared_ptr<GameVersionCollection> m_gameVersions;

	wxStaticText * m_modNameLabel;
	wxStaticText * m_modNameText;
	wxStaticText * m_modAliasLabel;
	wxGenericHyperlinkCtrl * m_modAliasDeepLink;
	wxStaticText * m_modTypeText;
	wxStaticText * m_initialReleaseDateText;
	wxStaticText * m_latestReleaseDateLabel;
	wxStaticText * m_latestReleaseDateText;
	wxStaticText * m_supportedGameVersionsLabel;
	wxStaticText * m_supportedGameVersionsText;
	wxStaticText * m_dependenciesLabel;
	ModDependenciesPanel * m_dependenciesPanel;
	wxStaticText * m_modWebsiteHyperlinkLabel;
	wxGenericHyperlinkCtrl * m_modWebsiteHyperlink;
	wxStaticText * m_modRepositoryHyperlinkLabel;
	wxGenericHyperlinkCtrl * m_modRepositoryHyperlink;
	wxStaticText * m_notesLabel;
	wxStaticText * m_notesText;
	wxStaticText * m_relatedModsLabel;
	RelatedModsPanel * m_relatedModsPanel;
	wxStaticText * m_similarModsLabel;
	wxWindow * m_similarModsSpacers[2];
	SimilarModsPanel * m_similarModsPanel;
	wxStaticText * m_teamNameLabel;
	wxGenericHyperlinkCtrl * m_teamNameDeepLink;
	wxStaticText * m_teamWebsiteHyperlinkLabel;
	wxGenericHyperlinkCtrl * m_teamWebsiteHyperlink;
	wxStaticText * m_teamEmailHyperlinkLabel;
	wxGenericHyperlinkCtrl * m_teamEmailHyperlink;
	wxStaticText * m_teamTwitterLabel;
	wxGenericHyperlinkCtrl * m_teamTwitterHyperlink;
	wxStaticText * m_teamDiscordLabel;
	wxGenericHyperlinkCtrl * m_teamDiscordHyperlink;
	wxStaticText * m_teamLocationLabel;
	wxStaticText * m_teamLocationText;
	wxStaticText * m_teamMembersLabel;
	wxWindow * m_teamMembersSpacers[2];
	ModTeamMembersPanel * m_teamMembersPanel;
	wxStaticText * m_downloadsLabel;
	wxWindow * m_downloadsSpacers[2];
	ModDownloadsPanel * m_downloadsPanel;
	boost::signals2::connection m_relatedModSelectionRequestedConnection;
	boost::signals2::connection m_similarModSelectionRequestedConnection;
	boost::signals2::connection m_modTeamMemberSelectionRequestedConnection;
	boost::signals2::connection m_modVersionTypeSelectionRequestedConnection;

	ModInfoPanel(const ModInfoPanel &) = delete;
	const ModInfoPanel & operator = (const ModInfoPanel &) = delete;
};

#endif // _MOD_INFO_PANEL_H_
