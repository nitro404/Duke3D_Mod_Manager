#ifndef _MOD_TEAM_MEMBERS_PANEL_H_
#define _MOD_TEAM_MEMBERS_PANEL_H_

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/hyperlink.h>

#include <memory>
#include <vector>

class ModTeam;
class ModTeamMember;
class ModTeamMemberPanel;

class ModTeamMembersPanel final : public wxPanel {
public:
	ModTeamMembersPanel(wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~ModTeamMembersPanel();

	void setTeam(std::shared_ptr<ModTeam> team);

private:
	std::shared_ptr<ModTeam> m_team;

	wxFlexGridSizer * m_teamMembersPanelSizer;
	std::vector<ModTeamMemberPanel *> m_teamMemberPanels;
};

#endif // _MOD_TEAM_MEMBERS_PANEL_H_
