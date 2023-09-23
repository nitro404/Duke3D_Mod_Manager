#ifndef _MOD_DEPENDENCY_PANEL_H_
#define _MOD_DEPENDENCY_PANEL_H_

#include <boost/signals2.hpp>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/hyperlink.h>

#include <memory>

class ModVersionType;

class ModDependencyPanel final : public wxPanel {
public:
	ModDependencyPanel(std::shared_ptr<ModVersionType> modVersionType, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~ModDependencyPanel();

	boost::signals2::signal<void (std::string /* modID */, std::string /* modVersion */, std::string /* modVersionType */)> modVersionTypeSelectionRequested;

private:
	void onModDependencyDeepLinkClicked(wxHyperlinkEvent & event);

	std::shared_ptr<ModVersionType> m_modVersionType;

	ModDependencyPanel(const ModDependencyPanel &) = delete;
	const ModDependencyPanel & operator = (const ModDependencyPanel &) = delete;
};

#endif // _MOD_DEPENDENCY_PANEL_H_
