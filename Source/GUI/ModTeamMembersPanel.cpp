#include "ModTeamMembersPanel.h"

#include "ModTeamMemberPanel.h"
#include "Mod/ModTeam.h"
#include "Mod/ModTeamMember.h"

#include <spdlog/spdlog.h>

#include <sstream>

ModTeamMembersPanel::ModTeamMembersPanel(wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Mod Team Members")
	, m_teamMembersPanelSizer(nullptr) {
	int border = 5;

	m_teamMembersPanelSizer = new wxFlexGridSizer(1, border, border);
	SetSizer(m_teamMembersPanelSizer);
}

ModTeamMembersPanel::~ModTeamMembersPanel() { }

void ModTeamMembersPanel::setTeam(std::shared_ptr<ModTeam> team) {
	if(m_team == team) {
		return;
	}

	m_team = team;

	if(m_team == nullptr) {
		return;
	}

	DestroyChildren();
	m_teamMemberPanels.clear();

	for(size_t i = 0; i < team->numberOfMembers(); i++) {
		ModTeamMemberPanel * teamMemberPanel = new ModTeamMemberPanel(team->getMember(i), this);
		m_teamMembersPanelSizer->Add(teamMemberPanel, 1, wxEXPAND | wxALL);
		m_teamMemberPanels.push_back(teamMemberPanel);
	}

	Layout();
}
