#ifndef _MOD_TEAM_MEMBER_PANEL_H_
#define _MOD_TEAM_MEMBER_PANEL_H_

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/hyperlink.h>

#include <memory>

class ModTeamMember;

class ModTeamMemberPanel final : public wxPanel {
public:
	ModTeamMemberPanel(std::shared_ptr<ModTeamMember> teamMember, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~ModTeamMemberPanel();

	ModTeamMemberPanel(const ModTeamMemberPanel &) = delete;
	const ModTeamMemberPanel & operator = (const ModTeamMemberPanel &) = delete;
};

#endif // _MOD_TEAM_MEMBER_PANEL_H_
