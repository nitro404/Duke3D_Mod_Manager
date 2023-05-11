#include "ModInfoPanel.h"

#include "Mod/Mod.h"
#include "Mod/ModCollection.h"
#include "Mod/ModTeam.h"
#include "Mod/ModTeamMember.h"
#include "Game/GameVersionCollection.h"

#include <sstream>
#include <string>

ModInfoPanel::ModInfoPanel(std::shared_ptr<ModCollection> mods, std::shared_ptr<GameVersionCollection> gameVersions, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxScrolledWindow(parent, windowID, position, size, style, "Mod Information")
	, m_mods(mods)
	, m_gameVersions(gameVersions)
	, m_modNameText(nullptr)
	, m_modAliasLabel(nullptr)
	, m_modAliasText(nullptr)
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
	, m_teamNameLabel(nullptr)
	, m_teamNameText(nullptr)
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
	, m_notesLabel(nullptr)
	, m_notesText(nullptr)
	, m_teamMembersLabel(nullptr)
	, m_teamMembersText(nullptr) {
	SetScrollRate(5, 5);

	wxStaticText * modNameLabel = new wxStaticText(this, wxID_ANY, "Mod Name:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	modNameLabel->SetFont(modNameLabel->GetFont().MakeBold());
	m_modNameText = new wxStaticText(this, wxID_ANY, "N/A", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_modAliasLabel = new wxStaticText(this, wxID_ANY, "Mod Alias:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_modAliasLabel->SetFont(m_modAliasLabel->GetFont().MakeBold());
	m_modAliasText = new wxStaticText(this, wxID_ANY, "N/A", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	wxStaticText * modTypeLabel = new wxStaticText(this, wxID_ANY, "Mod Type:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	modTypeLabel->SetFont(modTypeLabel->GetFont().MakeBold());
	m_modTypeText = new wxStaticText(this, wxID_ANY, "N/A", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	wxStaticText * initialReleaseDateLabel = new wxStaticText(this, wxID_ANY, "Initial Release Date:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	initialReleaseDateLabel->SetFont(initialReleaseDateLabel->GetFont().MakeBold());
	m_initialReleaseDateText = new wxStaticText(this, wxID_ANY, "N/A", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_latestReleaseDateLabel = new wxStaticText(this, wxID_ANY, "Latest Release Date:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_latestReleaseDateLabel->SetFont(m_latestReleaseDateLabel->GetFont().MakeBold());
	m_latestReleaseDateText = new wxStaticText(this, wxID_ANY, "N/A", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_supportedGameVersionsLabel = new wxStaticText(this, wxID_ANY, "Supported Game Versions:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_supportedGameVersionsLabel->SetFont(m_supportedGameVersionsLabel->GetFont().MakeBold());
	m_supportedGameVersionsText = new wxStaticText(this, wxID_ANY, "N/A", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_modWebsiteHyperlinkLabel = new wxStaticText(this, wxID_ANY, "Mod Website:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_modWebsiteHyperlinkLabel->SetFont(m_modWebsiteHyperlinkLabel->GetFont().MakeBold());
	m_modWebsiteHyperlink = new wxHyperlinkCtrl(this, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Mod Website");

	m_modRepositoryHyperlinkLabel = new wxStaticText(this, wxID_ANY, "Mod Repository:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_modRepositoryHyperlinkLabel->SetFont(m_modRepositoryHyperlinkLabel->GetFont().MakeBold());
	m_modRepositoryHyperlink = new wxHyperlinkCtrl(this, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Mod Repository");

	m_notesLabel = new wxStaticText(this, wxID_ANY, "Notes:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_notesLabel->SetFont(m_notesLabel->GetFont().MakeBold());
	m_notesText = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_teamNameLabel = new wxStaticText(this, wxID_ANY, "Team Name:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamNameLabel->SetFont(m_teamNameLabel->GetFont().MakeBold());
	m_teamNameText = new wxStaticText(this, wxID_ANY, "N/A", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_teamWebsiteHyperlinkLabel = new wxStaticText(this, wxID_ANY, "Team Website:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamWebsiteHyperlinkLabel->SetFont(m_teamWebsiteHyperlinkLabel->GetFont().MakeBold());
	m_teamWebsiteHyperlink = new wxHyperlinkCtrl(this, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Website");

	m_teamEmailHyperlinkLabel = new wxStaticText(this, wxID_ANY, "Team E-Mail:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamEmailHyperlinkLabel->SetFont(m_teamEmailHyperlinkLabel->GetFont().MakeBold());
	m_teamEmailHyperlink = new wxHyperlinkCtrl(this, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team E-Mail");

	m_teamTwitterLabel = new wxStaticText(this, wxID_ANY, "Team Twitter:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamTwitterLabel->SetFont(m_teamTwitterLabel->GetFont().MakeBold());
	m_teamTwitterHyperlink = new wxHyperlinkCtrl(this, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Twitter");

	m_teamDiscordLabel = new wxStaticText(this, wxID_ANY, "Team Discord:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamDiscordLabel->SetFont(m_teamDiscordLabel->GetFont().MakeBold());
	m_teamDiscordHyperlink = new wxHyperlinkCtrl(this, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Discord");

	m_teamLocationLabel = new wxStaticText(this, wxID_ANY, "Team Location:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamLocationLabel->SetFont(m_teamLocationLabel->GetFont().MakeBold());
	m_teamLocationText = new wxStaticText(this, wxID_ANY, "N/A", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_teamMembersLabel = new wxStaticText(this, wxID_ANY, "Team Members:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamMembersLabel->SetFont(m_teamMembersLabel->GetFont().MakeBold());

	m_teamMembersText = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	int border = 5;

	wxFlexGridSizer * modInfoPanelSizer = new wxFlexGridSizer(2, border, border);
	modInfoPanelSizer->AddGrowableCol(0, 0);
	modInfoPanelSizer->AddGrowableCol(1, 1);
	modInfoPanelSizer->Add(modNameLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_modNameText, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_modAliasLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_modAliasText, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(modTypeLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_modTypeText, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(initialReleaseDateLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_initialReleaseDateText, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_latestReleaseDateLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_latestReleaseDateText, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_supportedGameVersionsLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_supportedGameVersionsText, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_modWebsiteHyperlinkLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_modWebsiteHyperlink, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_modRepositoryHyperlinkLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_modRepositoryHyperlink, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_notesLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_notesText, 1, wxEXPAND | wxALL);
	modInfoPanelSizer->Add(m_teamNameLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_teamNameText, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_teamWebsiteHyperlinkLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_teamWebsiteHyperlink, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_teamEmailHyperlinkLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_teamEmailHyperlink, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_teamTwitterLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_teamTwitterHyperlink, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_teamDiscordLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_teamDiscordHyperlink, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_teamLocationLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_teamLocationText, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_teamMembersLabel, 1, wxEXPAND | wxALL);
	modInfoPanelSizer->Add(m_teamMembersText, 1, wxEXPAND | wxALL);
	SetSizer(modInfoPanelSizer);
}

ModInfoPanel::~ModInfoPanel() { }

void ModInfoPanel::setMod(std::shared_ptr<Mod> mod) {
	m_mod = mod;

	if(m_mod == nullptr) {
		return;
	}

	m_modNameText->SetLabelText(wxString::FromUTF8(m_mod->getName()));

	const std::string & modAlias = m_mod->getAlias();

	if(!modAlias.empty() && m_mods != nullptr) {
		std::shared_ptr<Mod> aliasMod(m_mods->getModWithID(modAlias));

		if(aliasMod != nullptr) {
			m_modAliasText->SetLabelText(aliasMod->getName());
			m_modAliasLabel->Show();
			m_modAliasText->Show();
		}
		else {
			m_modAliasLabel->Hide();
			m_modAliasText->Hide();
		}
	}
	else {
		m_modAliasLabel->Hide();
		m_modAliasText->Hide();
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
			m_teamNameText->SetLabelText(teamName);
			m_teamNameLabel->Show();
			m_teamNameText->Show();
		}
		else {
			m_teamNameLabel->Hide();
			m_teamNameText->Hide();
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
			std::stringstream teamMemberNameStream;

			for(size_t i = 0; i < team->numberOfMembers(); i++) {
				if(teamMemberNameStream.tellp() != 0) {
					teamMemberNameStream << "\n";
				}

				teamMemberNameStream << team->getMember(i)->getName();
			}

			m_teamMembersText->SetLabelText(wxString::FromUTF8(teamMemberNameStream.str()));

			m_teamMembersLabel->Show();
			m_teamMembersText->Show();
		}
		else {
			m_teamMembersLabel->Hide();
			m_teamMembersText->Hide();
		}
	}
	else {
		m_teamWebsiteHyperlinkLabel->Hide();
		m_teamWebsiteHyperlink->Hide();
		m_teamEmailHyperlinkLabel->Hide();
		m_teamEmailHyperlink->Hide();
		m_teamLocationLabel->Hide();
		m_teamLocationText->Hide();
		m_teamMembersLabel->Hide();
		m_teamMembersText->Hide();
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

	Layout();
}