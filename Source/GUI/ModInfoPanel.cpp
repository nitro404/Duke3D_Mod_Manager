#include "ModInfoPanel.h"

#include "Manager/SettingsManager.h"
#include "ModDownloadsPanel.h"
#include "ModTeamMembersPanel.h"
#include "Mod/Mod.h"
#include "Mod/ModCollection.h"
#include "Mod/ModTeam.h"
#include "Mod/ModTeamMember.h"
#include "RelatedModsPanel.h"
#include "SimilarModsPanel.h"
#include "Game/GameVersionCollection.h"
#include "WXUtilities.h"

#include <Analytics/Segment/SegmentAnalytics.h>

#include <spdlog/spdlog.h>

#include <any>
#include <map>
#include <sstream>
#include <string>

ModInfoPanel::ModInfoPanel(std::shared_ptr<ModCollection> mods, std::shared_ptr<GameVersionCollection> gameVersions, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxScrolledWindow(parent, windowID, position, size, style, "Mod Information")
	, m_mods(mods)
	, m_gameVersions(gameVersions)
	, m_modNameLabel(nullptr)
	, m_modNameText(nullptr)
	, m_modAliasLabel(nullptr)
	, m_modAliasDeepLink(nullptr)
	, m_modTypeText(nullptr)
	, m_initialReleaseDateText(nullptr)
	, m_latestReleaseDateLabel(nullptr)
	, m_latestReleaseDateText(nullptr)
	, m_supportedGameVersionsLabel(nullptr)
	, m_supportedGameVersionsText(nullptr)
	, m_modWebsiteHyperlinkLabel(nullptr)
	, m_modWebsiteHyperlink(nullptr)
	, m_modRepositoryHyperlinkLabel(nullptr)
	, m_modRepositoryHyperlink(nullptr)
	, m_notesLabel(nullptr)
	, m_notesText(nullptr)
	, m_relatedModsLabel(nullptr)
	, m_relatedModsPanel(nullptr)
	, m_similarModsLabel(nullptr)
	, m_similarModsPanel(nullptr)
	, m_teamNameLabel(nullptr)
	, m_teamNameDeepLink(nullptr)
	, m_teamWebsiteHyperlinkLabel(nullptr)
	, m_teamWebsiteHyperlink(nullptr)
	, m_teamEmailHyperlinkLabel(nullptr)
	, m_teamEmailHyperlink(nullptr)
	, m_teamTwitterLabel(nullptr)
	, m_teamTwitterHyperlink(nullptr)
	, m_teamDiscordLabel(nullptr)
	, m_teamDiscordHyperlink(nullptr)
	, m_teamLocationLabel(nullptr)
	, m_teamLocationText(nullptr)
	, m_teamMembersLabel(nullptr)
	, m_teamMembersPanel(nullptr)
	, m_downloadsLabel(nullptr)
	, m_downloadsPanel(nullptr) {
	SetScrollRate(5, 5);

	m_modNameLabel = new wxStaticText(this, wxID_ANY, "Mod Name:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_modNameLabel->SetFont(m_modNameLabel->GetFont().MakeBold());
	m_modNameText = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_modAliasLabel = new wxStaticText(this, wxID_ANY, "Mod Alias:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_modAliasLabel->SetFont(m_modAliasLabel->GetFont().MakeBold());
	m_modAliasDeepLink = WXUtilities::createDeepLink(this, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT, "Mod Alias");
	m_modAliasDeepLink->Bind(wxEVT_HYPERLINK, &ModInfoPanel::onModAliasDeepLinkClicked, this);

	wxStaticText * modTypeLabel = new wxStaticText(this, wxID_ANY, "Mod Type:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	modTypeLabel->SetFont(modTypeLabel->GetFont().MakeBold());
	m_modTypeText = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	wxStaticText * initialReleaseDateLabel = new wxStaticText(this, wxID_ANY, "Initial Release Date:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	initialReleaseDateLabel->SetFont(initialReleaseDateLabel->GetFont().MakeBold());
	m_initialReleaseDateText = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_latestReleaseDateLabel = new wxStaticText(this, wxID_ANY, "Latest Release Date:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_latestReleaseDateLabel->SetFont(m_latestReleaseDateLabel->GetFont().MakeBold());
	m_latestReleaseDateText = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_supportedGameVersionsLabel = new wxStaticText(this, wxID_ANY, "Supported Game Versions:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_supportedGameVersionsLabel->SetFont(m_supportedGameVersionsLabel->GetFont().MakeBold());
	m_supportedGameVersionsText = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_modWebsiteHyperlinkLabel = new wxStaticText(this, wxID_ANY, "Mod Website:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_modWebsiteHyperlinkLabel->SetFont(m_modWebsiteHyperlinkLabel->GetFont().MakeBold());
	m_modWebsiteHyperlink = WXUtilities::createHyperlink(this, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Mod Website");
	m_modWebsiteHyperlink->Bind(wxEVT_HYPERLINK, &ModInfoPanel::onModWebsiteHyperlinkClicked, this);

	m_modRepositoryHyperlinkLabel = new wxStaticText(this, wxID_ANY, "Mod Repository:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_modRepositoryHyperlinkLabel->SetFont(m_modRepositoryHyperlinkLabel->GetFont().MakeBold());
	m_modRepositoryHyperlink = WXUtilities::createHyperlink(this, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Mod Repository");
	m_modRepositoryHyperlink->Bind(wxEVT_HYPERLINK, &ModInfoPanel::onModRepositoryHyperlinkClicked, this);

	m_notesLabel = new wxStaticText(this, wxID_ANY, "Notes:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_notesLabel->SetFont(m_notesLabel->GetFont().MakeBold());
	m_notesText = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_relatedModsLabel = new wxStaticText(this, wxID_ANY, "Related Mods:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_relatedModsLabel->SetFont(m_relatedModsLabel->GetFont().MakeBold());
	m_relatedModsPanel = new RelatedModsPanel(mods, this);
	m_relatedModSelectionRequestedConnection = m_relatedModsPanel->modSelectionRequested.connect(std::bind(&ModInfoPanel::onRelatedModSelectionRequested, this, std::placeholders::_1));

	m_similarModsLabel = new wxStaticText(this, wxID_ANY, "Similar Mods:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_similarModsLabel->SetFont(m_similarModsLabel->GetFont().MakeBold());
	m_similarModsPanel = new SimilarModsPanel(mods, this);
	m_similarModSelectionRequestedConnection = m_similarModsPanel->modSelectionRequested.connect(std::bind(&ModInfoPanel::onSimilarModSelectionRequested, this, std::placeholders::_1));

	m_teamNameLabel = new wxStaticText(this, wxID_ANY, "Team Name:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamNameLabel->SetFont(m_teamNameLabel->GetFont().MakeBold());
	m_teamNameDeepLink = WXUtilities::createDeepLink(this, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxHL_ALIGN_LEFT);
	m_teamNameDeepLink->Bind(wxEVT_HYPERLINK, &ModInfoPanel::onModTeamNameDeepLinkClicked, this);

	m_teamWebsiteHyperlinkLabel = new wxStaticText(this, wxID_ANY, "Team Website:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamWebsiteHyperlinkLabel->SetFont(m_teamWebsiteHyperlinkLabel->GetFont().MakeBold());
	m_teamWebsiteHyperlink = WXUtilities::createHyperlink(this, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Website");
	m_teamWebsiteHyperlink->Bind(wxEVT_HYPERLINK, &ModInfoPanel::onTeamWebsiteHyperlinkClicked, this);

	m_teamEmailHyperlinkLabel = new wxStaticText(this, wxID_ANY, "Team E-Mail:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamEmailHyperlinkLabel->SetFont(m_teamEmailHyperlinkLabel->GetFont().MakeBold());
	m_teamEmailHyperlink = WXUtilities::createHyperlink(this, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team E-Mail");
	m_teamEmailHyperlink->Bind(wxEVT_HYPERLINK, &ModInfoPanel::onTeamEmailHyperlinkClicked, this);

	m_teamTwitterLabel = new wxStaticText(this, wxID_ANY, "Team Twitter:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamTwitterLabel->SetFont(m_teamTwitterLabel->GetFont().MakeBold());
	m_teamTwitterHyperlink = WXUtilities::createHyperlink(this, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Twitter");
	m_teamTwitterHyperlink->Bind(wxEVT_HYPERLINK, &ModInfoPanel::onTeamTwitterHyperlinkClicked, this);

	m_teamDiscordLabel = new wxStaticText(this, wxID_ANY, "Team Discord:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamDiscordLabel->SetFont(m_teamDiscordLabel->GetFont().MakeBold());
	m_teamDiscordHyperlink = WXUtilities::createHyperlink(this, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Discord");
	m_teamDiscordHyperlink->Bind(wxEVT_HYPERLINK, &ModInfoPanel::onTeamDiscordHyperlinkClicked, this);

	m_teamLocationLabel = new wxStaticText(this, wxID_ANY, "Team Location:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamLocationLabel->SetFont(m_teamLocationLabel->GetFont().MakeBold());
	m_teamLocationText = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_teamMembersLabel = new wxStaticText(this, wxID_ANY, "Team Members:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamMembersLabel->SetFont(m_teamMembersLabel->GetFont().MakeBold());
	m_teamMembersPanel = new ModTeamMembersPanel(this);
	m_modTeamMemberSelectionRequestedConnection = m_teamMembersPanel->modTeamMemberSelectionRequested.connect(std::bind(&ModInfoPanel::onModTeamMemberSelectionRequested, this, std::placeholders::_1));

	m_downloadsLabel = new wxStaticText(this, wxID_ANY, "Original Downloads:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_downloadsLabel->SetFont(m_downloadsLabel->GetFont().MakeBold());
	m_downloadsPanel = new ModDownloadsPanel(this);

	int border = 5;

	wxFlexGridSizer * modPanelSizer = new wxFlexGridSizer(2, border, border);
	modPanelSizer->AddGrowableCol(0, 0);
	modPanelSizer->AddGrowableCol(1, 1);
	modPanelSizer->Add(m_modNameLabel, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_modNameText, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_modAliasLabel, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_modAliasDeepLink, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(modTypeLabel, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_modTypeText, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(initialReleaseDateLabel, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_initialReleaseDateText, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_latestReleaseDateLabel, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_latestReleaseDateText, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_supportedGameVersionsLabel, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_supportedGameVersionsText, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_modWebsiteHyperlinkLabel, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_modWebsiteHyperlink, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_modRepositoryHyperlinkLabel, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_modRepositoryHyperlink, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_notesLabel, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_notesText, 1, wxEXPAND | wxALL);

	modPanelSizer->Add(m_relatedModsLabel, 1, wxEXPAND | wxHORIZONTAL);

	for(int i = 0; i < 2; i++) {
		m_relatedModsSpacers[i] = modPanelSizer->Add(new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER), 1, wxEXPAND | wxHORIZONTAL)->GetWindow();
	}

	modPanelSizer->Add(m_relatedModsPanel, 1, wxEXPAND | wxHORIZONTAL);

	modPanelSizer->Add(m_teamNameLabel, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_teamNameDeepLink, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_teamWebsiteHyperlinkLabel, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_teamWebsiteHyperlink, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_teamEmailHyperlinkLabel, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_teamEmailHyperlink, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_teamTwitterLabel, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_teamTwitterHyperlink, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_teamDiscordLabel, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_teamDiscordHyperlink, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_teamLocationLabel, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_teamLocationText, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_teamMembersLabel, 1, wxEXPAND | wxHORIZONTAL);

	for(int i = 0; i < 2; i++) {
		m_teamMembersSpacers[i] = modPanelSizer->Add(new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER), 1, wxEXPAND | wxHORIZONTAL)->GetWindow();
	}

	modPanelSizer->Add(m_teamMembersPanel, 1, wxEXPAND | wxHORIZONTAL);
	modPanelSizer->Add(m_downloadsLabel, 1, wxEXPAND | wxHORIZONTAL);

	for(int i = 0; i < 2; i++) {
		m_downloadsSpacers[i] = modPanelSizer->Add(new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER), 1, wxEXPAND | wxHORIZONTAL)->GetWindow();
	}

	modPanelSizer->Add(m_downloadsPanel, 1, wxEXPAND | wxHORIZONTAL);

	modPanelSizer->Add(m_similarModsLabel, 1, wxEXPAND | wxHORIZONTAL);

	for(int i = 0; i < 2; i++) {
		m_similarModsSpacers[i] = modPanelSizer->Add(new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER), 1, wxEXPAND | wxHORIZONTAL)->GetWindow();
	}

	modPanelSizer->Add(m_similarModsPanel, 1, wxEXPAND | wxHORIZONTAL);

	SetSizer(modPanelSizer);
}

ModInfoPanel::~ModInfoPanel() {
	m_relatedModSelectionRequestedConnection.disconnect();
	m_similarModSelectionRequestedConnection.disconnect();
	m_modTeamMemberSelectionRequestedConnection.disconnect();
}

void ModInfoPanel::setMod(std::shared_ptr<Mod> mod) {
	if(m_mod == mod) {
		return;
	}

	m_mod = mod;

	if(m_mod == nullptr) {
		return;
	}

	m_modNameText->SetLabelText(wxString::FromUTF8(m_mod->getName()));

	const std::string & modAlias = m_mod->getAlias();

	if(!modAlias.empty() && m_mods != nullptr) {
		std::shared_ptr<Mod> aliasMod(m_mods->getModWithID(modAlias));

		if(aliasMod != nullptr) {
			m_modAliasDeepLink->SetLabelText(aliasMod->getName());
			m_modAliasDeepLink->SetURL(aliasMod->getID());
			m_modAliasLabel->Show();
			m_modAliasDeepLink->Show();
		}
		else {
			m_modAliasLabel->Hide();
			m_modAliasDeepLink->Hide();
		}
	}
	else {
		m_modAliasLabel->Hide();
		m_modAliasDeepLink->Hide();
	}

	m_modTypeText->SetLabelText(m_mod->getType());

	std::optional<Date> optionalInitialReleaseDate(m_mod->getInitialReleaseDate());
	std::optional<Date> optionalLatestReleaseDate(m_mod->getLatestReleaseDate());

	m_initialReleaseDateText->SetLabelText(optionalInitialReleaseDate.has_value() ? optionalInitialReleaseDate->toString() : "N/A");

	if(optionalInitialReleaseDate == optionalLatestReleaseDate) {
		m_latestReleaseDateLabel->Hide();
		m_latestReleaseDateText->Hide();
	}
	else {
		m_latestReleaseDateLabel->Show();
		m_latestReleaseDateText->Show();

		m_latestReleaseDateText->SetLabelText(optionalLatestReleaseDate.has_value() ? optionalLatestReleaseDate->toString() : "N/A");
	}

	if(m_gameVersions != nullptr) {
		std::vector<std::string> supportedGameVersionLongNames(m_mod->getSupportedGameVersionLongNames(*m_gameVersions));
		std::stringstream supportedGameVersionLongNamesStream;

		for(const std::string & gameVersionLongName : supportedGameVersionLongNames) {
			if(supportedGameVersionLongNamesStream.tellp() != 0) {
				supportedGameVersionLongNamesStream << "\n";
			}

			supportedGameVersionLongNamesStream << gameVersionLongName;
		}

		if(m_mod->isStandAlone()) {
			m_supportedGameVersionsLabel->Hide();
			m_supportedGameVersionsText->Hide();
		}
		else {
			m_supportedGameVersionsText->SetLabelText(wxString::FromUTF8(supportedGameVersionLongNamesStream.str()));
			m_supportedGameVersionsLabel->Show();
			m_supportedGameVersionsText->Show();
		}
	}
	else {
		m_supportedGameVersionsLabel->Hide();
		m_supportedGameVersionsText->Hide();
	}

	const std::string & modWebsite = m_mod->getWebsite();

	if(!modWebsite.empty()) {
		m_modWebsiteHyperlinkLabel->Show();
		m_modWebsiteHyperlink->SetLabelText(modWebsite);
		m_modWebsiteHyperlink->SetURL(modWebsite);
		m_modWebsiteHyperlink->Show();
	}
	else {
		m_modWebsiteHyperlinkLabel->Hide();
		m_modWebsiteHyperlink->Hide();
	}

	const std::string & modRepositoryURL = m_mod->getRepositoryURL();

	if(!modRepositoryURL.empty()) {
		m_modRepositoryHyperlinkLabel->Show();
		m_modRepositoryHyperlink->SetLabelText(modRepositoryURL);
		m_modRepositoryHyperlink->SetURL(modRepositoryURL);
		m_modRepositoryHyperlink->Show();
	}
	else {
		m_modRepositoryHyperlinkLabel->Hide();
		m_modRepositoryHyperlink->Hide();
	}

	if(m_mod->hasTeam()) {
		std::shared_ptr<ModTeam> team(m_mod->getTeam());

		const std::string & teamName = team->getName();

		if(!teamName.empty()) {
			m_teamNameDeepLink->SetLabelText(teamName);
			m_teamNameDeepLink->SetURL(teamName);
			m_teamNameLabel->Show();
			m_teamNameDeepLink->Show();
		}
		else {
			m_teamNameLabel->Hide();
			m_teamNameDeepLink->Hide();
		}

		const std::string & teamWebsite = team->getWebsite();

		if(!teamWebsite.empty()) {
			m_teamWebsiteHyperlink->SetLabelText(teamWebsite);
			m_teamWebsiteHyperlink->SetURL(teamWebsite);

			m_teamWebsiteHyperlinkLabel->Show();
			m_teamWebsiteHyperlink->Show();
		}
		else {
			m_teamWebsiteHyperlinkLabel->Hide();
			m_teamWebsiteHyperlink->Hide();
		}

		const std::string & teamEmail = team->getEmail();

		if(!teamEmail.empty()) {
			m_teamEmailHyperlink->SetLabelText(teamEmail);
			m_teamEmailHyperlink->SetURL("mailto:" + teamEmail);

			m_teamEmailHyperlinkLabel->Show();
			m_teamEmailHyperlink->Show();
		}
		else {
			m_teamEmailHyperlinkLabel->Hide();
			m_teamEmailHyperlink->Hide();
		}

		const std::string & teamTwitter = team->getTwitter();

		if(!teamTwitter.empty()) {
			m_teamTwitterHyperlink->SetLabelText(teamTwitter);
			m_teamTwitterHyperlink->SetURL("https://twitter.com/" + teamTwitter);

			m_teamTwitterLabel->Show();
			m_teamTwitterHyperlink->Show();
		}
		else {
			m_teamTwitterLabel->Hide();
			m_teamTwitterHyperlink->Hide();
		}

		const std::string & teamDiscord = team->getDiscord();

		if(!teamDiscord.empty()) {
			m_teamDiscordHyperlink->SetLabelText(teamDiscord);
			m_teamDiscordHyperlink->SetURL(teamDiscord);

			m_teamDiscordLabel->Show();
			m_teamDiscordHyperlink->Show();
		}
		else {
			m_teamDiscordLabel->Hide();
			m_teamDiscordHyperlink->Hide();
		}

		if(team->getLocation().hasValue()) {
			m_teamLocationText->SetLabelText(wxString::FromUTF8(team->getLocation().getDetails()));

			m_teamLocationLabel->Show();
			m_teamLocationText->Show();
		}
		else {
			m_teamLocationLabel->Hide();
			m_teamLocationText->Hide();
		}

		if(team->numberOfMembers() != 0) {
			m_teamMembersPanel->setTeam(team);
			m_teamMembersLabel->Show();
			m_teamMembersPanel->Show();
		}
		else {
			m_teamMembersLabel->Hide();
			m_teamMembersPanel->Hide();
		}

		for(int i = 0; i < 2; i++) {
			m_teamMembersSpacers[i]->Show();
		}
	}
	else {
		m_teamNameLabel->Hide();
		m_teamNameDeepLink->Hide();
		m_teamWebsiteHyperlinkLabel->Hide();
		m_teamWebsiteHyperlink->Hide();
		m_teamEmailHyperlinkLabel->Hide();
		m_teamEmailHyperlink->Hide();
		m_teamTwitterLabel->Hide();
		m_teamTwitterHyperlink->Hide();
		m_teamDiscordLabel->Hide();
		m_teamDiscordHyperlink->Hide();
		m_teamLocationLabel->Hide();
		m_teamLocationText->Hide();
		m_teamMembersLabel->Hide();
		m_teamMembersPanel->Hide();

		for(int i = 0; i < 2; i++) {
			m_teamMembersSpacers[i]->Hide();
		}
	}

	if(m_mod->numberOfNotes() != 0) {
		std::stringstream notesStream;

		for(size_t i = 0; i < m_mod->numberOfNotes(); i++) {
			if(notesStream.tellp() != 0) {
				notesStream << '\n';
			}

			notesStream << " - " << m_mod->getNote(i);
		}

		m_notesText->SetLabelText(wxString::FromUTF8(notesStream.str()));

		m_notesText->Wrap(m_notesText->GetClientSize().GetWidth());
		FitInside();

		m_notesLabel->Show();
		m_notesText->Show();
	}
	else {
		m_notesLabel->Hide();
		m_notesText->Hide();
	}

	if(m_mod->numberOfRelatedMods() != 0) {
		m_relatedModsPanel->setMod(m_mod);
		m_relatedModsLabel->Show();
		m_relatedModsPanel->Show();

		for(int i = 0; i < 2; i++) {
			m_relatedModsSpacers[i]->Show();
		}
	}
	else {
		m_relatedModsLabel->Hide();
		m_relatedModsPanel->Hide();

		for(int i = 0; i < 2; i++) {
			m_relatedModsSpacers[i]->Hide();
		}
	}

	if(m_mod->numberOfSimilarMods() != 0) {
		m_similarModsPanel->setMod(m_mod);
		m_similarModsLabel->Show();
		m_similarModsPanel->Show();

		for(int i = 0; i < 2; i++) {
			m_similarModsSpacers[i]->Show();
		}
	}
	else {
		m_similarModsLabel->Hide();
		m_similarModsPanel->Hide();

		for(int i = 0; i < 2; i++) {
			m_similarModsSpacers[i]->Hide();
		}
	}

	if(m_mod->numberOfDownloads() != 0) {
		m_downloadsPanel->setMod(m_mod);
		m_downloadsLabel->Show();
		m_downloadsPanel->Show();

		for(int i = 0; i < 2; i++) {
			m_downloadsSpacers[i]->Show();
		}
	}
	else {
		m_downloadsLabel->Hide();
		m_downloadsPanel->Hide();

		for(int i = 0; i < 2; i++) {
			m_downloadsSpacers[i]->Hide();
		}
	}

	Layout();
}

void ModInfoPanel::onModAliasDeepLinkClicked(wxHyperlinkEvent & event) {
	event.Skip(false);

	modSelectionRequested(m_mod->getAlias());

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["modID"] = m_mod->getID();
		properties["modName"] = m_mod->getName();

		SegmentAnalytics::getInstance()->track("Mod Alias Deep Link Clicked", properties);
	}
}

void ModInfoPanel::onModTeamNameDeepLinkClicked(wxHyperlinkEvent & event) {
	event.Skip(false);

	modTeamSelectionRequested(m_mod->getTeam()->getName());

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["modID"] = m_mod->getID();
		properties["modName"] = m_mod->getName();
		properties["teamName"] = m_mod->getTeam()->getName();

		SegmentAnalytics::getInstance()->track("Team Deep Link Clicked", properties);
	}
}

void ModInfoPanel::onModWebsiteHyperlinkClicked(wxHyperlinkEvent & event) {
	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["modID"] = m_mod->getID();
		properties["modName"] = m_mod->getName();
		properties["url"] = m_mod->getWebsite();

		SegmentAnalytics::getInstance()->track("Mod Website Hyperlink Clicked", properties);
	}
}

void ModInfoPanel::onModRepositoryHyperlinkClicked(wxHyperlinkEvent & event) {
	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["modID"] = m_mod->getID();
		properties["modName"] = m_mod->getName();
		properties["url"] = m_mod->getRepositoryURL();

		SegmentAnalytics::getInstance()->track("Mod Repository Hyperlink Clicked", properties);
	}
}

void ModInfoPanel::onTeamWebsiteHyperlinkClicked(wxHyperlinkEvent & event) {
	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["modID"] = m_mod->getID();
		properties["modName"] = m_mod->getName();
		properties["url"] = m_mod->getTeam()->getWebsite();

		SegmentAnalytics::getInstance()->track("Team Website Hyperlink Clicked", properties);
	}
}

void ModInfoPanel::onTeamEmailHyperlinkClicked(wxHyperlinkEvent & event) {
	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["modID"] = m_mod->getID();
		properties["modName"] = m_mod->getName();
		properties["teamEmail"] = m_mod->getTeam()->getEmail();
		properties["url"] = std::string(event.GetURL().mb_str());

		SegmentAnalytics::getInstance()->track("Team Email Hyperlink Clicked", properties);
	}
}

void ModInfoPanel::onTeamTwitterHyperlinkClicked(wxHyperlinkEvent & event) {
	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["modID"] = m_mod->getID();
		properties["modName"] = m_mod->getName();
		properties["teamTwitter"] = m_mod->getTeam()->getTwitter();
		properties["url"] = std::string(event.GetURL().mb_str());

		SegmentAnalytics::getInstance()->track("Team Twitter Hyperlink Clicked", properties);
	}
}

void ModInfoPanel::onTeamDiscordHyperlinkClicked(wxHyperlinkEvent & event) {
	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["modID"] = m_mod->getID();
		properties["modName"] = m_mod->getName();
		properties["url"] = std::string(event.GetURL().mb_str());

		SegmentAnalytics::getInstance()->track("Team Discord Hyperlink Clicked", properties);
	}
}

void ModInfoPanel::onRelatedModSelectionRequested(const std::string & relatedModID) {
	modSelectionRequested(relatedModID);
}

void ModInfoPanel::onSimilarModSelectionRequested(const std::string & similarModID) {
	modSelectionRequested(similarModID);
}

void ModInfoPanel::onModTeamMemberSelectionRequested(const std::string & modTeamMemberName) {
	modTeamMemberSelectionRequested(modTeamMemberName);
}
