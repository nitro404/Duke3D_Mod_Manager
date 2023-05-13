#include "ModTeamMemberPanel.h"

#include "Mod/ModTeamMember.h"

ModTeamMemberPanel::ModTeamMemberPanel(std::shared_ptr<ModTeamMember> teamMember, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Mod Team Member") {
	wxStaticText * nameText = new wxStaticText(this, wxID_ANY, teamMember != nullptr ? wxString::FromUTF8(teamMember->getName()) : wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Team Member Name");

	int border = 5;

	wxPanel * detailsContainerPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);

	wxPanel * detailsSpacerPanel = new wxPanel(detailsContainerPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	detailsSpacerPanel->SetMinClientSize(wxSize(40, 1));

	wxPanel * detailsPanel = new wxPanel(detailsContainerPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER, "Mod Team Member Details");

	wxFlexGridSizer * detailsPanelSizer = new wxFlexGridSizer(2, border, border);
	detailsPanel->SetSizer(detailsPanelSizer);

	if(teamMember != nullptr) {
		if(teamMember->hasAlias()) {
			wxStaticText * aliasLabel = new wxStaticText(detailsPanel, wxID_ANY, "Alias:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			aliasLabel->SetFont(aliasLabel->GetFont().MakeBold());
			wxStaticText * aliasText = new wxStaticText(detailsPanel, wxID_ANY, wxString::FromUTF8(teamMember->getAlias()), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Team Member Alias");

			detailsPanelSizer->Add(aliasLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(aliasText, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasLocation()) {
			wxStaticText * locationLabel = new wxStaticText(detailsPanel, wxID_ANY, "Location:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			locationLabel->SetFont(locationLabel->GetFont().MakeBold());
			wxStaticText * locationText = new wxStaticText(detailsPanel, wxID_ANY, wxString::FromUTF8(teamMember->getLocation().getDetails()), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Team Member Location");

			detailsPanelSizer->Add(locationLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(locationText, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasEmail()) {
			wxStaticText * emailLabel = new wxStaticText(detailsPanel, wxID_ANY, "Email:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			emailLabel->SetFont(emailLabel->GetFont().MakeBold());
			wxHyperlinkCtrl * emailHyperlink = new wxHyperlinkCtrl(detailsPanel, wxID_ANY, teamMember->getEmail(), "mailto:" + teamMember->getEmail(), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Member E-Mail");

			detailsPanelSizer->Add(emailLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(emailHyperlink, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasTwitter()) {
			wxStaticText * twitterLabel = new wxStaticText(detailsPanel, wxID_ANY, "Twitter:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			twitterLabel->SetFont(twitterLabel->GetFont().MakeBold());
			wxHyperlinkCtrl * twitterHyperlink = new wxHyperlinkCtrl(detailsPanel, wxID_ANY, teamMember->getTwitter(), "https://twitter.com/" + teamMember->getTwitter(), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Member Twitter");

			detailsPanelSizer->Add(twitterLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(twitterHyperlink, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasWebsite()) {
			wxStaticText * websiteLabel = new wxStaticText(detailsPanel, wxID_ANY, "Website:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			websiteLabel->SetFont(websiteLabel->GetFont().MakeBold());
			wxHyperlinkCtrl * websiteHyperlink = new wxHyperlinkCtrl(detailsPanel, wxID_ANY, teamMember->getWebsite(), teamMember->getWebsite(), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Member Website");

			detailsPanelSizer->Add(websiteLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(websiteHyperlink, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasYouTube()) {
			wxStaticText * youtubeLabel = new wxStaticText(detailsPanel, wxID_ANY, "YouTube:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			youtubeLabel->SetFont(youtubeLabel->GetFont().MakeBold());
			wxHyperlinkCtrl * youtubeHyperlink = new wxHyperlinkCtrl(detailsPanel, wxID_ANY, teamMember->getYouTube(), "https://youtube.com/" + teamMember->getYouTube(), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Member YouTube");

			detailsPanelSizer->Add(youtubeLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(youtubeHyperlink, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasReddit()) {
			wxStaticText * redditLabel = new wxStaticText(detailsPanel, wxID_ANY, "Reddit:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			redditLabel->SetFont(redditLabel->GetFont().MakeBold());
			wxHyperlinkCtrl * redditHyperlink = new wxHyperlinkCtrl(detailsPanel, wxID_ANY, teamMember->getReddit(), "https://www.reddit.com/user/" + teamMember->getReddit(), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Member Reddit");

			detailsPanelSizer->Add(redditLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(redditHyperlink, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasGitHub()) {
			wxStaticText * githubLabel = new wxStaticText(detailsPanel, wxID_ANY, "GitHub:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			githubLabel->SetFont(githubLabel->GetFont().MakeBold());
			wxHyperlinkCtrl * githubHyperlink = new wxHyperlinkCtrl(detailsPanel, wxID_ANY, teamMember->getGitHub(), "https://github.com/" + teamMember->getGitHub(), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Member GitHub");

			detailsPanelSizer->Add(githubLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(githubHyperlink, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasDiscord()) {
			wxStaticText * discordLabel = new wxStaticText(detailsPanel, wxID_ANY, "Discord:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			discordLabel->SetFont(discordLabel->GetFont().MakeBold());
			wxStaticText * discordText = new wxStaticText(detailsPanel, wxID_ANY, teamMember->getDiscord(), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Team Member Discord");

			detailsPanelSizer->Add(discordLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(discordText, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasSteamID()) {
			wxStaticText * steamLabel = new wxStaticText(detailsPanel, wxID_ANY, "Steam ID:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			steamLabel->SetFont(steamLabel->GetFont().MakeBold());
			wxHyperlinkCtrl * steamHyperlink = new wxHyperlinkCtrl(detailsPanel, wxID_ANY, teamMember->getSteamID(), "https://steamcommunity.com/id/" + teamMember->getSteamID(), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Member Steam ID");

			detailsPanelSizer->Add(steamLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(steamHyperlink, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasAIM()) {
			wxStaticText * aimLabel = new wxStaticText(detailsPanel, wxID_ANY, "AIM:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			aimLabel->SetFont(aimLabel->GetFont().MakeBold());
			wxStaticText * aimText = new wxStaticText(detailsPanel, wxID_ANY, teamMember->getAIM(), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Team Member AIM");

			detailsPanelSizer->Add(aimLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(aimText, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasICQ()) {
			wxStaticText * icqLabel = new wxStaticText(detailsPanel, wxID_ANY, "ICQ:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			icqLabel->SetFont(icqLabel->GetFont().MakeBold());
			wxStaticText * icqText = new wxStaticText(detailsPanel, wxID_ANY, teamMember->getICQ(), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Team Member ICQ");

			detailsPanelSizer->Add(icqLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(icqText, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasYahoo()) {
			wxStaticText * yahooLabel = new wxStaticText(detailsPanel, wxID_ANY, "Yahoo:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			yahooLabel->SetFont(yahooLabel->GetFont().MakeBold());
			wxStaticText * yahooText = new wxStaticText(detailsPanel, wxID_ANY, teamMember->getYahoo(), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Team Member Yahoo");

			detailsPanelSizer->Add(yahooLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(yahooText, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasPhoneNumber()) {
			wxStaticText * phoneNumberLabel = new wxStaticText(detailsPanel, wxID_ANY, "Phone:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			phoneNumberLabel->SetFont(phoneNumberLabel->GetFont().MakeBold());
			wxStaticText * phoneNumberText = new wxStaticText(detailsPanel, wxID_ANY, teamMember->getPhoneNumber(), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Team Member Phone Number");

			detailsPanelSizer->Add(phoneNumberLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(phoneNumberText, 1, wxEXPAND | wxALL);
		}
	}

	wxFlexGridSizer * modTeamMemberPanelSizer = new wxFlexGridSizer(2, 0, 0);
	modTeamMemberPanelSizer->Add(nameText, 1, wxEXPAND | wxALL);
	modTeamMemberPanelSizer->AddSpacer(1);
	modTeamMemberPanelSizer->Add(detailsContainerPanel, 1, wxEXPAND | wxALL);
	SetSizer(modTeamMemberPanelSizer);

	wxFlexGridSizer * detailsContainerSizer = new wxFlexGridSizer(2, 0, 0);
	detailsContainerSizer->Add(detailsSpacerPanel, 1, wxEXPAND | wxALL);
	detailsContainerSizer->Add(detailsPanel, 1, wxEXPAND | wxALL);
	detailsContainerPanel->SetSizer(detailsContainerSizer);
}

ModTeamMemberPanel::~ModTeamMemberPanel() { }