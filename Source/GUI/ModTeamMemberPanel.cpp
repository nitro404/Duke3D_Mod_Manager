#include "ModTeamMemberPanel.h"

#include "Manager/SettingsManager.h"
#include "Mod/ModTeamMember.h"
#include "WXUtilities.h"

#include <Analytics/Segment/SegmentAnalytics.h>

#include <any>
#include <map>

ModTeamMemberPanel::ModTeamMemberPanel(std::shared_ptr<ModTeamMember> teamMember, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Mod Team Member")
	, m_teamMember(teamMember) {
	wxGenericHyperlinkCtrl * nameDeepLink = WXUtilities::createDeepLink(this, wxID_ANY, wxString::FromUTF8(teamMember->getName()), teamMember->getName(), wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxHL_ALIGN_LEFT, "Team Member Name");
	nameDeepLink->Bind(wxEVT_HYPERLINK, &ModTeamMemberPanel::onModTeamMemberNameDeepLinkClicked, this);

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
			wxStaticText * aliasText = new wxStaticText(detailsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Team Member Alias");
			// Note: For some reason ampersands are not rendered if you assign the text contents upon creation rather than through the set label text function
			aliasText->SetLabelText(wxString::FromUTF8(teamMember->getAlias()));

			detailsPanelSizer->Add(aliasLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(aliasText, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasLocation()) {
			wxStaticText * locationLabel = new wxStaticText(detailsPanel, wxID_ANY, "Location:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			locationLabel->SetFont(locationLabel->GetFont().MakeBold());
			wxStaticText * locationText = new wxStaticText(detailsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Team Member Location");
			// Note: For some reason ampersands are not rendered if you assign the text contents upon creation rather than through the set label text function
			locationText->SetLabelText(wxString::FromUTF8(teamMember->getLocation().getDetails()));

			detailsPanelSizer->Add(locationLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(locationText, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasEmail()) {
			wxStaticText * emailLabel = new wxStaticText(detailsPanel, wxID_ANY, "Email:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			emailLabel->SetFont(emailLabel->GetFont().MakeBold());
			wxGenericHyperlinkCtrl * emailHyperlink = WXUtilities::createHyperlink(detailsPanel, wxID_ANY, teamMember->getEmail(), "mailto:" + teamMember->getEmail(), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Member E-Mail");
			emailHyperlink->Bind(wxEVT_HYPERLINK, &ModTeamMemberPanel::onModTeamMemberEmailHyperlinkClicked, this);

			detailsPanelSizer->Add(emailLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(emailHyperlink, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasTwitter()) {
			wxStaticText * twitterLabel = new wxStaticText(detailsPanel, wxID_ANY, "Twitter:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			twitterLabel->SetFont(twitterLabel->GetFont().MakeBold());
			wxGenericHyperlinkCtrl * twitterHyperlink = WXUtilities::createHyperlink(detailsPanel, wxID_ANY, teamMember->getTwitter(), "https://twitter.com/" + teamMember->getTwitter(), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Member Twitter");
			twitterHyperlink->Bind(wxEVT_HYPERLINK, &ModTeamMemberPanel::onModTeamMemberTwitterHyperlinkClicked, this);

			detailsPanelSizer->Add(twitterLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(twitterHyperlink, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasWebsite()) {
			wxStaticText * websiteLabel = new wxStaticText(detailsPanel, wxID_ANY, "Website:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			websiteLabel->SetFont(websiteLabel->GetFont().MakeBold());
			wxGenericHyperlinkCtrl * websiteHyperlink = WXUtilities::createHyperlink(detailsPanel, wxID_ANY, teamMember->getWebsite(), teamMember->getWebsite(), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Member Website");
			websiteHyperlink->Bind(wxEVT_HYPERLINK, &ModTeamMemberPanel::onModTeamMemberWebsiteHyperlinkClicked, this);

			detailsPanelSizer->Add(websiteLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(websiteHyperlink, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasYouTube()) {
			wxStaticText * youtubeLabel = new wxStaticText(detailsPanel, wxID_ANY, "YouTube:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			youtubeLabel->SetFont(youtubeLabel->GetFont().MakeBold());
			wxGenericHyperlinkCtrl * youtubeHyperlink = WXUtilities::createHyperlink(detailsPanel, wxID_ANY, teamMember->getYouTube(), "https://youtube.com/" + teamMember->getYouTube(), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Member YouTube");
			youtubeHyperlink->Bind(wxEVT_HYPERLINK, &ModTeamMemberPanel::onModTeamMemberYouTubeHyperlinkClicked, this);

			detailsPanelSizer->Add(youtubeLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(youtubeHyperlink, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasReddit()) {
			wxStaticText * redditLabel = new wxStaticText(detailsPanel, wxID_ANY, "Reddit:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			redditLabel->SetFont(redditLabel->GetFont().MakeBold());
			wxGenericHyperlinkCtrl * redditHyperlink = WXUtilities::createHyperlink(detailsPanel, wxID_ANY, teamMember->getReddit(), "https://www.reddit.com/user/" + teamMember->getReddit(), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Member Reddit");
			redditHyperlink->Bind(wxEVT_HYPERLINK, &ModTeamMemberPanel::onModTeamMemberRedditHyperlinkClicked, this);

			detailsPanelSizer->Add(redditLabel, 1, wxEXPAND | wxALL);
			detailsPanelSizer->Add(redditHyperlink, 1, wxEXPAND | wxALL);
		}

		if(teamMember->hasGitHub()) {
			wxStaticText * githubLabel = new wxStaticText(detailsPanel, wxID_ANY, "GitHub:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			githubLabel->SetFont(githubLabel->GetFont().MakeBold());
			wxGenericHyperlinkCtrl * githubHyperlink = WXUtilities::createHyperlink(detailsPanel, wxID_ANY, teamMember->getGitHub(), "https://github.com/" + teamMember->getGitHub(), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Member GitHub");
			githubHyperlink->Bind(wxEVT_HYPERLINK, &ModTeamMemberPanel::onModTeamMemberGitHubHyperlinkClicked, this);

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
			wxGenericHyperlinkCtrl * steamHyperlink = WXUtilities::createHyperlink(detailsPanel, wxID_ANY, teamMember->getSteamID(), "https://steamcommunity.com/id/" + teamMember->getSteamID(), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Member Steam ID");
			steamHyperlink->Bind(wxEVT_HYPERLINK, &ModTeamMemberPanel::onModTeamMemberSteamHyperlinkClicked, this);

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
	modTeamMemberPanelSizer->Add(nameDeepLink, 1, wxEXPAND | wxALL);
	modTeamMemberPanelSizer->AddSpacer(1);
	modTeamMemberPanelSizer->Add(detailsContainerPanel, 1, wxEXPAND | wxALL);
	SetSizer(modTeamMemberPanelSizer);

	wxFlexGridSizer * detailsContainerSizer = new wxFlexGridSizer(2, 0, 0);
	detailsContainerSizer->Add(detailsSpacerPanel, 1, wxEXPAND | wxALL);
	detailsContainerSizer->Add(detailsPanel, 1, wxEXPAND | wxALL);
	detailsContainerPanel->SetSizer(detailsContainerSizer);
}

ModTeamMemberPanel::~ModTeamMemberPanel() { }

void ModTeamMemberPanel::onModTeamMemberNameDeepLinkClicked(wxHyperlinkEvent & event) {
	event.Skip(false);

	modTeamMemberSelectionRequested(m_teamMember->getName());

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["teamMemberName"] = m_teamMember->getName();

		SegmentAnalytics::getInstance()->track("Team Member Deep Link Clicked", properties);
	}
}

void ModTeamMemberPanel::onModTeamMemberEmailHyperlinkClicked(wxHyperlinkEvent & event) {
	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["teamMemberName"] = m_teamMember->getName();
		properties["teamMemberEmail"] = m_teamMember->getEmail();
		properties["url"] = std::string(event.GetURL().mb_str());

		SegmentAnalytics::getInstance()->track("Team Member Email Hyperlink Clicked", properties);
	}
}

void ModTeamMemberPanel::onModTeamMemberTwitterHyperlinkClicked(wxHyperlinkEvent & event) {
	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["teamMemberName"] = m_teamMember->getName();
		properties["teamMemberTwitter"] = m_teamMember->getTwitter();
		properties["url"] = std::string(event.GetURL().mb_str());

		SegmentAnalytics::getInstance()->track("Team Member Twitter Hyperlink Clicked", properties);
	}
}

void ModTeamMemberPanel::onModTeamMemberWebsiteHyperlinkClicked(wxHyperlinkEvent & event) {
	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["teamMemberName"] = m_teamMember->getName();
		properties["url"] = std::string(event.GetURL().mb_str());

		SegmentAnalytics::getInstance()->track("Team Member Website Hyperlink Clicked", properties);
	}
}

void ModTeamMemberPanel::onModTeamMemberYouTubeHyperlinkClicked(wxHyperlinkEvent & event) {
	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["teamMemberName"] = m_teamMember->getName();
		properties["teamMemberYouTube"] = m_teamMember->getYouTube();
		properties["url"] = std::string(event.GetURL().mb_str());

		SegmentAnalytics::getInstance()->track("Team Member YouTube Hyperlink Clicked", properties);
	}
}

void ModTeamMemberPanel::onModTeamMemberRedditHyperlinkClicked(wxHyperlinkEvent & event) {
	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["teamMemberName"] = m_teamMember->getName();
		properties["teamMemberReddit"] = m_teamMember->getReddit();
		properties["url"] = std::string(event.GetURL().mb_str());

		SegmentAnalytics::getInstance()->track("Team Member Reddit Hyperlink Clicked", properties);
	}
}

void ModTeamMemberPanel::onModTeamMemberGitHubHyperlinkClicked(wxHyperlinkEvent & event) {
	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["teamMemberName"] = m_teamMember->getName();
		properties["url"] = std::string(event.GetURL().mb_str());

		SegmentAnalytics::getInstance()->track("Team Member GitHub Hyperlink Clicked", properties);
	}
}

void ModTeamMemberPanel::onModTeamMemberSteamHyperlinkClicked(wxHyperlinkEvent & event) {
	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["teamMemberName"] = m_teamMember->getName();
		properties["teamMemberSteamID"] = m_teamMember->getSteamID();
		properties["url"] = std::string(event.GetURL().mb_str());

		SegmentAnalytics::getInstance()->track("Team Member Steam Hyperlink Clicked", properties);
	}
}
