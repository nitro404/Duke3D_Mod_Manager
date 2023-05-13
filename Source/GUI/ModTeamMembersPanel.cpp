#include "ModTeamMembersPanel.h"

#include "ModTeamMemberPanel.h"
#include "Mod/ModTeam.h"
#include "Mod/ModTeamMember.h"

#include <spdlog/spdlog.h>

#include <sstream>

ModTeamMembersPanel::ModTeamMembersPanel(wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Mod Team Members")
	, m_modTeamPanelSizer(nullptr)
	, m_teamMembersLabel(nullptr)
	, m_modTeamMembersPanelSizer(nullptr)
	, m_teamMembersPanel(nullptr) {

	m_teamMembersLabel = new wxStaticText(this, wxID_ANY, "Team Members:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamMembersLabel->SetFont(m_teamMembersLabel->GetFont().MakeBold());

	m_teamMembersPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER, "Mod Team Member List");

	int border = 5;

	m_modTeamMembersPanelSizer = new wxFlexGridSizer(1, border, border);
	m_teamMembersPanel->SetSizer(m_modTeamMembersPanelSizer);

	m_modTeamPanelSizer = new wxFlexGridSizer(2, border, border);
	m_modTeamPanelSizer->Add(m_teamMembersLabel, 1, wxEXPAND | wxALL);
	m_modTeamPanelSizer->AddSpacer(1);
	m_modTeamPanelSizer->AddSpacer(1);
	m_modTeamPanelSizer->Add(m_teamMembersPanel, 1, wxEXPAND | wxALL);
	SetSizer(m_modTeamPanelSizer);
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

	m_teamMembersPanel->DestroyChildren();
	m_teamMemberPanels.clear();

	for(size_t i = 0; i < team->numberOfMembers(); i++) {
		ModTeamMemberPanel * teamMemberPanel = new ModTeamMemberPanel(team->getMember(i), m_teamMembersPanel);
		m_modTeamMembersPanelSizer->Add(teamMemberPanel, 1, wxEXPAND | wxALL);
		m_teamMemberPanels.push_back(teamMemberPanel);
	}

	Layout();
}

void ModTeamMembersPanel::setMinimumLeftColoumnWidth(int columnWidth) {
	m_teamMembersLabel->SetMinClientSize(wxSize(columnWidth, m_teamMembersLabel->GetMinClientSize().y));

	Layout();
}
