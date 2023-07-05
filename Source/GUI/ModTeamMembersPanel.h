#ifndef _MOD_TEAM_MEMBERS_PANEL_H_
#define _MOD_TEAM_MEMBERS_PANEL_H_

#include <Signal/SignalConnectionGroup.h>

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
#include <vector>

class ModTeam;
class ModTeamMember;
class ModTeamMemberPanel;

class ModTeamMembersPanel final : public wxPanel {
public:
	ModTeamMembersPanel(wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~ModTeamMembersPanel();

	void setTeam(std::shared_ptr<ModTeam> team);

	boost::signals2::signal<void (std::string /* modTeamMemberName */)> modTeamMemberSelectionRequested;

private:
	void onModTeamMemberSelectionRequested(const std::string & modTeamMemberName);

	std::shared_ptr<ModTeam> m_team;

	wxFlexGridSizer * m_teamMembersPanelSizer;
	std::vector<ModTeamMemberPanel *> m_teamMemberPanels;
	SignalConnectionGroup m_modTeamMemberPanelConnections;

	ModTeamMembersPanel(const ModTeamMembersPanel &) = delete;
	const ModTeamMembersPanel & operator = (const ModTeamMembersPanel &) = delete;
};

#endif // _MOD_TEAM_MEMBERS_PANEL_H_
