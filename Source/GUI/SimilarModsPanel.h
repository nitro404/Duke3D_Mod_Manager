#ifndef _SIMILAR_MODS_PANEL_H_
#define _SIMILAR_MODS_PANEL_H_

#include <boost/signals2.hpp>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/hyperlink.h>

#include <memory>
#include <vector>

class Mod;
class ModCollection;

class wxGenericHyperlinkCtrl;

class SimilarModsPanel final : public wxPanel {
public:
	SimilarModsPanel(std::shared_ptr<ModCollection> mods, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~SimilarModsPanel();

	bool setMod(std::shared_ptr<Mod> mod);
	bool setMods(std::shared_ptr<ModCollection> mods);

	boost::signals2::signal<void (std::string /* modID */)> modSelectionRequested;

private:
	void onSimilarModDeepLinkClicked(wxHyperlinkEvent & event);

	std::shared_ptr<ModCollection> m_mods;
	std::shared_ptr<Mod> m_mod;

	std::vector<wxGenericHyperlinkCtrl *> m_similarModDeepLinks;
	wxFlexGridSizer * m_similarModsPanelSizer;

	SimilarModsPanel(const SimilarModsPanel &) = delete;
	const SimilarModsPanel & operator = (const SimilarModsPanel &) = delete;
};

#endif // _SIMILAR_MODS_PANEL_H_
