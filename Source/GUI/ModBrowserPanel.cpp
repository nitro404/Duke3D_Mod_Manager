#include "ModBrowserPanel.h"

#include "Game/GameVersion.h"
#include "Game/GameVersionCollection.h"
#include "Manager/ModManager.h"
#include "Manager/ModMatch.h"
#include "Mod/Mod.h"
#include "Mod/ModAuthorInformation.h"
#include "Mod/ModGameVersion.h"
#include "Mod/ModTeam.h"
#include "Mod/ModTeamMember.h"
#include "Mod/ModVersion.h"
#include "Mod/ModVersionType.h"
#include "WXUtilities.h"

#include <spdlog/spdlog.h>
#include <wx/choicdlg.h>
#include <wx/gbsizer.h>
#include <wx/listbox.h>

#include <sstream>

ModBrowserPanel::ModBrowserPanel(std::shared_ptr<ModManager> modManager, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Mod Browser")
	, m_modManager(modManager)
	, m_modSearchTextField(nullptr)
	, m_selectRandomModButton(nullptr)
	, m_modListFilterTypeComboBox(nullptr)
	, m_modListSortTypeComboBox(nullptr)
	, m_modListSortDirectionComboBox(nullptr)
	, m_clearButton(nullptr)
	, m_modListLabel(nullptr)
	, m_modListBox(nullptr)
	, m_modVersionListLabel(nullptr)
	, m_modVersionListBox(nullptr)
	, m_modVersionTypeListLabel(nullptr)
	, m_modVersionTypeListBox(nullptr)
	, m_modGameVersionListLabel(nullptr)
	, m_modGameVersionListBox(nullptr)
	, m_modInfoBox(nullptr)
	, m_modInfoPanel(nullptr)
	, m_modNameText(nullptr)
	, m_modTypeText(nullptr)
	, m_initialReleaseDateText(nullptr)
	, m_latestReleaseDateLabel(nullptr)
	, m_latestReleaseDateText(nullptr)
	, m_supportedGameVersionsText(nullptr)
	, m_modWebsiteHyperlinkLabel(nullptr)
	, m_modWebsiteHyperlink(nullptr)
	, m_teamWebsiteHyperlinkLabel(nullptr)
	, m_teamWebsiteHyperlink(nullptr)
	, m_teamEmailHyperlinkLabel(nullptr)
	, m_teamEmailHyperlink(nullptr)
	, m_teamLocationLabel(nullptr)
	, m_teamLocationText(nullptr)
	, m_notesLabel(nullptr)
	, m_notesText(nullptr)
	, m_teamMembersLabel(nullptr)
	, m_teamMembersText(nullptr)
	, m_gameOptionsPanel(nullptr)
	, m_ipAddressTextField(nullptr)
	, m_portLabel(nullptr)
	, m_portTextField(nullptr)
	, m_preferredDOSBoxVersionComboBox(nullptr)
	, m_modGameTypeComboBox(nullptr)
	, m_preferredGameVersionComboBox(nullptr)
	, m_launchModButton(nullptr) {
	m_modManager->addListener(*this);
	m_modManager->getDOSBoxVersions()->addListener(*this);
	m_modManager->getOrganizedMods()->addListener(*this);
	m_modManager->getGameVersions()->addListener(*this);

	wxPanel * modListOptionsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "Mod List Options");

	m_modSearchTextField = new wxSearchCtrl(modListOptionsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Mod Search");
	m_modSearchTextField->Bind(wxEVT_TEXT, &ModBrowserPanel::onModSearchTextChanged, this);
	m_modSearchTextField->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &ModBrowserPanel::onModSearchCancelled, this);

	m_selectRandomModButton = new wxButton(modListOptionsPanel, wxID_ANY, "Random", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Select Random Mod");
	m_selectRandomModButton->Bind(wxEVT_BUTTON, &ModBrowserPanel::onSelectRandomModButtonPressed, this);

	if(m_modManager->getOrganizedMods()->numberOfMods() == 0) {
		m_selectRandomModButton->Disable();
	}

	wxStaticText * modListFilterTypeLabel = new wxStaticText(modListOptionsPanel, wxID_ANY, "Filter Type:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	modListFilterTypeLabel->SetFont(modListFilterTypeLabel->GetFont().MakeBold());
	m_modListFilterTypeComboBox = new wxComboBox(modListOptionsPanel, wxID_ANY, std::string(magic_enum::enum_name(OrganizedModCollection::DEFAULT_FILTER_TYPE)), wxDefaultPosition, wxDefaultSize, WXUtilities::createEnumWXArrayString<OrganizedModCollection::FilterType>(), 0, wxDefaultValidator, "Mod List Filter Type");
	m_modListFilterTypeComboBox->SetEditable(false);
	m_modListFilterTypeComboBox->Bind(wxEVT_COMBOBOX, &ModBrowserPanel::onModListFilterTypeSelected, this);

	wxStaticText * modListSortTypeLabel = new wxStaticText(modListOptionsPanel, wxID_ANY, "Sort Type:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	modListSortTypeLabel->SetFont(modListSortTypeLabel->GetFont().MakeBold());
	m_modListSortTypeComboBox = new wxComboBox(modListOptionsPanel, wxID_ANY, std::string(magic_enum::enum_name(OrganizedModCollection::DEFAULT_SORT_TYPE)), wxDefaultPosition, wxDefaultSize, WXUtilities::createEnumWXArrayString<OrganizedModCollection::SortType>(), 0, wxDefaultValidator, "Mod List Sort Type");
	m_modListSortTypeComboBox->SetEditable(false);
	m_modListSortTypeComboBox->Bind(wxEVT_COMBOBOX, &ModBrowserPanel::onModListSortTypeSelected, this);

	wxStaticText * modListSortDirectionLabel = new wxStaticText(modListOptionsPanel, wxID_ANY, "Direction:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	modListSortDirectionLabel->SetFont(modListSortDirectionLabel->GetFont().MakeBold());
	m_modListSortDirectionComboBox = new wxComboBox(modListOptionsPanel, wxID_ANY, std::string(magic_enum::enum_name(OrganizedModCollection::DEFAULT_SORT_DIRECTION)), wxDefaultPosition, wxDefaultSize, WXUtilities::createEnumWXArrayString<OrganizedModCollection::SortDirection>(), 0, wxDefaultValidator, "Mod List Sort Direction");
	m_modListSortDirectionComboBox->SetEditable(false);
	m_modListSortDirectionComboBox->Bind(wxEVT_COMBOBOX, &ModBrowserPanel::onModListSortDirectionSelected, this);

	m_clearButton = new wxButton(modListOptionsPanel, wxID_ANY, "Clear", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Clear");
	m_clearButton->Disable();
	m_clearButton->Bind(wxEVT_BUTTON, &ModBrowserPanel::onClearButtonPressed, this);

	wxPanel * modSelectionPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "Mod Selection");

	m_modListLabel = new wxStaticText(modSelectionPanel, wxID_ANY, "Mods", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_modListLabel->SetFont(m_modListLabel->GetFont().MakeBold());
	m_modListBox = new wxListBox(modSelectionPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, WXUtilities::createItemWXArrayString(m_modManager->getOrganizedMods()->getOrganizedItemDisplayNames()), wxLB_SINGLE | wxLB_ALWAYS_SB);
	m_modListBox->Bind(wxEVT_LISTBOX, &ModBrowserPanel::onModSelected, this);

	m_modVersionListLabel = new wxStaticText(modSelectionPanel, wxID_ANY, "Mod Versions", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_modVersionListLabel->SetFont(m_modVersionListLabel->GetFont().MakeBold());
	m_modVersionListBox = new wxListBox(modSelectionPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, {}, wxLB_SINGLE | wxLB_ALWAYS_SB);
	m_modVersionListBox->Bind(wxEVT_LISTBOX, &ModBrowserPanel::onModVersionSelected, this);

	m_modVersionTypeListLabel = new wxStaticText(modSelectionPanel, wxID_ANY, "Mod Version Types", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_modVersionTypeListLabel->SetFont(m_modVersionTypeListLabel->GetFont().MakeBold());
	m_modVersionTypeListBox = new wxListBox(modSelectionPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, {}, wxLB_SINGLE | wxLB_ALWAYS_SB);
	m_modVersionTypeListBox->Bind(wxEVT_LISTBOX, &ModBrowserPanel::onModVersionTypeSelected, this);

	m_modGameVersionListLabel = new wxStaticText(modSelectionPanel, wxID_ANY, "Mod Game Versions", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_modGameVersionListLabel->SetFont(m_modGameVersionListLabel->GetFont().MakeBold());
	m_modGameVersionListBox = new wxListBox(modSelectionPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, {}, wxLB_SINGLE | wxLB_ALWAYS_SB);
	m_modGameVersionListBox->Bind(wxEVT_LISTBOX, &ModBrowserPanel::onModGameVersionSelected, this);

	m_modInfoBox = new wxStaticBox(modSelectionPanel, wxID_ANY, "Mod Information", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Mod Information");
	m_modInfoBox->SetOwnFont(m_modInfoBox->GetFont().MakeBold());

	m_modInfoPanel = new wxScrolledWindow(m_modInfoBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "Mod Information");
	m_modInfoPanel->SetScrollRate(5, 5);
	m_modInfoPanel->Hide();

	wxStaticText * modNameLabel = new wxStaticText(m_modInfoPanel, wxID_ANY, "Mod Name:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	modNameLabel->SetFont(modNameLabel->GetFont().MakeBold());
	m_modNameText = new wxStaticText(m_modInfoPanel, wxID_ANY, "N/A", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	wxStaticText * modTypeLabel = new wxStaticText(m_modInfoPanel, wxID_ANY, "Mod Type:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	modTypeLabel->SetFont(modTypeLabel->GetFont().MakeBold());
	m_modTypeText = new wxStaticText(m_modInfoPanel, wxID_ANY, "N/A", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	wxStaticText * initialReleaseDateLabel = new wxStaticText(m_modInfoPanel, wxID_ANY, "Initial Release Date:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	initialReleaseDateLabel->SetFont(initialReleaseDateLabel->GetFont().MakeBold());
	m_initialReleaseDateText = new wxStaticText(m_modInfoPanel, wxID_ANY, "N/A", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_latestReleaseDateLabel = new wxStaticText(m_modInfoPanel, wxID_ANY, "Latest Release Date:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_latestReleaseDateLabel->SetFont(m_latestReleaseDateLabel->GetFont().MakeBold());
	m_latestReleaseDateText = new wxStaticText(m_modInfoPanel, wxID_ANY, "N/A", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	wxStaticText * supportedGameVersionsLabel = new wxStaticText(m_modInfoPanel, wxID_ANY, "Supported Game Versions:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	supportedGameVersionsLabel->SetFont(supportedGameVersionsLabel->GetFont().MakeBold());
	m_supportedGameVersionsText = new wxStaticText(m_modInfoPanel, wxID_ANY, "N/A", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_modWebsiteHyperlinkLabel = new wxStaticText(m_modInfoPanel, wxID_ANY, "Mod Website:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_modWebsiteHyperlinkLabel->SetFont(m_modWebsiteHyperlinkLabel->GetFont().MakeBold());
	m_modWebsiteHyperlink = new wxHyperlinkCtrl(m_modInfoPanel, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Mod Website");

	m_notesLabel = new wxStaticText(m_modInfoPanel, wxID_ANY, "Notes:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_notesLabel->SetFont(m_notesLabel->GetFont().MakeBold());
	m_notesText = new wxStaticText(m_modInfoPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_teamWebsiteHyperlinkLabel = new wxStaticText(m_modInfoPanel, wxID_ANY, "Team Website:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamWebsiteHyperlinkLabel->SetFont(m_teamWebsiteHyperlinkLabel->GetFont().MakeBold());
	m_teamWebsiteHyperlink = new wxHyperlinkCtrl(m_modInfoPanel, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team Website");

	m_teamEmailHyperlinkLabel = new wxStaticText(m_modInfoPanel, wxID_ANY, "Team E-Mail:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamEmailHyperlinkLabel->SetFont(m_teamEmailHyperlinkLabel->GetFont().MakeBold());
	m_teamEmailHyperlink = new wxHyperlinkCtrl(m_modInfoPanel, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Team E-Mail");

	m_teamLocationLabel = new wxStaticText(m_modInfoPanel, wxID_ANY, "Team Location:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamLocationLabel->SetFont(m_teamLocationLabel->GetFont().MakeBold());
	m_teamLocationText = new wxStaticText(m_modInfoPanel, wxID_ANY, "N/A", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_teamMembersLabel = new wxStaticText(m_modInfoPanel, wxID_ANY, "Team Members:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_teamMembersLabel->SetFont(m_teamMembersLabel->GetFont().MakeBold());

	m_teamMembersText = new wxStaticText(m_modInfoPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_gameOptionsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "Game Options");

	wxStaticText * ipAddressLabel = new wxStaticText(m_gameOptionsPanel, wxID_ANY, "IP Address:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	ipAddressLabel->SetFont(ipAddressLabel->GetFont().MakeBold());
	m_ipAddressTextField = new wxTextCtrl(m_gameOptionsPanel, wxID_ANY, m_modManager->getDOSBoxServerIPAddress(), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Server IP Address");
	m_ipAddressTextField->Bind(wxEVT_TEXT, &ModBrowserPanel::onIPAddressTextChanged, this);

	if(m_modManager->getGameType() == GameType::Client) {
		m_ipAddressTextField->Enable();
	}
	else {
		m_ipAddressTextField->Disable();
	}

	std::optional<uint16_t> optionalDOSBoxServerPort;
	std::string portLabelCaption;

	switch(m_modManager->getGameType()) {
		case GameType::Game:
		case GameType::Setup: {
			portLabelCaption = "Port";
			break;
		}

		case GameType::Client: {
			portLabelCaption = "Remote Port";
			optionalDOSBoxServerPort = m_modManager->getDOSBoxRemoteServerPort();
			break;
		}

		case GameType::Server: {
			portLabelCaption = "Local Port";
			optionalDOSBoxServerPort = m_modManager->getDOSBoxLocalServerPort();
			break;
		}
	}

	m_portLabel = new wxStaticText(m_gameOptionsPanel, wxID_ANY, fmt::format("{}:", portLabelCaption), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_portLabel->SetFont(m_portLabel->GetFont().MakeBold());
	m_portTextField = new wxTextCtrl(m_gameOptionsPanel, wxID_ANY, optionalDOSBoxServerPort.has_value() ? std::to_string(optionalDOSBoxServerPort.value()) : "", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Server Port");
	m_portTextField->SetMaxSize(wxSize(50, m_portTextField->GetMaxHeight()));
	m_portTextField->Bind(wxEVT_TEXT, &ModBrowserPanel::onPortTextChanged, this);

	if(m_modManager->getGameType() == GameType::Client || m_modManager->getGameType() == GameType::Server) {
		m_portTextField->Enable();
	}
	else {
		m_portTextField->Disable();
	}

	std::shared_ptr<DOSBoxVersion> preferredDOSBoxVersion(m_modManager->getPreferredDOSBoxVersion());

	wxStaticText * preferredDOSBoxVersionLabel = new wxStaticText(m_gameOptionsPanel, wxID_ANY, "DOSBox:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	preferredDOSBoxVersionLabel->SetFont(preferredDOSBoxVersionLabel->GetFont().MakeBold());
	m_preferredDOSBoxVersionComboBox = new wxComboBox(m_gameOptionsPanel, wxID_ANY, preferredDOSBoxVersion == nullptr ? "" : preferredDOSBoxVersion->getName(), wxDefaultPosition, wxDefaultSize, WXUtilities::createItemWXArrayString(m_modManager->getDOSBoxVersions()->getDOSBoxVersionDisplayNames(false)), 0, wxDefaultValidator, "DOSBox Versions");
	m_preferredDOSBoxVersionComboBox->SetEditable(false);
	m_preferredDOSBoxVersionComboBox->Bind(wxEVT_COMBOBOX, &ModBrowserPanel::onPreferredDOSBoxVersionSelected, this);

	wxStaticText * modGameTypeLabel = new wxStaticText(m_gameOptionsPanel, wxID_ANY, "Game Type:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	modGameTypeLabel->SetFont(modGameTypeLabel->GetFont().MakeBold());
	m_modGameTypeComboBox = new wxComboBox(m_gameOptionsPanel, wxID_ANY, std::string(magic_enum::enum_name(m_modManager->getGameType())), wxDefaultPosition, wxDefaultSize, WXUtilities::createEnumWXArrayString<GameType>(), 0, wxDefaultValidator, "Mod Game Type");
	m_modGameTypeComboBox->SetEditable(false);
	m_modGameTypeComboBox->Bind(wxEVT_COMBOBOX, &ModBrowserPanel::onModGameTypeSelected, this);

	std::shared_ptr<GameVersion> preferredGameVersion(m_modManager->getPreferredGameVersion());

	wxStaticText * preferredGameVersionLabel = new wxStaticText(m_gameOptionsPanel, wxID_ANY, "Game Version:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	preferredGameVersionLabel->SetFont(preferredGameVersionLabel->GetFont().MakeBold());
	m_preferredGameVersionComboBox = new wxComboBox(m_gameOptionsPanel, wxID_ANY, preferredGameVersion == nullptr ? "" : preferredGameVersion->getName(), wxDefaultPosition, wxDefaultSize, WXUtilities::createItemWXArrayString(m_modManager->getGameVersions()->getGameVersionDisplayNames(false)), 0, wxDefaultValidator, "Mod Game Versions");
	m_preferredGameVersionComboBox->SetEditable(false);
	m_preferredGameVersionComboBox->Bind(wxEVT_COMBOBOX, &ModBrowserPanel::onPreferredGameVersionSelected, this);

	m_launchModButton = new wxButton(m_gameOptionsPanel, wxID_ANY, "Launch Mod", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Launch Mod");
	m_launchModButton->Disable();
	m_launchModButton->Bind(wxEVT_BUTTON, &ModBrowserPanel::onLaunchModButtonPressed, this);

	int border = 5;

	wxGridBagSizer * modListOptionsSizer = new wxGridBagSizer(border, border);
	modListOptionsSizer->Add(m_modSearchTextField, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	modListOptionsSizer->Add(m_selectRandomModButton, wxGBPosition(0, 1), wxGBSpan(1, 1), wxSTRETCH_NOT | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	modListOptionsSizer->Add(modListFilterTypeLabel, wxGBPosition(0, 2), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	modListOptionsSizer->Add(m_modListFilterTypeComboBox, wxGBPosition(0, 3), wxGBSpan(1, 1), wxSTRETCH_NOT | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, border);
	modListOptionsSizer->Add(modListSortTypeLabel, wxGBPosition(0, 4), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	modListOptionsSizer->Add(m_modListSortTypeComboBox, wxGBPosition(0, 5), wxGBSpan(1, 1), wxSTRETCH_NOT | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, border);
	modListOptionsSizer->Add(modListSortDirectionLabel, wxGBPosition(0, 6), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	modListOptionsSizer->Add(m_modListSortDirectionComboBox, wxGBPosition(0, 7), wxGBSpan(1, 1), wxSTRETCH_NOT | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, border);
	modListOptionsSizer->Add(m_clearButton, wxGBPosition(0, 8), wxGBSpan(1, 1), wxSTRETCH_NOT | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, border);
	modListOptionsSizer->AddGrowableRow(0, 1);
	modListOptionsSizer->AddGrowableCol(0, 6);
	modListOptionsSizer->AddGrowableCol(1, 1);
	modListOptionsSizer->AddGrowableCol(2, 1);
	modListOptionsSizer->AddGrowableCol(3, 1);
	modListOptionsSizer->AddGrowableCol(4, 1);
	modListOptionsSizer->AddGrowableCol(5, 1);
	modListOptionsSizer->AddGrowableCol(6, 1);
	modListOptionsSizer->AddGrowableCol(7, 1);
	modListOptionsSizer->AddGrowableCol(8, 1);
	modListOptionsPanel->SetSizerAndFit(modListOptionsSizer);

	wxGridBagSizer * modSelectionSizer = new wxGridBagSizer(0, 0);
	modSelectionSizer->Add(m_modListLabel, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	modSelectionSizer->Add(m_modListBox, wxGBPosition(1, 0), wxGBSpan(5, 1), wxEXPAND | wxALL, border);
	modSelectionSizer->Add(m_modVersionListLabel, wxGBPosition(0, 1), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	modSelectionSizer->Add(m_modVersionListBox, wxGBPosition(1, 1), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	modSelectionSizer->Add(m_modVersionTypeListLabel, wxGBPosition(2, 1), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	modSelectionSizer->Add(m_modVersionTypeListBox, wxGBPosition(3, 1), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	modSelectionSizer->Add(m_modGameVersionListLabel, wxGBPosition(4, 1), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	modSelectionSizer->Add(m_modGameVersionListBox, wxGBPosition(5, 1), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	modSelectionSizer->Add(m_modInfoBox, wxGBPosition(1, 2), wxGBSpan(5, 1), wxEXPAND | wxALL, border);
	modSelectionSizer->AddGrowableRow(1, 5);
	modSelectionSizer->AddGrowableRow(3, 5);
	modSelectionSizer->AddGrowableRow(5, 1);
	modSelectionSizer->AddGrowableCol(1, 1);
	modSelectionSizer->AddGrowableCol(2, 5);
	modSelectionPanel->SetSizerAndFit(modSelectionSizer);

	wxBoxSizer * modInfoSizer = new wxBoxSizer(wxHORIZONTAL);
	modInfoSizer->Add(m_modInfoPanel, 1, wxEXPAND | wxALL, 20);
	m_modInfoBox->SetSizer(modInfoSizer);

	wxFlexGridSizer * modInfoPanelSizer = new wxFlexGridSizer(2, border, border);
	modInfoPanelSizer->AddGrowableCol(0, 0);
	modInfoPanelSizer->AddGrowableCol(1, 1);
	modInfoPanelSizer->Add(modNameLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_modNameText, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(modTypeLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_modTypeText, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(initialReleaseDateLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_initialReleaseDateText, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_latestReleaseDateLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_latestReleaseDateText, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(supportedGameVersionsLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_supportedGameVersionsText, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_modWebsiteHyperlinkLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_modWebsiteHyperlink, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_notesLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_notesText, 1, wxEXPAND | wxALL);
	modInfoPanelSizer->Add(m_teamWebsiteHyperlinkLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_teamWebsiteHyperlink, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_teamEmailHyperlinkLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_teamEmailHyperlink, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_teamLocationLabel, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_teamLocationText, 1, wxEXPAND | wxHORIZONTAL);
	modInfoPanelSizer->Add(m_teamMembersLabel, 1, wxEXPAND | wxALL);
	modInfoPanelSizer->Add(m_teamMembersText, 1, wxEXPAND | wxALL);
	m_modInfoPanel->SetSizer(modInfoPanelSizer);

	wxGridBagSizer * gameOptionsSizer = new wxGridBagSizer(border, border);
	gameOptionsSizer->Add(ipAddressLabel, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	gameOptionsSizer->Add(m_ipAddressTextField, wxGBPosition(0, 1), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	gameOptionsSizer->Add(m_portLabel, wxGBPosition(0, 2), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	gameOptionsSizer->Add(m_portTextField, wxGBPosition(0, 3), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	gameOptionsSizer->Add(preferredDOSBoxVersionLabel, wxGBPosition(0, 4), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	gameOptionsSizer->Add(m_preferredDOSBoxVersionComboBox, wxGBPosition(0, 5), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	gameOptionsSizer->Add(modGameTypeLabel, wxGBPosition(0, 6), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	gameOptionsSizer->Add(m_modGameTypeComboBox, wxGBPosition(0, 7), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	gameOptionsSizer->Add(preferredGameVersionLabel, wxGBPosition(0, 8), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	gameOptionsSizer->Add(m_preferredGameVersionComboBox, wxGBPosition(0, 9), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxALIGN_CENTER_VERTICAL, border);
	gameOptionsSizer->Add(m_launchModButton, wxGBPosition(0, 10), wxGBSpan(1, 1), wxSTRETCH_NOT | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, border);
	gameOptionsSizer->AddGrowableRow(0, 1);
	gameOptionsSizer->AddGrowableCol(0, 1);
	gameOptionsSizer->AddGrowableCol(1, 1);
	gameOptionsSizer->AddGrowableCol(2, 1);
	gameOptionsSizer->AddGrowableCol(3, 1);
	gameOptionsSizer->AddGrowableCol(4, 1);
	gameOptionsSizer->AddGrowableCol(5, 1);
	gameOptionsSizer->AddGrowableCol(6, 1);
	gameOptionsSizer->AddGrowableCol(7, 1);
	gameOptionsSizer->AddGrowableCol(8, 1);
	gameOptionsSizer->AddGrowableCol(9, 1);
	gameOptionsSizer->AddGrowableCol(10, 1);
	m_gameOptionsPanel->SetSizerAndFit(gameOptionsSizer);

	wxBoxSizer * modBrowserSizer = new wxBoxSizer(wxVERTICAL);
	modBrowserSizer->Add(modListOptionsPanel, 0, wxEXPAND | wxHORIZONTAL, 0);
	modBrowserSizer->Add(modSelectionPanel, 1, wxEXPAND | wxALL, 0);
	modBrowserSizer->Add(m_gameOptionsPanel, 0, wxEXPAND | wxHORIZONTAL, 0);
	SetSizer(modBrowserSizer);
}

ModBrowserPanel::~ModBrowserPanel() {
	m_modManager->removeListener(*this);
	m_modManager->getDOSBoxVersions()->removeListener(*this);
	m_modManager->getOrganizedMods()->removeListener(*this);
	m_modManager->getGameVersions()->removeListener(*this);
}

void ModBrowserPanel::update() {
	updateModListFilterType();
	updateModListSortOptions();
	updateModGameType();
	updatePreferredDOSBoxVersion();
	updatePreferredGameVersion();
	updateModSelection();
}

void ModBrowserPanel::updateModListFilterType() {
	std::optional<uint64_t> optionalFilterTypeIndex = magic_enum::enum_index(m_modManager->getOrganizedMods()->getFilterType());

	if(optionalFilterTypeIndex.has_value()) {
		m_modListFilterTypeComboBox->SetSelection(optionalFilterTypeIndex.value());
	}
}

void ModBrowserPanel::updateModListSortOptions() {
	updateModListSortType();
	updateModListSortDirection();
}

void ModBrowserPanel::updateModListSortType() {
	std::optional<uint64_t> optionalSortTypeIndex = magic_enum::enum_index(m_modManager->getOrganizedMods()->getSortType());

	if(optionalSortTypeIndex.has_value()) {
		m_modListSortTypeComboBox->SetSelection(optionalSortTypeIndex.value());
	}
}

void ModBrowserPanel::updateModListSortDirection() {
	std::optional<uint64_t> optionalSortDirectionIndex = magic_enum::enum_index(m_modManager->getOrganizedMods()->getSortDirection());

	if(optionalSortDirectionIndex.has_value()) {
		m_modListSortDirectionComboBox->SetSelection(optionalSortDirectionIndex.value());
	}
}

void ModBrowserPanel::updateModList() {
	updateModInfo();

	std::shared_ptr<OrganizedModCollection> organizedMods(m_modManager->getOrganizedMods());

	if(m_searchQuery.empty()) {
		std::shared_ptr<Mod> mod(m_modManager->getSelectedMod());

		m_modMatches.clear();

		if(organizedMods->shouldDisplayMods()) {
			m_modListLabel->SetLabelText("Mods");
		}
		else if(organizedMods->shouldDisplayGameVersions()) {
			m_modListLabel->SetLabelText("Game Versions");
		}
		else if(organizedMods->shouldDisplayTeams()) {
			m_modListLabel->SetLabelText("Teams");
		}
		else if(organizedMods->shouldDisplayAuthors()) {
			m_modListLabel->SetLabelText("Authors");
		}

		m_modListBox->Set(WXUtilities::createItemWXArrayString(organizedMods->getOrganizedItemDisplayNames()));

		if(organizedMods->numberOfMods() == 0) {
			m_selectRandomModButton->Enable();
		}

		if(mod != nullptr) {
			m_modListBox->SetSelection(organizedMods->indexOfSelectedItem());
		}
		else {
			m_modListBox->SetSelection(wxNOT_FOUND);
		}

		m_selectRandomModButton->Enable();
	}
	else {
		m_selectRandomModButton->Disable();

		wxArrayString modMatchesArrayString;
		m_modMatches = ModManager::searchForMod(organizedMods->getOrganizedMods(), m_searchQuery);

		for(const ModMatch & modMatch : m_modMatches) {
			modMatchesArrayString.Add(modMatch.toString());
		}

		m_modListLabel->SetLabelText("Search Results");
		m_modListBox->Set(modMatchesArrayString);

		m_selectRandomModButton->Disable();
	}

	updateModVersionList();
}

void ModBrowserPanel::updateModVersionList() {
	if(m_searchQuery.empty()) {
		std::shared_ptr<Mod> mod(m_modManager->getSelectedMod());

		if(mod != nullptr && m_modManager->getOrganizedMods()->shouldDisplayMods()) {
			size_t modVersionIndex = m_modManager->getSelectedModVersionIndex();

			m_modVersionListBox->Set(WXUtilities::createItemWXArrayString(mod->getVersionDisplayNames("Default")));

			if(modVersionIndex != std::numeric_limits<size_t>::max()) {
				m_modVersionListBox->SetSelection(modVersionIndex);
			}
			else {
				m_modVersionListBox->SetSelection(wxNOT_FOUND);
			}
		}
		else {
			m_modVersionListBox->Clear();
		}
	}
	else {
		m_modVersionListBox->Clear();
	}

	updateModVersionTypeList();
}

void ModBrowserPanel::updateModVersionTypeList() {
	if(m_searchQuery.empty()) {
		std::shared_ptr<Mod> mod(m_modManager->getSelectedMod());

		if(mod != nullptr && m_modManager->getOrganizedMods()->shouldDisplayMods()) {
			size_t modVersionIndex = m_modManager->getSelectedModVersionIndex();
			size_t modVersionTypeIndex = m_modManager->getSelectedModVersionTypeIndex();

			if(modVersionIndex != std::numeric_limits<size_t>::max()) {
				m_modVersionTypeListBox->Set(WXUtilities::createItemWXArrayString(mod->getVersion(modVersionIndex)->getTypeDisplayNames("Default")));

				if(modVersionTypeIndex != std::numeric_limits<size_t>::max()) {
					m_launchModButton->Enable();
					m_modVersionTypeListBox->SetSelection(modVersionTypeIndex);
				}
				else {
					m_launchModButton->Disable();
					m_modVersionTypeListBox->SetSelection(wxNOT_FOUND);
				}
			}
			else {
				m_launchModButton->Disable();
				m_modVersionTypeListBox->Clear();
			}
		}
		else {
			m_launchModButton->Disable();
			m_modVersionTypeListBox->Clear();
		}
	}
	else {
		m_launchModButton->Disable();
		m_modVersionTypeListBox->Clear();
	}

	updateModGameVersionList();
}

void ModBrowserPanel::updateModGameVersionList() {
	if(m_searchQuery.empty()) {
		std::shared_ptr<Mod> mod(m_modManager->getSelectedMod());

		if(mod != nullptr && m_modManager->getOrganizedMods()->shouldDisplayMods()) {
			size_t modVersionIndex = m_modManager->getSelectedModVersionIndex();
			size_t modVersionTypeIndex = m_modManager->getSelectedModVersionTypeIndex();
			size_t modGameVersionIndex = m_modManager->getSelectedModGameVersionIndex();

			if(modVersionIndex != std::numeric_limits<size_t>::max()) {
				std::shared_ptr<GameVersion> selectedGameVersion(m_modManager->getSelectedGameVersion());

				if(modVersionTypeIndex != std::numeric_limits<size_t>::max() && selectedGameVersion != nullptr) {
					std::shared_ptr<ModVersionType> selectedModVersionType(mod->getVersion(modVersionIndex)->getType(modVersionTypeIndex));
					std::vector<std::string> compatibleModGameVersionNames(selectedModVersionType->getCompatibleModGameVersionNames(*selectedGameVersion));
					m_modGameVersionListBox->Set(WXUtilities::createItemWXArrayString(compatibleModGameVersionNames));

					if(modGameVersionIndex != std::numeric_limits<size_t>::max()) {
						std::shared_ptr<ModGameVersion> selectedModGameVersion(selectedModVersionType->getGameVersion(modGameVersionIndex));

						std::vector<std::string>::const_iterator compatibleModGameVersionNameIterator(std::find_if(compatibleModGameVersionNames.cbegin(), compatibleModGameVersionNames.cend(), [selectedModGameVersion](const std::string & currentModGameVersionName) {
							return Utilities::areStringsEqualIgnoreCase(selectedModGameVersion->getGameVersion(), currentModGameVersionName);
						}));

						if(compatibleModGameVersionNameIterator != compatibleModGameVersionNames.cend()) {
							m_modGameVersionListBox->SetSelection(compatibleModGameVersionNameIterator - compatibleModGameVersionNames.cbegin());
						}
						else {
							spdlog::error("Failed to find compatible mod game version name '{}' in list for mod '{}' with selected game version '{}'.", selectedModGameVersion->getGameVersion(), mod->getFullName(modVersionIndex, modVersionTypeIndex), selectedGameVersion->getName());

							m_modGameVersionListBox->SetSelection(wxNOT_FOUND);
						}
					}
					else {
						m_modGameVersionListBox->SetSelection(wxNOT_FOUND);
					}
				}
				else {
					m_modGameVersionListBox->Clear();
				}
			}
			else {
				m_modGameVersionListBox->Clear();
			}
		}
		else {
			m_modGameVersionListBox->Clear();
		}
	}
	else {
		m_modGameVersionListBox->Clear();
	}
}

void ModBrowserPanel::updateModSelection() {
	if(m_searchQuery.empty()) {
		size_t selectedItemIndex = m_modManager->getOrganizedMods()->indexOfSelectedItem();

		if(selectedItemIndex != std::numeric_limits<size_t>::max()) {
			m_modListBox->SetSelection(selectedItemIndex);
		}
		else {
			m_modListBox->SetSelection(wxNOT_FOUND);
		}
	}

	updateModInfo();
	updateModVersionList();
}

void ModBrowserPanel::updateModInfo() {
	if(m_searchQuery.empty()) {
		std::shared_ptr<Mod> mod(m_modManager->getSelectedMod());

		if(mod != nullptr && m_modManager->getOrganizedMods()->shouldDisplayMods()) {
			m_modInfoPanel->Show();
			m_modNameText->SetLabelText(mod->getName());
			m_modTypeText->SetLabelText(mod->getType());

			std::optional<Date> optionalInitialReleaseDate(mod->getInitialReleaseDate());
			std::optional<Date> optionalLatestReleaseDate(mod->getLatestReleaseDate());

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

			std::vector<std::string> supportedGameVersionNames(mod->getSupportedGameVersionNames(*m_modManager->getGameVersions()));
			std::stringstream supportedGameVersionNamesStream;

			for(const std::string & gameVersionName : supportedGameVersionNames) {
				if(supportedGameVersionNamesStream.tellp() != 0) {
					supportedGameVersionNamesStream << "\n";
				}

				supportedGameVersionNamesStream << gameVersionName;
			}

			m_supportedGameVersionsText->SetLabelText(supportedGameVersionNamesStream.str());

			const std::string & modWebsite = mod->getWebsite();

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

			if(mod->hasTeam()) {
				std::shared_ptr<ModTeam> team(mod->getTeam());
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

				if(team->hasLocation()) {
					m_teamLocationText->SetLabelText(team->getLocation());

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

					m_teamMembersText->SetLabelText(teamMemberNameStream.str());

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

			if(mod->numberOfNotes() != 0) {
				std::stringstream notesStream;

				for(size_t i = 0; i < mod->numberOfNotes(); i++) {
					if(notesStream.tellp() != 0) {
						notesStream << '\n';
					}

					notesStream << " - " << mod->getNote(i);
				}

				m_notesText->SetLabelText(notesStream.str());

				m_notesText->Wrap(m_notesText->GetClientSize().GetWidth());
				m_modInfoPanel->FitInside();

				m_notesLabel->Show();
				m_notesText->Show();
			}
			else {
				m_notesLabel->Hide();
				m_notesText->Hide();
			}

			m_modInfoBox->Layout();
		}
		else {
			m_modInfoPanel->Hide();
		}
	}
	else {
		m_modInfoPanel->Hide();
	}
}

void ModBrowserPanel::updatePreferredDOSBoxVersion() {
	std::shared_ptr<DOSBoxVersion> preferredDOSBoxVersion(m_modManager->getPreferredDOSBoxVersion());

	if(preferredDOSBoxVersion == nullptr) {
		return;
	}

	size_t preferredDOSBoxVersionIndex = m_modManager->getDOSBoxVersions()->indexOfDOSBoxVersion(*preferredDOSBoxVersion);

	if(preferredDOSBoxVersionIndex == std::numeric_limits<size_t>::max()) {
		return;
	}

	m_preferredDOSBoxVersionComboBox->SetSelection(preferredDOSBoxVersionIndex);
}

void ModBrowserPanel::updatePreferredDOSBoxVersionList() {
	m_preferredDOSBoxVersionComboBox->Set(WXUtilities::createItemWXArrayString(m_modManager->getDOSBoxVersions()->getDOSBoxVersionDisplayNames(false)));

	updatePreferredDOSBoxVersion();
}

void ModBrowserPanel::updateModGameType() {
	std::optional<uint64_t> optionalGameTypeIndex = magic_enum::enum_index(m_modManager->getGameType());

	if(optionalGameTypeIndex.has_value()) {
		m_modGameTypeComboBox->SetSelection(optionalGameTypeIndex.value());
	}

	updateDOSBoxServerSettings();
}

void ModBrowserPanel::updatePreferredGameVersion() {
	std::shared_ptr<GameVersion> preferredGameVersion(m_modManager->getPreferredGameVersion());

	if(preferredGameVersion == nullptr) {
		return;
	}

	size_t preferredGameVersionIndex = m_modManager->getGameVersions()->indexOfGameVersion(*preferredGameVersion);

	if(preferredGameVersionIndex == std::numeric_limits<size_t>::max()) {
		return;
	}

	m_preferredGameVersionComboBox->SetSelection(preferredGameVersionIndex);

	updateModGameVersionList();
}

void ModBrowserPanel::updatePreferredGameVersionList() {
	m_preferredGameVersionComboBox->Set(WXUtilities::createItemWXArrayString(m_modManager->getGameVersions()->getGameVersionDisplayNames(false)));

	updatePreferredGameVersion();
}

void ModBrowserPanel::updateDOSBoxServerSettings() {
	updateDOSBoxServerIPAddress();
	updateDOSBoxServerPort();

	m_gameOptionsPanel->Layout();
}

void ModBrowserPanel::updateDOSBoxServerIPAddress() {
	switch(m_modManager->getGameType()) {
		case GameType::Game:
		case GameType::Setup: {
			m_ipAddressTextField->Disable();
			m_portLabel->SetLabelText("Port:");
			m_portTextField->SetValue(wxEmptyString);
			m_portTextField->Disable();
			break;
		}

		case GameType::Client: {
			m_ipAddressTextField->SetValue(m_modManager->getDOSBoxServerIPAddress());
			m_ipAddressTextField->Enable();
			m_portLabel->SetLabelText("Remote Port:");
			m_portTextField->SetValue(std::to_string(m_modManager->getDOSBoxRemoteServerPort()));
			m_portTextField->Enable();
			break;
		}

		case GameType::Server: {
			m_ipAddressTextField->Disable();
			m_portLabel->SetLabelText("Local Port:");
			m_portTextField->SetValue(std::to_string(m_modManager->getDOSBoxLocalServerPort()));
			m_portTextField->Enable();
			break;
		}
	}
}

void ModBrowserPanel::updateDOSBoxServerPort() {
	switch(m_modManager->getGameType()) {
		case GameType::Game:
		case GameType::Setup: {
			m_portLabel->SetLabelText("Port:");
			m_portTextField->Disable();
			break;
		}

		case GameType::Client: {
			m_portLabel->SetLabelText("Remote Port:");
			m_portTextField->SetValue(std::to_string(m_modManager->getDOSBoxRemoteServerPort()));
			m_portTextField->Enable();
			break;
		}

		case GameType::Server: {
			m_portLabel->SetLabelText("Local Port:");
			m_portTextField->SetValue(std::to_string(m_modManager->getDOSBoxLocalServerPort()));
			m_portTextField->Enable();
			break;
		}
	}
}

void ModBrowserPanel::clear() {
	std::shared_ptr<OrganizedModCollection> organizedMods(m_modManager->getOrganizedMods());
	organizedMods->clearSelectedItems();
	organizedMods->setFilterType(OrganizedModCollection::FilterType::None);

	clearSearch();

	m_clearButton->Disable();
}

void ModBrowserPanel::clearSearch() {
	m_modMatches.clear();
	m_searchQuery.clear();
	m_modSearchTextField->Clear();
}

void ModBrowserPanel::onModSearchTextChanged(wxCommandEvent & event) {
	m_searchQuery = event.GetString();

	m_clearButton->Enable();

	updateModList();
}

void ModBrowserPanel::onModSearchCancelled(wxCommandEvent & event) {
	clearSearch();

	updateModList();
}

void ModBrowserPanel::onSelectRandomModButtonPressed(wxCommandEvent & event) {
	if(!m_searchQuery.empty()) {
		return;
	}

	m_modManager->getOrganizedMods()->selectRandomItem();
}

void ModBrowserPanel::onModListFilterTypeSelected(wxCommandEvent & event) {
	int selectedFilterTypeIndex = m_modListFilterTypeComboBox->GetSelection();

	if(selectedFilterTypeIndex != wxNOT_FOUND) {
		m_modManager->getOrganizedMods()->setFilterType(magic_enum::enum_value<OrganizedModCollection::FilterType>(selectedFilterTypeIndex));

		m_clearButton->Enable();
	}
}

void ModBrowserPanel::onModListSortTypeSelected(wxCommandEvent & event) {
	int selectedSortTypeIndex = m_modListSortTypeComboBox->GetSelection();

	if(selectedSortTypeIndex != wxNOT_FOUND) {
		m_modManager->getOrganizedMods()->setSortType(magic_enum::enum_value<OrganizedModCollection::SortType>(selectedSortTypeIndex));
	}
}

void ModBrowserPanel::onModListSortDirectionSelected(wxCommandEvent & event) {
	int selectedSortDirectionIndex = m_modListSortDirectionComboBox->GetSelection();

	if(selectedSortDirectionIndex != wxNOT_FOUND) {
		m_modManager->getOrganizedMods()->setSortDirection(magic_enum::enum_value<OrganizedModCollection::SortDirection>(selectedSortDirectionIndex));
	}
}

void ModBrowserPanel::onModSelected(wxCommandEvent & event) {
	int selectedModIndex = m_modListBox->GetSelection();

	if(selectedModIndex == wxNOT_FOUND) {
		return;
	}

	if(m_searchQuery.empty()) {
		if(!m_modManager->getOrganizedMods()->selectItem(selectedModIndex)) {
			spdlog::error("Failed to select item '{}' with index: '{}'.", std::string(event.GetString()), selectedModIndex);
		}
	}
	else {
		ModMatch modMatch(m_modMatches[selectedModIndex]);
		m_modMatches.clear();
		m_searchQuery.clear();
		m_modSearchTextField->Clear();
		updateModList();
		m_modManager->getOrganizedMods()->setSelectedMod(modMatch.getMod().get());
		m_modManager->setSelectedModFromMatch(modMatch);
	}
}

void ModBrowserPanel::onModVersionSelected(wxCommandEvent & event) {
	int selectedModVersionIndex = m_modVersionListBox->GetSelection();

	if(selectedModVersionIndex == wxNOT_FOUND) {
		return;
	}

	if(!m_searchQuery.empty()) {
		return;
	}

	if(!m_modManager->setSelectedModVersionIndex(selectedModVersionIndex)) {
		spdlog::error("Failed to select mod version at index: '{}'.", selectedModVersionIndex);
	}
}

void ModBrowserPanel::onModVersionTypeSelected(wxCommandEvent & event) {
	int selectedModVersionTypeIndex = m_modVersionTypeListBox->GetSelection();

	if(selectedModVersionTypeIndex == wxNOT_FOUND) {
		return;
	}

	if(!m_searchQuery.empty()) {
		return;
	}

	if(!m_modManager->setSelectedModVersionTypeIndex(selectedModVersionTypeIndex)) {
		spdlog::error("Failed to select mod version type at index: '{}'.", selectedModVersionTypeIndex);
	}
}

void ModBrowserPanel::onModGameVersionSelected(wxCommandEvent & event) {
	int selectedModGameVersionIndex = m_modGameVersionListBox->GetSelection();

	if(selectedModGameVersionIndex == wxNOT_FOUND) {
		return;
	}

	if(!m_searchQuery.empty()) {
		return;
	}

	if(!m_modManager->setSelectedModGameVersionIndex(selectedModGameVersionIndex)) {
		spdlog::error("Failed to select mod game version at index: '{}'.", selectedModGameVersionIndex);
	}

	m_launchModButton->Enable();
}

void ModBrowserPanel::onPreferredDOSBoxVersionSelected(wxCommandEvent & event) {
	int selectedDOSBoxVersionIndex = m_preferredDOSBoxVersionComboBox->GetSelection();

	if(selectedDOSBoxVersionIndex != wxNOT_FOUND) {
		m_modManager->setPreferredDOSBoxVersion(m_modManager->getDOSBoxVersions()->getDOSBoxVersion(selectedDOSBoxVersionIndex));
	}
}

void ModBrowserPanel::onModGameTypeSelected(wxCommandEvent & event) {
	int selectedModGameTypeIndex = m_modGameTypeComboBox->GetSelection();

	if(selectedModGameTypeIndex != wxNOT_FOUND) {
		m_modManager->setGameType(magic_enum::enum_value<GameType>(selectedModGameTypeIndex));
	}
}

void ModBrowserPanel::onPreferredGameVersionSelected(wxCommandEvent & event) {
	int selectedModGameVersionIndex = m_preferredGameVersionComboBox->GetSelection();

	if(selectedModGameVersionIndex != wxNOT_FOUND) {
		m_modManager->setPreferredGameVersion(m_modManager->getGameVersions()->getGameVersion(selectedModGameVersionIndex));
	}
}

void ModBrowserPanel::onIPAddressTextChanged(wxCommandEvent & event) {
	m_modManager->setDOSBoxServerIPAddress(event.GetString());
}

void ModBrowserPanel::onPortTextChanged(wxCommandEvent & event) {
	std::optional<uint16_t> port(Utilities::parseUnsignedShort(event.GetString()));

	if(port.has_value()) {
		switch(m_modManager->getGameType()) {
			case GameType::Game:
			case GameType::Setup: {
				break;
			}

			case GameType::Client: {
				m_modManager->setDOSBoxRemoteServerPort(port.value());
				break;
			}

			case GameType::Server: {
				m_modManager->setDOSBoxLocalServerPort(port.value());
				break;
			}
		}
	}
}

void ModBrowserPanel::onClearButtonPressed(wxCommandEvent & event) {
	clear();
}

void ModBrowserPanel::onLaunchModButtonPressed(wxCommandEvent & event) {
	if(m_modManager == nullptr || !m_modManager->isInitialized()) {
		wxMessageBox(
			"Mod manager is not initialized!",
			"Launch Failed",
			wxOK | wxICON_ERROR,
			this
		);

		return;
	}

	std::shared_ptr<Mod> selectedMod(m_modManager->getSelectedMod());

	if(m_modManager->hasModSelected()) {
		if(!m_modManager->hasModVersionSelected()) {
			wxMessageBox(
				fmt::format(
					"Failed to launch '{}', no mod version selected.",
					selectedMod->getName()
				),
				"Launch Failed",
				wxOK | wxICON_ERROR,
				this
			);

			return;
		}

		if(!m_modManager->hasModVersionTypeSelected()) {
			wxMessageBox(
				fmt::format(
					"Failed to launch '{}', no mod version type selected.",
					selectedMod->getFullName(m_modManager->getSelectedModVersionIndex())
				),
				"Launch Failed",
				wxOK | wxICON_ERROR,
				this
			);

			return;
		}
	}

	std::shared_ptr<GameVersion> alternateGameVersion;
	std::shared_ptr<ModGameVersion> alternateModGameVersion;
	size_t selectedModVersionIndex = m_modManager->getSelectedModVersionIndex();
	size_t selectedModVersionTypeIndex = m_modManager->getSelectedModVersionTypeIndex();
	std::string fullModName(selectedMod->getFullName(selectedModVersionIndex, selectedModVersionTypeIndex));
	std::shared_ptr<GameVersion> selectedGameVersion(m_modManager->getSelectedGameVersion());
	std::shared_ptr<DOSBoxVersion> selectedDOSBoxVersion(m_modManager->getSelectedDOSBoxVersion());

	if(!m_modManager->isModSupportedOnSelectedGameVersion()) {
		std::vector<std::shared_ptr<ModGameVersion>> * modGameVersions = nullptr;
		std::shared_ptr<ModVersionType> selectedModVersionType(selectedMod->getVersion(selectedModVersionIndex)->getType(selectedModVersionTypeIndex));
		std::vector<std::pair<std::shared_ptr<GameVersion>, std::vector<std::shared_ptr<ModGameVersion>>>> compatibleGameVersions(m_modManager->getGameVersions()->getGameVersionsCompatibleWith(selectedModVersionType->getGameVersions(), true, true));

		if(compatibleGameVersions.empty()) {
			wxMessageBox(
				fmt::format(
					"Failed to launch '{}', there are no game versions compatible with this mod.",
					fullModName
				),
				"Launch Failed",
				wxOK | wxICON_ERROR,
				this
			);

			return;
		}
		else if(compatibleGameVersions.size() == 1) {
			alternateGameVersion = compatibleGameVersions[0].first;
			modGameVersions = &compatibleGameVersions[0].second;

			int result = wxMessageBox(
				fmt::format(
					"{}\n"
					"Launching mod using '{}' instead!",
					selectedGameVersion != nullptr
						? fmt::format("'{}' is not supported on '{}'.", fullModName, selectedGameVersion->getName())
						: "No game version selected.",
					alternateGameVersion->getName()
				),
				"Unsupported Game Version",
				wxOK | wxCANCEL | wxICON_INFORMATION,
				this
			);

			if(result == wxCANCEL) {
				return;
			}
		}
		else {
			std::vector<std::string> compatibleGameVersionNames;

			for(std::vector<std::pair<std::shared_ptr<GameVersion>, std::vector<std::shared_ptr<ModGameVersion>>>>::const_iterator i = compatibleGameVersions.begin(); i != compatibleGameVersions.end(); ++i) {
				compatibleGameVersionNames.push_back((*i).first->getName());
			}

			int selectedCompatibleGameVersionIndex = wxGetSingleChoiceIndex(
				fmt::format("{}\nPlease choose an alternative compatible game version to run:",
					selectedGameVersion != nullptr
						? fmt::format("'{}' is not supported on '{}'.", fullModName, selectedGameVersion->getName())
						: "No game version selected."
				),
				"Choose Game Version",
				WXUtilities::createItemWXArrayString(compatibleGameVersionNames),
				0,
				this
			);

			if(selectedCompatibleGameVersionIndex == wxNOT_FOUND) {
				return;
			}

			alternateGameVersion = compatibleGameVersions[selectedCompatibleGameVersionIndex].first;
			modGameVersions = &compatibleGameVersions[selectedCompatibleGameVersionIndex].second;
		}

		if(modGameVersions->empty()) {
			return;
		}
		else if(modGameVersions->size() == 1) {
			alternateModGameVersion = (*modGameVersions)[0];
		}
		else {
			std::vector<std::string> modGameVersionNames;

			for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = modGameVersions->begin(); i != modGameVersions->end(); ++i) {
				modGameVersionNames.push_back((*i)->getFullName());
			}

			int selectedModGameVersionIndex = wxGetSingleChoiceIndex(
				fmt::format("Choose a '{}' mod game version to run:", fullModName),
				"Choose Mod Game Version",
				WXUtilities::createItemWXArrayString(modGameVersionNames),
				0,
				this
			);

			if(selectedModGameVersionIndex == wxNOT_FOUND) {
				return;
			}

			alternateModGameVersion = (*modGameVersions)[selectedModGameVersionIndex];
		}

		if(alternateGameVersion == nullptr || alternateModGameVersion == nullptr) {
			spdlog::error("Alternative game version not selected, aborting launch.");
			return;
		}

		if(selectedGameVersion != nullptr) {
			spdlog::info("Using game version '{}' since '{}' is not supported on '{}'.", alternateGameVersion->getName(), fullModName, selectedGameVersion->getName());
		}
		else {
			spdlog::info("Using game version '{}' since no game version was selected.", alternateGameVersion->getName());
		}
	}

	if(!m_modManager->runSelectedMod(alternateGameVersion, alternateModGameVersion)) {
		std::shared_ptr<GameVersion> activeGameVersion(alternateGameVersion != nullptr ? alternateGameVersion : selectedGameVersion);

		if(!activeGameVersion->isConfigured()) {
			wxMessageBox(
				fmt::format(
					"Failed to launch '{}', game version '{}' is not configured/installed.",
					selectedMod->getName(),
					activeGameVersion->getName()
				),
				"Launch Failed",
				wxOK | wxICON_ERROR,
				this
			);

			return;
		}

		if(activeGameVersion->doesRequireDOSBox() && !selectedDOSBoxVersion->isConfigured()) {
			wxMessageBox(
				fmt::format(
					"Failed to launch '{}', '{}' is not configured/installed.",
					selectedMod->getName(),
					selectedDOSBoxVersion->getName()
				),
				"Launch Failed",
				wxOK | wxICON_ERROR,
				this
			);

			return;
		}

		wxMessageBox(
			fmt::format(
				"Failed to launch '{}'{}!\n"
				"\n"
				"See console for details.",
				activeGameVersion->getName(),
				selectedMod == nullptr ? "" : fmt::format(" with mod '{}'", fullModName)
			),
			"Launch Failed",
			wxOK | wxICON_ERROR,
			this
		);
	}
}

void ModBrowserPanel::modSelectionChanged(const std::shared_ptr<Mod> & mod, size_t modVersionIndex, size_t modVersionTypeIndex, size_t modGameVersionIndex) {
	updateModSelection();

	m_clearButton->Enable();
}

void ModBrowserPanel::gameTypeChanged(GameType gameType) {
	updateModGameType();
	updateDOSBoxServerSettings();
}

void ModBrowserPanel::preferredDOSBoxVersionChanged(const std::shared_ptr<DOSBoxVersion> & dosboxVersion) {
	updatePreferredDOSBoxVersion();
}

void ModBrowserPanel::preferredGameVersionChanged(const std::shared_ptr<GameVersion> & gameVersion) {
	updatePreferredGameVersion();
}

void ModBrowserPanel::dosboxServerIPAddressChanged(const std::string & ipAddress) {
	m_ipAddressTextField->SetValue(ipAddress);
}

void ModBrowserPanel::dosboxLocalServerPortChanged(uint16_t port) {
	if(m_modManager->getGameType() == GameType::Server) {
		m_portTextField->SetValue(std::to_string(port));
	}
}

void ModBrowserPanel::dosboxRemoteServerPortChanged(uint16_t port) {
	if(m_modManager->getGameType() == GameType::Client) {
		m_portTextField->SetValue(std::to_string(port));
	}
}

void ModBrowserPanel::filterTypeChanged(OrganizedModCollection::FilterType filterType) {
	m_modManager->getOrganizedMods()->clearSelectedItems();

	updateModListFilterType();
	updateModList();
}

void ModBrowserPanel::sortOptionsChanged(OrganizedModCollection::SortType sortType, OrganizedModCollection::SortDirection sortDirection) {
	updateModListSortOptions();
	updateModList();
}

void ModBrowserPanel::selectedModChanged(const std::shared_ptr<Mod> & mod) {
	updateModSelection();
}

void ModBrowserPanel::selectedGameVersionChanged(const std::shared_ptr<GameVersion> & gameVersion) {
	updateModSelection();
}

void ModBrowserPanel::selectedTeamChanged(const std::shared_ptr<ModAuthorInformation> & team) {
	updateModSelection();
}

void ModBrowserPanel::selectedAuthorChanged(const std::shared_ptr<ModAuthorInformation> & author) {
	updateModSelection();
}

void ModBrowserPanel::organizedModCollectionChanged(const std::vector<std::shared_ptr<Mod>> & organizedMods) {
	updateModList();
}

void ModBrowserPanel::organizedModGameVersionCollectionChanged(const std::vector<std::shared_ptr<GameVersion>> & organizedMods) {
	updateModList();
}

void ModBrowserPanel::organizedModTeamCollectionChanged(const std::vector<std::shared_ptr<ModAuthorInformation>> & organizedMods) {
	updateModList();
}

void ModBrowserPanel::organizedModAuthorCollectionChanged(const std::vector<std::shared_ptr<ModAuthorInformation>> & organizedMods) {
	updateModList();
}

void ModBrowserPanel::dosboxVersionCollectionSizeChanged(DOSBoxVersionCollection & dosboxVersionCollection) {
	updatePreferredDOSBoxVersionList();
}

void ModBrowserPanel::dosboxVersionCollectionItemModified(DOSBoxVersionCollection & dosboxVersionCollection, DOSBoxVersion & dosboxVersion) {
	updatePreferredDOSBoxVersionList();
}

void ModBrowserPanel::gameVersionCollectionSizeChanged(GameVersionCollection & gameVersionCollection) {
	updatePreferredGameVersionList();
}

void ModBrowserPanel::gameVersionCollectionItemModified(GameVersionCollection & gameVersionCollection, GameVersion & gameVersion) {
	updatePreferredGameVersionList();
}
