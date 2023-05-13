#ifndef _MOD_INFO_PANEL_H_
#define _MOD_INFO_PANEL_H_

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
class ModDownloadsPanel;
class ModTeamMembersPanel;
class GameVersionCollection;

class ModInfoPanel final : public wxScrolledWindow {
public:
	ModInfoPanel(std::shared_ptr<ModCollection> mods, std::shared_ptr<GameVersionCollection> gameVersions, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~ModInfoPanel();

	void setMod(std::shared_ptr<Mod> mod);

private:
	std::shared_ptr<Mod> m_mod;
	std::shared_ptr<ModCollection> m_mods;
	std::shared_ptr<GameVersionCollection> m_gameVersions;

	wxPanel * m_modPanel;
	wxStaticText * m_modNameLabel;
	wxStaticText * m_modNameText;
	wxStaticText * m_modAliasLabel;
	wxStaticText * m_modAliasText;
	wxStaticText * m_modTypeText;
	wxStaticText * m_initialReleaseDateText;
	wxStaticText * m_latestReleaseDateLabel;
	wxStaticText * m_latestReleaseDateText;
	wxStaticText * m_supportedGameVersionsLabel;
	wxStaticText * m_supportedGameVersionsText;
	wxStaticText * m_modWebsiteHyperlinkLabel;
	wxHyperlinkCtrl * m_modWebsiteHyperlink;
	wxStaticText * m_modRepositoryHyperlinkLabel;
	wxHyperlinkCtrl * m_modRepositoryHyperlink;
	wxStaticText * m_notesLabel;
	wxStaticText * m_notesText;
	wxStaticText * m_teamNameLabel;
	wxStaticText * m_teamNameText;
	wxStaticText * m_teamWebsiteHyperlinkLabel;
	wxHyperlinkCtrl * m_teamWebsiteHyperlink;
	wxStaticText * m_teamEmailHyperlinkLabel;
	wxHyperlinkCtrl * m_teamEmailHyperlink;
	wxStaticText * m_teamTwitterLabel;
	wxHyperlinkCtrl * m_teamTwitterHyperlink;
	wxStaticText * m_teamDiscordLabel;
	wxHyperlinkCtrl * m_teamDiscordHyperlink;
	wxStaticText * m_teamLocationLabel;
	wxStaticText * m_teamLocationText;
	ModTeamMembersPanel * m_teamMembersPanel;
	wxStaticText * m_downloadsLabel;
	ModDownloadsPanel * m_downloadsPanel;
};

#endif // _MOD_INFO_PANEL_H_
