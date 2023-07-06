#ifndef _MOD_TEAM_MEMBER_PANEL_H_
#define _MOD_TEAM_MEMBER_PANEL_H_

#include <boost/signals2.hpp>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/hyperlink.h>

#include <memory>
#include <string>

class ModTeamMember;

class ModTeamMemberPanel final : public wxPanel {
public:
	ModTeamMemberPanel(std::shared_ptr<ModTeamMember> teamMember, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~ModTeamMemberPanel();

	boost::signals2::signal<void (std::string /* modTeamMemberName */)> modTeamMemberSelectionRequested;

private:
	void onModTeamMemberNameDeepLinkClicked(wxHyperlinkEvent & event);

	ModTeamMemberPanel(const ModTeamMemberPanel &) = delete;
	const ModTeamMemberPanel & operator = (const ModTeamMemberPanel &) = delete;
};

#endif // _MOD_TEAM_MEMBER_PANEL_H_
