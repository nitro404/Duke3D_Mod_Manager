#include "ModBrowserPanel.h"

#include "../ProcessRunningDialog.h"
#include "../WXUtilities.h"
#include "Download/DownloadManager.h"
#include "Game/GameVersion.h"
#include "Game/GameVersionCollection.h"
#include "Info/ModInfoPanel.h"
#include "Manager/ModManager.h"
#include "Manager/ModMatch.h"
#include "Manager/SettingsManager.h"
#include "Mod/FavouriteModCollection.h"
#include "Mod/Mod.h"
#include "Mod/ModAuthorInformation.h"
#include "Mod/ModCollection.h"
#include "Mod/ModGameVersion.h"
#include "Mod/ModIdentifier.h"
#include "Mod/ModTeam.h"
#include "Mod/ModTeamMember.h"
#include "Mod/ModVersion.h"
#include "Mod/ModVersionType.h"
#include "Mod/StandAloneModCollection.h"

#include <Network/HTTPRequest.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <wx/choicdlg.h>
#include <wx/gbsizer.h>
#include <wx/listbox.h>

#include <filesystem>
#include <sstream>

wxDECLARE_EVENT(EVENT_LAUNCH_FAILED, LaunchFailedEvent);
wxDECLARE_EVENT(EVENT_GAME_PROCESS_TERMINATED, GameProcessTerminatedEvent);
wxDECLARE_EVENT(EVENT_MOD_INSTALL_PROGRESS, ModInstallProgressEvent);
wxDECLARE_EVENT(EVENT_MOD_INSTALL_DONE, ModInstallDoneEvent);

class LaunchFailedEvent final : public wxEvent {
public:
	LaunchFailedEvent()
		: wxEvent(0, EVENT_LAUNCH_FAILED) { }

	virtual ~LaunchFailedEvent() { }

	virtual wxEvent * Clone() const override {
		return new LaunchFailedEvent(*this);
	}

	DECLARE_DYNAMIC_CLASS(LaunchFailedEvent);
};

IMPLEMENT_DYNAMIC_CLASS(LaunchFailedEvent, wxEvent);

class GameProcessTerminatedEvent final : public wxEvent {
public:
	GameProcessTerminatedEvent()
		: wxEvent(0, EVENT_GAME_PROCESS_TERMINATED) { }

	virtual ~GameProcessTerminatedEvent() { }

	virtual wxEvent * Clone() const override {
		return new GameProcessTerminatedEvent(*this);
	}

	DECLARE_DYNAMIC_CLASS(GameProcessTerminatedEvent);
};

IMPLEMENT_DYNAMIC_CLASS(GameProcessTerminatedEvent, wxEvent);

class ModInstallProgressEvent final : public wxEvent {
public:
	ModInstallProgressEvent(int value = 0, const std::string & message = {})
		: wxEvent(0, EVENT_MOD_INSTALL_PROGRESS)
		, m_value(value)
		, m_message(message) { }

	virtual ~ModInstallProgressEvent() { }

	int getValue() const {
		return m_value;
	}

	void setValue(int value) {
		m_value = value;
	}

	const std::string & getMessage() const {
		return m_message;
	}

	void setMessage(const std::string & message) {
		m_message = message;
	}

	virtual wxEvent * Clone() const override {
		return new ModInstallProgressEvent(*this);
	}

	DECLARE_DYNAMIC_CLASS(ModInstallProgressEvent);

private:
	int m_value;
	std::string m_message;
};

IMPLEMENT_DYNAMIC_CLASS(ModInstallProgressEvent, wxEvent);

class ModInstallDoneEvent final : public wxEvent {
public:
	ModInstallDoneEvent(bool success = true)
		: wxEvent(0, EVENT_MOD_INSTALL_DONE)
		, m_success(success) { }

	virtual ~ModInstallDoneEvent() { }

	virtual wxEvent * Clone() const override {
		return new ModInstallDoneEvent(*this);
	}

	bool wasSuccessful() const {
		return m_success;
	}

	DECLARE_DYNAMIC_CLASS(ModInstallDoneEvent);

private:
	bool m_success;
};

IMPLEMENT_DYNAMIC_CLASS(ModInstallDoneEvent, wxEvent);

wxDEFINE_EVENT(EVENT_LAUNCH_FAILED, LaunchFailedEvent);
wxDEFINE_EVENT(EVENT_GAME_PROCESS_TERMINATED, GameProcessTerminatedEvent);
wxDEFINE_EVENT(EVENT_MOD_INSTALL_PROGRESS, ModInstallProgressEvent);
wxDEFINE_EVENT(EVENT_MOD_INSTALL_DONE, ModInstallDoneEvent);

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
	, m_modPopupMenuItemIndex(wxNOT_FOUND)
	, m_modListAddFavouriteMenuItem(nullptr)
	, m_modListRemoveFavouriteMenuItem(nullptr)
	, m_modVersionPopupMenuItemIndex(wxNOT_FOUND)
	, m_modVersionListAddFavouriteMenuItem(nullptr)
	, m_modVersionTypePopupMenuItemIndex(wxNOT_FOUND)
	, m_modVersionTypeListAddFavouriteMenuItem(nullptr)
	, m_modInfoBox(nullptr)
	, m_modInfoPanel(nullptr)
	, m_gameOptionsPanel(nullptr)
	, m_ipAddressTextField(nullptr)
	, m_portLabel(nullptr)
	, m_portTextField(nullptr)
	, m_preferredDOSBoxVersionComboBox(nullptr)
	, m_modGameTypeComboBox(nullptr)
	, m_preferredGameVersionComboBox(nullptr)
	, m_uninstallButton(nullptr)
	, m_launchButton(nullptr)
	, m_gameRunningDialog(nullptr)
	, m_modInstallationCancelled(false)
	, m_installModProgressDialog(nullptr) {
	std::shared_ptr<OrganizedModCollection> organizedMods(m_modManager->getOrganizedMods());
	std::shared_ptr<DOSBoxVersionCollection> dosboxVersions(m_modManager->getDOSBoxVersions());
	std::shared_ptr<GameVersionCollection> gameVersions(m_modManager->getGameVersions());
	m_modSelectionChangedConnection = m_modManager->modSelectionChanged.connect(std::bind(&ModBrowserPanel::onModSelectionChanged, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	m_gameTypeChangedConnection = m_modManager->gameTypeChanged.connect(std::bind(&ModBrowserPanel::onGameTypeChanged, this, std::placeholders::_1));
	m_preferredDOSBoxVersionChangedConnection = m_modManager->preferredDOSBoxVersionChanged.connect(std::bind(&ModBrowserPanel::onPreferredDOSBoxVersionChanged, this, std::placeholders::_1));
	m_preferredGameVersionChangedConnection = m_modManager->preferredGameVersionChanged.connect(std::bind(&ModBrowserPanel::onPreferredGameVersionChanged, this, std::placeholders::_1));
	m_dosboxServerIPAddressChangedConnection = m_modManager->dosboxServerIPAddressChanged.connect(std::bind(&ModBrowserPanel::onDOSBoxServerIPAddressChanged, this, std::placeholders::_1));
	m_dosboxLocalServerPortChangedConnection = m_modManager->dosboxLocalServerPortChanged.connect(std::bind(&ModBrowserPanel::onDOSBoxLocalServerPortChanged, this, std::placeholders::_1));
	m_dosboxRemoteServerPortChangedConnection = m_modManager->dosboxRemoteServerPortChanged.connect(std::bind(&ModBrowserPanel::onDOSBoxRemoteServerPortChanged, this, std::placeholders::_1));
	m_filterTypeChangedConnection = organizedMods->filterTypeChanged.connect(std::bind(&ModBrowserPanel::onFilterTypeChanged, this, std::placeholders::_1));
	m_sortOptionsChangedConnection = organizedMods->sortOptionsChanged.connect(std::bind(&ModBrowserPanel::onSortOptionsChanged, this, std::placeholders::_1, std::placeholders::_2));
	m_selectedModChangedConnection = organizedMods->selectedModChanged.connect(std::bind(&ModBrowserPanel::onSelectedModChanged, this, std::placeholders::_1));
	m_selectedFavouriteModChangedConnection = organizedMods->selectedFavouriteModChanged.connect(std::bind(&ModBrowserPanel::onSelectedFavouriteModChanged, this, std::placeholders::_1));
	m_selectedGameVersionChangedConnection = organizedMods->selectedGameVersionChanged.connect(std::bind(&ModBrowserPanel::onSelectedGameVersionChanged, this, std::placeholders::_1));
	m_selectedTeamChangedConnection = organizedMods->selectedTeamChanged.connect(std::bind(&ModBrowserPanel::onSelectedTeamChanged, this, std::placeholders::_1));
	m_selectedAuthorChangedConnection = organizedMods->selectedAuthorChanged.connect(std::bind(&ModBrowserPanel::onSelectedAuthorChanged, this, std::placeholders::_1));
	m_organizedModCollectionChangedConnection = organizedMods->organizedModCollectionChanged.connect(std::bind(&ModBrowserPanel::onOrganizedModCollectionChanged, this, std::placeholders::_1));
	m_organizedFavouriteModCollectionChangedConnection = organizedMods->organizedFavouriteModCollectionChanged.connect(std::bind(&ModBrowserPanel::onOrganizedFavouriteModCollectionChanged, this, std::placeholders::_1));
	m_organizedModGameVersionCollectionChangedConnection = organizedMods->organizedModGameVersionCollectionChanged.connect(std::bind(&ModBrowserPanel::onOrganizedModGameVersionCollectionChanged, this, std::placeholders::_1));
	m_organizedModTeamCollectionChangedConnection = organizedMods->organizedModTeamCollectionChanged.connect(std::bind(&ModBrowserPanel::onOrganizedModTeamCollectionChanged, this, std::placeholders::_1));
	m_organizedModAuthorCollectionChangedConnection = organizedMods->organizedModAuthorCollectionChanged.connect(std::bind(&ModBrowserPanel::onOrganizedModAuthorCollectionChanged, this, std::placeholders::_1));
	m_dosboxVersionCollectionSizeChangedConnection = dosboxVersions->sizeChanged.connect(std::bind(&ModBrowserPanel::onDOSBoxVersionCollectionSizeChanged, this, std::placeholders::_1));
	m_dosboxVersionCollectionItemModifiedConnection = dosboxVersions->itemModified.connect(std::bind(&ModBrowserPanel::onDOSBoxVersionCollectionItemModified, this, std::placeholders::_1, std::placeholders::_2));
	m_gameVersionCollectionSizeChangedConnection = gameVersions->sizeChanged.connect(std::bind(&ModBrowserPanel::onGameVersionCollectionSizeChanged, this, std::placeholders::_1));
	m_gameVersionCollectionItemModifiedConnection = gameVersions->itemModified.connect(std::bind(&ModBrowserPanel::onGameVersionCollectionItemModified, this, std::placeholders::_1, std::placeholders::_2));

	m_launchedConnection = m_modManager->launched.connect(std::bind(&ModBrowserPanel::onLaunched, this));
	m_launchStatusConnection = m_modManager->launchStatus.connect(std::bind(&ModBrowserPanel::onLaunchStatus, this, std::placeholders::_1));
	m_launchErrorConnection = m_modManager->launchError.connect(std::bind(&ModBrowserPanel::onLaunchError, this, std::placeholders::_1));
	m_gameProcessTerminatedConnection = m_modManager->gameProcessTerminated.connect(std::bind(&ModBrowserPanel::onGameProcessTerminated, this, std::placeholders::_1, std::placeholders::_2));

	Bind(EVENT_LAUNCH_FAILED, &ModBrowserPanel::onLaunchFailed, this);
	Bind(EVENT_GAME_PROCESS_TERMINATED, &ModBrowserPanel::onGameProcessEnded, this);
	Bind(EVENT_MOD_INSTALL_PROGRESS, &ModBrowserPanel::onModInstallProgress, this);
	Bind(EVENT_MOD_INSTALL_DONE, &ModBrowserPanel::onModInstallDone, this);

	wxPanel * modListOptionsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "Mod List Options");

	m_modSearchTextField = new wxSearchCtrl(modListOptionsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Mod Search");
	m_modSearchTextField->Bind(wxEVT_TEXT, &ModBrowserPanel::onModSearchTextChanged, this);
	m_modSearchTextField->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &ModBrowserPanel::onModSearchCancelled, this);

	m_selectRandomModButton = new wxButton(modListOptionsPanel, wxID_ANY, "Random", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Select Random Mod");
	m_selectRandomModButton->Bind(wxEVT_BUTTON, &ModBrowserPanel::onSelectRandomModButtonPressed, this);

	if(organizedMods->numberOfMods() == 0) {
		m_selectRandomModButton->Disable();
	}

	wxStaticText * modListFilterTypeLabel = new wxStaticText(modListOptionsPanel, wxID_ANY, "Filter Type:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	modListFilterTypeLabel->SetFont(modListFilterTypeLabel->GetFont().MakeBold());
	m_modListFilterTypeComboBox = new wxComboBox(modListOptionsPanel, wxID_ANY, Utilities::toCapitalCase(magic_enum::enum_name(organizedMods->getFilterType())), wxDefaultPosition, wxDefaultSize, WXUtilities::createEnumWXArrayString<OrganizedModCollection::FilterType>(), 0, wxDefaultValidator, "Mod List Filter Type");
	m_modListFilterTypeComboBox->SetEditable(false);
	m_modListFilterTypeComboBox->Bind(wxEVT_COMBOBOX, &ModBrowserPanel::onModListFilterTypeSelected, this);

	wxStaticText * modListSortTypeLabel = new wxStaticText(modListOptionsPanel, wxID_ANY, "Sort Type:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	modListSortTypeLabel->SetFont(modListSortTypeLabel->GetFont().MakeBold());
	m_modListSortTypeComboBox = new wxComboBox(modListOptionsPanel, wxID_ANY, Utilities::toCapitalCase(magic_enum::enum_name(organizedMods->getSortType())), wxDefaultPosition, wxDefaultSize, WXUtilities::createEnumWXArrayString<OrganizedModCollection::SortType>(organizedMods->getInvalidSortTypesInCurrentContext()), 0, wxDefaultValidator, "Mod List Sort Type");
	m_modListSortTypeComboBox->SetEditable(false);
	m_modListSortTypeComboBox->SetMinClientSize(wxSize(170, m_modListSortTypeComboBox->GetMinClientSize().GetHeight()));
	m_modListSortTypeComboBox->Bind(wxEVT_COMBOBOX, &ModBrowserPanel::onModListSortTypeSelected, this);

	wxStaticText * modListSortDirectionLabel = new wxStaticText(modListOptionsPanel, wxID_ANY, "Direction:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	modListSortDirectionLabel->SetFont(modListSortDirectionLabel->GetFont().MakeBold());
	m_modListSortDirectionComboBox = new wxComboBox(modListOptionsPanel, wxID_ANY, Utilities::toCapitalCase(magic_enum::enum_name(organizedMods->getSortDirection())), wxDefaultPosition, wxDefaultSize, WXUtilities::createEnumWXArrayString<OrganizedModCollection::SortDirection>(), 0, wxDefaultValidator, "Mod List Sort Direction");
	m_modListSortDirectionComboBox->SetEditable(false);
	m_modListSortDirectionComboBox->Bind(wxEVT_COMBOBOX, &ModBrowserPanel::onModListSortDirectionSelected, this);

	m_clearButton = new wxButton(modListOptionsPanel, wxID_ANY, "Clear", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Clear");
	m_clearButton->Disable();
	m_clearButton->Bind(wxEVT_BUTTON, &ModBrowserPanel::onClearButtonPressed, this);

	wxPanel * modSelectionPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "Mod Selection");

	m_modListLabel = new wxStaticText(modSelectionPanel, wxID_ANY, "Mods", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_modListLabel->SetFont(m_modListLabel->GetFont().MakeBold());
	m_modListBox = new wxListBox(modSelectionPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, WXUtilities::createItemWXArrayString(organizedMods->getOrganizedItemDisplayNames()), wxLB_SINGLE | wxLB_ALWAYS_SB);
	m_modListBox->SetMinClientSize(wxSize(280, m_modListBox->GetMinClientSize().GetHeight()));
	m_modListBox->Bind(wxEVT_LISTBOX, &ModBrowserPanel::onModSelected, this);
	m_modListBox->Bind(wxEVT_RIGHT_UP, &ModBrowserPanel::onModListRightClicked, this);

	m_modVersionListLabel = new wxStaticText(modSelectionPanel, wxID_ANY, "Mod Versions", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_modVersionListLabel->SetFont(m_modVersionListLabel->GetFont().MakeBold());
	m_modVersionListBox = new wxListBox(modSelectionPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, {}, wxLB_SINGLE | wxLB_ALWAYS_SB);
	m_modVersionListBox->SetMinClientSize(wxSize(160, m_modVersionListBox->GetMinClientSize().GetHeight()));
	m_modVersionListBox->Bind(wxEVT_LISTBOX, &ModBrowserPanel::onModVersionSelected, this);
	m_modVersionListBox->Bind(wxEVT_RIGHT_UP, &ModBrowserPanel::onModVersionListRightClicked, this);

	m_modVersionTypeListLabel = new wxStaticText(modSelectionPanel, wxID_ANY, "Mod Version Types", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_modVersionTypeListLabel->SetFont(m_modVersionTypeListLabel->GetFont().MakeBold());
	m_modVersionTypeListBox = new wxListBox(modSelectionPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, {}, wxLB_SINGLE | wxLB_ALWAYS_SB);
	m_modVersionTypeListBox->Bind(wxEVT_LISTBOX, &ModBrowserPanel::onModVersionTypeSelected, this);
	m_modVersionTypeListBox->Bind(wxEVT_RIGHT_UP, &ModBrowserPanel::onModVersionTypeListRightClicked, this);

	m_modGameVersionListLabel = new wxStaticText(modSelectionPanel, wxID_ANY, "Compatible Mod Game Versions", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_modGameVersionListLabel->SetFont(m_modGameVersionListLabel->GetFont().MakeBold());
	m_modGameVersionListBox = new wxListBox(modSelectionPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, {}, wxLB_SINGLE | wxLB_ALWAYS_SB);
	m_modGameVersionListBox->Bind(wxEVT_LISTBOX, &ModBrowserPanel::onModGameVersionSelected, this);

	m_modListPopupMenu = std::make_unique<wxMenu>();
	m_modListAddFavouriteMenuItem = new wxMenuItem(m_modListPopupMenu.get(), wxID_ANY, "Add Favourite Mod", wxEmptyString, wxITEM_NORMAL);
	m_modListRemoveFavouriteMenuItem = new wxMenuItem(m_modListPopupMenu.get(), wxID_ANY, "Remove Favourite Mod", wxEmptyString, wxITEM_NORMAL);
	wxMenuItem * modListCancelMenuItem = new wxMenuItem(m_modListPopupMenu.get(), wxID_ANY, "Cancel", wxEmptyString, wxITEM_NORMAL);
	m_modListPopupMenu->Append(m_modListAddFavouriteMenuItem);
	m_modListPopupMenu->Append(m_modListRemoveFavouriteMenuItem);
	m_modListPopupMenu->Append(modListCancelMenuItem);
	m_modListPopupMenu->Bind(wxEVT_MENU, &ModBrowserPanel::onModPopupMenuItemPressed, this);

	m_modVersionListPopupMenu = std::make_unique<wxMenu>();
	m_modVersionListAddFavouriteMenuItem = new wxMenuItem(m_modVersionListPopupMenu.get(), wxID_ANY, "Add Favourite Mod Version", wxEmptyString, wxITEM_NORMAL);
	wxMenuItem * modVersionListCancelMenuItem = new wxMenuItem(m_modVersionListPopupMenu.get(), wxID_ANY, "Cancel", wxEmptyString, wxITEM_NORMAL);
	m_modVersionListPopupMenu->Append(m_modVersionListAddFavouriteMenuItem);
	m_modVersionListPopupMenu->Append(modVersionListCancelMenuItem);
	m_modVersionListPopupMenu->Bind(wxEVT_MENU, &ModBrowserPanel::onModVersionPopupMenuItemPressed, this);

	m_modVersionTypeListPopupMenu = std::make_unique<wxMenu>();
	m_modVersionTypeListAddFavouriteMenuItem = new wxMenuItem(m_modVersionTypeListPopupMenu.get(), wxID_ANY, "Add Favourite Mod Version Type", wxEmptyString, wxITEM_NORMAL);
	wxMenuItem * modVersionTypeListCancelMenuItem = new wxMenuItem(m_modVersionTypeListPopupMenu.get(), wxID_ANY, "Cancel", wxEmptyString, wxITEM_NORMAL);
	m_modVersionTypeListPopupMenu->Append(m_modVersionTypeListAddFavouriteMenuItem);
	m_modVersionTypeListPopupMenu->Append(modVersionTypeListCancelMenuItem);
	m_modVersionTypeListPopupMenu->Bind(wxEVT_MENU, &ModBrowserPanel::onModVersionTypePopupMenuItemPressed, this);

	m_modInfoBox = new wxStaticBox(modSelectionPanel, wxID_ANY, "Mod Information", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Mod Information");
	m_modInfoBox->SetOwnFont(m_modInfoBox->GetFont().MakeBold());

	m_modInfoPanel = new ModInfoPanel(m_modManager->getMods(), m_modManager->getGameVersions(), m_modInfoBox);
	m_modInfoPanel->Hide();
	m_modInfoPanelSignalConnectionGroup = SignalConnectionGroup(
		m_modInfoPanel->modSelectionRequested.connect(std::bind(&ModBrowserPanel::onModSelectionRequested, this, std::placeholders::_1)),
		m_modInfoPanel->modTeamSelectionRequested.connect(std::bind(&ModBrowserPanel::onModTeamSelectionRequested, this, std::placeholders::_1)),
		m_modInfoPanel->modTeamMemberSelectionRequested.connect(std::bind(&ModBrowserPanel::onModTeamMemberSelectionRequested, this, std::placeholders::_1)),
		m_modInfoPanel->modVersionTypeSelectionRequested.connect(std::bind(&ModBrowserPanel::onModVersionTypeSelectionRequested, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
	);

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
	m_preferredDOSBoxVersionComboBox = new wxComboBox(m_gameOptionsPanel, wxID_ANY, preferredDOSBoxVersion == nullptr ? "" : preferredDOSBoxVersion->getShortName(), wxDefaultPosition, wxDefaultSize, WXUtilities::createItemWXArrayString(dosboxVersions->getDOSBoxVersionShortNames(false)), 0, wxDefaultValidator, "DOSBox Versions");
	m_preferredDOSBoxVersionComboBox->SetEditable(false);
	m_preferredDOSBoxVersionComboBox->Bind(wxEVT_COMBOBOX, &ModBrowserPanel::onPreferredDOSBoxVersionSelected, this);

	wxStaticText * modGameTypeLabel = new wxStaticText(m_gameOptionsPanel, wxID_ANY, "Game Type:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	modGameTypeLabel->SetFont(modGameTypeLabel->GetFont().MakeBold());
	m_modGameTypeComboBox = new wxComboBox(m_gameOptionsPanel, wxID_ANY, Utilities::toCapitalCase(magic_enum::enum_name(m_modManager->getGameType())), wxDefaultPosition, wxDefaultSize, WXUtilities::createEnumWXArrayString<GameType>(), 0, wxDefaultValidator, "Game Type");
	m_modGameTypeComboBox->SetEditable(false);
	m_modGameTypeComboBox->Bind(wxEVT_COMBOBOX, &ModBrowserPanel::onModGameTypeSelected, this);

	std::shared_ptr<GameVersion> preferredGameVersion(m_modManager->getPreferredGameVersion());

	wxStaticText * preferredGameVersionLabel = new wxStaticText(m_gameOptionsPanel, wxID_ANY, "Game Version:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	preferredGameVersionLabel->SetFont(preferredGameVersionLabel->GetFont().MakeBold());
	m_preferredGameVersionComboBox = new wxComboBox(m_gameOptionsPanel, wxID_ANY, preferredGameVersion == nullptr ? "" : preferredGameVersion->getShortName(), wxDefaultPosition, wxDefaultSize, WXUtilities::createItemWXArrayString(m_modManager->getGameVersions()->getGameVersionShortNames(false)), 0, wxDefaultValidator, "Game Versions");
	m_preferredGameVersionComboBox->SetEditable(false);
	m_preferredGameVersionComboBox->Bind(wxEVT_COMBOBOX, &ModBrowserPanel::onPreferredGameVersionSelected, this);

	m_uninstallButton = new wxButton(m_gameOptionsPanel, wxID_ANY, "Uninstall", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Uninstall Mod Game Version");
	m_uninstallButton->Bind(wxEVT_BUTTON, &ModBrowserPanel::onUninstallButtonPressed, this);
	m_uninstallButton->Disable();

	m_launchButton = new wxButton(m_gameOptionsPanel, wxID_ANY, "Launch Game", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Launch Game/Mod");
	m_launchButton->Bind(wxEVT_BUTTON, &ModBrowserPanel::onLaunchButtonPressed, this);

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
	modSelectionSizer->AddGrowableCol(2, 5);
	modSelectionPanel->SetSizerAndFit(modSelectionSizer);

	wxBoxSizer * modInfoSizer = new wxBoxSizer(wxHORIZONTAL);
	modInfoSizer->Add(m_modInfoPanel, 1, wxEXPAND | wxALL, 20);
	m_modInfoBox->SetSizer(modInfoSizer);

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
	gameOptionsSizer->Add(m_uninstallButton, wxGBPosition(0, 10), wxGBSpan(1, 1), wxSTRETCH_NOT | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, border);
	gameOptionsSizer->Add(m_launchButton, wxGBPosition(0, 11), wxGBSpan(1, 1), wxSTRETCH_NOT | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, border);
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

	if(m_modManager->hasModSelected()) {
		updateModList();
		organizedMods->setSelectedMod(m_modManager->getSelectedMod().get());
	}
}

ModBrowserPanel::~ModBrowserPanel() {
	m_launchedConnection.disconnect();
	m_launchStatusConnection.disconnect();
	m_launchErrorConnection.disconnect();
	m_gameProcessTerminatedConnection.disconnect();
	m_modSelectionChangedConnection.disconnect();
	m_gameTypeChangedConnection.disconnect();
	m_preferredDOSBoxVersionChangedConnection.disconnect();
	m_preferredGameVersionChangedConnection.disconnect();
	m_dosboxServerIPAddressChangedConnection.disconnect();
	m_dosboxLocalServerPortChangedConnection.disconnect();
	m_dosboxRemoteServerPortChangedConnection.disconnect();
	m_preferredDOSBoxVersionChangedConnection.disconnect();
	m_preferredGameVersionChangedConnection.disconnect();
	m_dosboxServerIPAddressChangedConnection.disconnect();
	m_dosboxLocalServerPortChangedConnection.disconnect();
	m_dosboxRemoteServerPortChangedConnection.disconnect();
	m_filterTypeChangedConnection.disconnect();
	m_sortOptionsChangedConnection.disconnect();
	m_selectedModChangedConnection.disconnect();
	m_selectedFavouriteModChangedConnection.disconnect();
	m_selectedGameVersionChangedConnection.disconnect();
	m_selectedTeamChangedConnection.disconnect();
	m_selectedAuthorChangedConnection.disconnect();
	m_organizedModCollectionChangedConnection.disconnect();
	m_organizedFavouriteModCollectionChangedConnection.disconnect();
	m_organizedModGameVersionCollectionChangedConnection.disconnect();
	m_organizedModTeamCollectionChangedConnection.disconnect();
	m_organizedModAuthorCollectionChangedConnection.disconnect();
	m_dosboxVersionCollectionSizeChangedConnection.disconnect();
	m_dosboxVersionCollectionItemModifiedConnection.disconnect();
	m_gameVersionCollectionSizeChangedConnection.disconnect();
	m_gameVersionCollectionItemModifiedConnection.disconnect();
	m_modInfoPanelSignalConnectionGroup.disconnect();
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
	m_modListFilterTypeComboBox->SetSelection(magic_enum::enum_index(m_modManager->getOrganizedMods()->getFilterType()).value_or(wxNOT_FOUND));
}

void ModBrowserPanel::updateModListSortOptions() {
	updateModListSortType();
	updateModListSortDirection();
}

void ModBrowserPanel::updateModListSortType() {
	std::shared_ptr<OrganizedModCollection> organizedMods(m_modManager->getOrganizedMods());

	m_modListSortTypeComboBox->Set(WXUtilities::createEnumWXArrayString<OrganizedModCollection::SortType>(organizedMods->getInvalidSortTypesInCurrentContext()));
	m_modListSortTypeComboBox->SetSelection(organizedMods->indexOfCurrentSortType());
}

void ModBrowserPanel::updateModListSortDirection() {
	m_modListSortDirectionComboBox->SetSelection(magic_enum::enum_index(m_modManager->getOrganizedMods()->getSortDirection()).value_or(wxNOT_FOUND));
}

void ModBrowserPanel::updateModList() {
	updateModInfo();

	std::shared_ptr<OrganizedModCollection> organizedMods(m_modManager->getOrganizedMods());

	if(m_searchQuery.empty()) {
		std::shared_ptr<Mod> mod(m_modManager->getSelectedMod());

		clearSearchResults();

		if(organizedMods->shouldDisplayMods()) {
			m_modListLabel->SetLabelText("Mods");
		}
		if(organizedMods->shouldDisplayFavouriteMods()) {
			m_modListLabel->SetLabelText("Favourite Mods");
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

		wxArrayString matchesArrayString;

		if(organizedMods->shouldDisplayMods()) {
			m_modMatches = ModManager::searchForMod(organizedMods->getOrganizedMods(), m_searchQuery);

			for(size_t i = 0; i < m_modMatches.size(); i++) {
				std::stringstream modMatchStringStream;
				modMatchStringStream << std::to_string(i + 1) << ". " << m_modMatches[i].toString();

				if(organizedMods->getSortType() == OrganizedModCollection::SortType::InitialReleaseDate || organizedMods->getSortType() == OrganizedModCollection::SortType::LatestReleaseDate) {
					std::string releaseDate(organizedMods->getSortType() == OrganizedModCollection::SortType::InitialReleaseDate ?  m_modMatches[i].getMod()->getInitialReleaseDateAsString() : m_modMatches[i].getMod()->getLatestReleaseDateAsString());

					if(!releaseDate.empty()) {
						modMatchStringStream << " (" << releaseDate << ")";
					}
				}
				else if(organizedMods->getSortType() == OrganizedModCollection::SortType::NumberOfVersions) {
					modMatchStringStream << " (" << std::to_string(m_modMatches[i].getMod()->numberOfVersions()) << ")";
				}

				matchesArrayString.Add(wxString::FromUTF8(modMatchStringStream.str()));
			}
		}
		else if(organizedMods->shouldDisplayFavouriteMods()) {
			m_favouriteModMatches = ModManager::searchForFavouriteMod(organizedMods->getOrganizedFavouriteMods(), m_searchQuery);

			for(size_t i = 0; i < m_favouriteModMatches.size(); i++) {
				matchesArrayString.Add(wxString::FromUTF8(fmt::format("{}. {}", i + 1, m_favouriteModMatches[i]->getFullName())));
			}
		}
		else if(organizedMods->shouldDisplayGameVersions()) {
			m_gameVersionMatches = ModManager::searchForGameVersion(organizedMods->getOrganizedGameVersions(), m_searchQuery);

			for(size_t i = 0; i < m_gameVersionMatches.size(); i++) {
				std::stringstream gameVersionMatchStringStream;
				gameVersionMatchStringStream << std::to_string(i + 1) << ". " << m_gameVersionMatches[i]->getLongName();

				if(organizedMods->getSortType() == OrganizedModCollection::SortType::NumberOfSupportedMods ||
				   (organizedMods->getFilterType() == OrganizedModCollection::FilterType::SupportedGameVersions && organizedMods->getSortType() != OrganizedModCollection::SortType::NumberOfCompatibleMods)) {
					gameVersionMatchStringStream << " (" << std::to_string(organizedMods->getSupportedModCountForGameVersionWithID(m_gameVersionMatches[i]->getID())) << ")";
				}
				else if(organizedMods->getFilterType() == OrganizedModCollection::FilterType::CompatibleGameVersions || organizedMods->getSortType() == OrganizedModCollection::SortType::NumberOfCompatibleMods) {
					gameVersionMatchStringStream << " (" << std::to_string(organizedMods->getCompatibleModCountForGameVersionWithID(m_gameVersionMatches[i]->getID())) << ")";
				}

				matchesArrayString.Add(wxString::FromUTF8(gameVersionMatchStringStream.str()));
			}
		}
		else if(organizedMods->shouldDisplayTeams() || organizedMods->shouldDisplayAuthors()) {
			m_modAuthorMatches = ModManager::searchForAuthor(organizedMods->shouldDisplayTeams() ? organizedMods->getOrganizedTeams() : organizedMods->getOrganizedAuthors(), m_searchQuery);

			for(size_t i = 0; i < m_modAuthorMatches.size(); i++) {
				matchesArrayString.Add(wxString::FromUTF8(fmt::format("{}. {} ({})", i + 1, m_modAuthorMatches[i]->getName(), m_modAuthorMatches[i]->getModCount())));
			}
		}

		m_modListLabel->SetLabelText("Search Results");
		m_modListBox->Set(matchesArrayString);

		m_selectRandomModButton->Disable();
	}

	updateModVersionList();
}

void ModBrowserPanel::updateModVersionList() {
	if(m_searchQuery.empty()) {
		std::shared_ptr<Mod> mod(m_modManager->getSelectedMod());
		std::shared_ptr<OrganizedModCollection> organizedMods(m_modManager->getOrganizedMods());

		if(mod != nullptr && (organizedMods->shouldDisplayMods() || organizedMods->shouldDisplayFavouriteMods())) {
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
		std::shared_ptr<OrganizedModCollection> organizedMods(m_modManager->getOrganizedMods());

		if(mod != nullptr) {
			if(organizedMods->shouldDisplayMods() || organizedMods->shouldDisplayFavouriteMods()) {
				size_t modVersionIndex = m_modManager->getSelectedModVersionIndex();
				size_t modVersionTypeIndex = m_modManager->getSelectedModVersionTypeIndex();

				if(modVersionIndex != std::numeric_limits<size_t>::max()) {
					m_modVersionTypeListBox->Set(WXUtilities::createItemWXArrayString(mod->getVersion(modVersionIndex)->getTypeDisplayNames("Default")));

					if(modVersionTypeIndex != std::numeric_limits<size_t>::max()) {
						updateLaunchButton();
						m_launchButton->Enable();
						m_modVersionTypeListBox->SetSelection(modVersionTypeIndex);
					}
					else {
						m_launchButton->Disable();
						m_modVersionTypeListBox->SetSelection(wxNOT_FOUND);
					}
				}
				else {
					m_launchButton->Disable();
					m_modVersionTypeListBox->Clear();
				}
			}
			else {
				m_launchButton->Disable();
				m_modVersionTypeListBox->Clear();
			}
		}
		else {
			updateLaunchButton();
			m_launchButton->Enable();
			m_modVersionTypeListBox->Clear();
		}
	}
	else {
		m_launchButton->Disable();
		m_modVersionTypeListBox->Clear();
	}

	updateModGameVersionList();
}

void ModBrowserPanel::updateModGameVersionList() {
	m_uninstallButton->Disable();

	if(m_searchQuery.empty()) {
		std::shared_ptr<Mod> mod(m_modManager->getSelectedMod());
		std::shared_ptr<OrganizedModCollection> organizedMods(m_modManager->getOrganizedMods());
		std::shared_ptr<StandAloneModCollection> standAloneMods(m_modManager->getStandAloneMods());
		std::shared_ptr<GameVersionCollection> gameVersions(m_modManager->getGameVersions());

		if(mod != nullptr && (organizedMods->shouldDisplayMods() || organizedMods->shouldDisplayFavouriteMods())) {
			size_t modVersionIndex = m_modManager->getSelectedModVersionIndex();
			size_t modVersionTypeIndex = m_modManager->getSelectedModVersionTypeIndex();
			size_t modGameVersionIndex = m_modManager->getSelectedModGameVersionIndex();

			if(modVersionIndex != std::numeric_limits<size_t>::max()) {
				std::shared_ptr<GameVersion> selectedGameVersion(m_modManager->getSelectedGameVersion());

				if(modVersionTypeIndex != std::numeric_limits<size_t>::max() && selectedGameVersion != nullptr) {
					std::shared_ptr<ModVersionType> selectedModVersionType(mod->getVersion(modVersionIndex)->getType(modVersionTypeIndex));
					std::shared_ptr<ModGameVersion> selectedModGameVersion;

					if(modGameVersionIndex != std::numeric_limits<size_t>::max()) {
						selectedModGameVersion = selectedModVersionType->getGameVersion(modGameVersionIndex);
					}

					if(selectedModVersionType->isStandAlone()) {
						std::vector<std::string> modGameVersionShortNames(selectedModVersionType->getModGameVersionShortNames(*gameVersions));
						m_modGameVersionListBox->Set(WXUtilities::createItemWXArrayString(modGameVersionShortNames));

						if(modGameVersionIndex != std::numeric_limits<size_t>::max()) {
							m_modGameVersionListBox->SetSelection(modGameVersionIndex);

							m_modGameVersionListLabel->SetLabelText("Mod Game Versions");

							updateUninstallButton();
						}
						else {
							m_modGameVersionListBox->SetSelection(wxNOT_FOUND);
						}
					}
					else {
						std::vector<std::string> compatibleModGameVersionShortNames(selectedModVersionType->getCompatibleModGameVersionShortNames(*selectedGameVersion, *gameVersions));
						m_modGameVersionListBox->Set(WXUtilities::createItemWXArrayString(compatibleModGameVersionShortNames));

						if(modGameVersionIndex != std::numeric_limits<size_t>::max()) {
							std::vector<std::string>::const_iterator compatibleModGameVersionShortNameIterator(std::find_if(compatibleModGameVersionShortNames.cbegin(), compatibleModGameVersionShortNames.cend(), [gameVersions, selectedModGameVersion](const std::string & currentModGameVersionShortName) {
								return Utilities::areStringsEqualIgnoreCase(gameVersions->getShortNameOfGameVersionWithID(selectedModGameVersion->getGameVersionID()), currentModGameVersionShortName);
							}));

							if(compatibleModGameVersionShortNameIterator != compatibleModGameVersionShortNames.cend()) {
								m_modGameVersionListBox->SetSelection(compatibleModGameVersionShortNameIterator - compatibleModGameVersionShortNames.cbegin());

								m_modGameVersionListLabel->SetLabelText("Compatible Mod Game Versions");

								updateUninstallButton();
							}
							else {
								spdlog::error("Failed to find compatible mod game version '{}' in list for mod '{}' with selected game version '{}'.", gameVersions->getLongNameOfGameVersionWithID(selectedModGameVersion->getGameVersionID()), mod->getFullName(modVersionIndex, modVersionTypeIndex), selectedGameVersion->getLongName());

								m_modGameVersionListBox->SetSelection(wxNOT_FOUND);
							}
						}
						else {
							m_modGameVersionListBox->SetSelection(wxNOT_FOUND);
						}
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
	updateLaunchButton();
}

void ModBrowserPanel::updateLaunchButton() {
	std::shared_ptr<ModVersionType> selectedModVersionType(m_modManager->getSelectedModVersionType());

	if(selectedModVersionType == nullptr) {
		m_launchButton->SetLabelText("Launch Game");
	}
	else {
		std::shared_ptr<ModGameVersion> selectedModGameVersion(m_modManager->getSelectedModGameVersion());

		if(selectedModGameVersion != nullptr && selectedModGameVersion->isStandAlone() && !m_modManager->getStandAloneMods()->hasStandAloneMod(*selectedModGameVersion)) {
			m_launchButton->SetLabelText("Install Mod");
		}
		else {
			m_launchButton->SetLabelText("Launch Mod");
		}
	}
}

void ModBrowserPanel::updateModInfo() {
	if(m_searchQuery.empty()) {
		std::shared_ptr<Mod> mod(m_modManager->getSelectedMod());

		if(mod != nullptr && (m_modManager->getOrganizedMods()->shouldDisplayMods() || m_modManager->getOrganizedMods()->shouldDisplayFavouriteMods())) {
			m_modInfoPanel->setMod(mod);
			m_modInfoPanel->setModVersionType(m_modManager->getSelectedModVersionType());
			m_modInfoBox->Layout();
			m_modInfoPanel->Show();
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
	m_preferredDOSBoxVersionComboBox->Set(WXUtilities::createItemWXArrayString(m_modManager->getDOSBoxVersions()->getDOSBoxVersionShortNames(false)));

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
	m_preferredGameVersionComboBox->Set(WXUtilities::createItemWXArrayString(m_modManager->getGameVersions()->getGameVersionShortNames(false)));

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

void ModBrowserPanel::updateUninstallButton() {
	std::shared_ptr<ModGameVersion> selectedModGameVersion(m_modManager->getSelectedModGameVersion());

	if(selectedModGameVersion == nullptr) {
		return;
	}

	if(selectedModGameVersion->isStandAlone()) {
		WXUtilities::setButtonEnabled(m_uninstallButton, m_modManager->getStandAloneMods()->isStandAloneModInstalled(*selectedModGameVersion));
	}
	else {
		WXUtilities::setButtonEnabled(m_uninstallButton, !m_modManager->isUsingLocalMode() && m_modManager->getDownloadManager()->isModGameVersionDownloaded(*selectedModGameVersion));
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
	clearSearchResults();

	m_searchQuery.clear();
	m_modSearchTextField->Clear();
}

void ModBrowserPanel::clearSearchResults() {
	m_modMatches.clear();
	m_favouriteModMatches.clear();
	m_gameVersionMatches.clear();
	m_modAuthorMatches.clear();
}

bool ModBrowserPanel::installStandAloneMod(std::shared_ptr<ModGameVersion> standAloneModGameVersion) {
	if(m_installModProgressDialog) {
		spdlog::error("Stand-alone mod installation already in progress!");
		return false;
	}

	m_modInstallationCancelled = false;

	if(!ModGameVersion::isValid(standAloneModGameVersion.get()) || !standAloneModGameVersion->isStandAlone()) {
		wxMessageBox("Cannot install invalid stand-alone mod!", "Invalid Stand-Alone Mod", wxOK | wxICON_ERROR, this);
		return false;
	}

	SettingsManager * settings = SettingsManager::getInstance();
	std::shared_ptr<StandAloneModCollection> standAloneMods(m_modManager->getStandAloneMods());
	std::shared_ptr<StandAloneMod> standAloneMod(standAloneMods->getStandAloneMod(*standAloneModGameVersion));

	if(standAloneMod != nullptr) {
		wxMessageBox(fmt::format("Stand-alone mod '{}' is already installed!", standAloneModGameVersion->getParentModVersion()->getFullName()), "Stand-Alone Mod Already Installed", wxOK | wxICON_WARNING, this);
		return true;
	}

	if(!settings->standAloneModDisclaimerAcknowledged) {
		int result = wxMessageBox("Stand-alone mods bring their own game executable file(s), which will be executed while playing. While this is generally quite safe as all mods are manually curated, vetted, and scanned for viruses / trojans, I assume no responsibility for any damages incurred as a result of this in the case that something does slip through this process. Do you understand the risk and wish to proceed?", "Comfirm Installation", wxOK | wxCANCEL | wxICON_WARNING, this);

		if(result == wxCANCEL) {
			return false;
		}

		settings->standAloneModDisclaimerAcknowledged = true;
	}

	wxDirDialog selectInstallDirectoryDialog(this, "Install Stand-Alone Mod to Directory", std::filesystem::current_path().string(), wxDD_DIR_MUST_EXIST, wxDefaultPosition, wxDefaultSize, "Install Stand-Alone Mod");
	int selectInstallDirectoryResult = selectInstallDirectoryDialog.ShowModal();

	if(selectInstallDirectoryResult == wxID_CANCEL) {
		return false;
	}

	std::string destinationDirectoryPath(selectInstallDirectoryDialog.GetPath());

	if(Utilities::areStringsEqual(Utilities::getAbsoluteFilePath(destinationDirectoryPath), Utilities::getAbsoluteFilePath(std::filesystem::current_path().string()))) {
		wxMessageBox("Cannot install stand-alone mod directly into mod manager directory!", "Invalid Installation Directory", wxOK | wxICON_ERROR, this);
		return false;
	}

	if(!std::filesystem::is_directory(std::filesystem::path(destinationDirectoryPath))) {
		wxMessageBox("Invalid stand-alone mod installation directory selected!", "Invalid Installation Directory", wxOK | wxICON_ERROR, this);
		return false;
	}

	for(const std::filesystem::directory_entry & entry : std::filesystem::directory_iterator(std::filesystem::path(destinationDirectoryPath))) {
		int installToNonEmptyDirectoryResult = wxMessageBox(fmt::format("Stand-alone mod installation installation directory '{}' is not empty!\n\nInstalling to a directory which already contains files may cause issues. Are you sure you would like to install '{}' to this directory?", destinationDirectoryPath, standAloneModGameVersion->getParentModVersion()->getFullName()), "Non-Empty Installation Directory", wxYES_NO | wxCANCEL | wxICON_WARNING, this);

		if(installToNonEmptyDirectoryResult != wxYES) {
			return false;
		}

		break;
	}

	m_installModProgressDialog = new wxProgressDialog("Installing Stand-Alone Mod", fmt::format("Installing '{}', please wait...", standAloneModGameVersion->getParentModVersion()->getFullName()), 101, this, wxPD_CAN_ABORT | wxPD_REMAINING_TIME);
	m_installModProgressDialog->SetIcon(wxICON(D3DMODMGR_ICON));

	SignalConnectionGroup modDownloadConnections(
		m_modManager->modDownloadStatusChanged.connect([this](const ModGameVersion & modGameVersion, uint8_t downloadStep, uint8_t downloadStepCount, const std::string & status) {
			QueueEvent(new ModInstallProgressEvent(m_installModProgressDialog->GetValue(), status));

			return true;
		}),
		m_modManager->modDownloadProgress.connect([this, standAloneModGameVersion](const ModGameVersion & modGameVersion, HTTPRequest & request, size_t numberOfBytesDownloaded, size_t totalNumberOfBytes) {
			QueueEvent(new ModInstallProgressEvent(
				static_cast<int>((static_cast<double>(numberOfBytesDownloaded) / static_cast<double>(totalNumberOfBytes)) * 100.0),
				fmt::format("Downloaded {} / {} of '{}' stand-alone mod files from: '{}'.", Utilities::fileSizeToString(numberOfBytesDownloaded), Utilities::fileSizeToString(totalNumberOfBytes), standAloneModGameVersion->getParentModVersion()->getFullName(), request.getUrl())
			));

			return !m_modInstallationCancelled;
		})
	);

	m_installModFuture = std::async(std::launch::async, [this, standAloneModGameVersion, destinationDirectoryPath, modDownloadConnections]() mutable {
		bool aborted = false;
		bool modInstalled = m_modManager->installStandAloneMod(*standAloneModGameVersion, destinationDirectoryPath, true, &aborted);

		modDownloadConnections.disconnect();

		if(aborted) {
			QueueEvent(new ModInstallDoneEvent(false));

			return false;
		}
		else if(!modInstalled) {
			QueueEvent(new ModInstallDoneEvent(false));

			wxMessageBox(fmt::format("Failed to install stand-alone '{}' mod.", standAloneModGameVersion->getParentModVersion()->getFullName()), "Stand-Alone Mod Install Failed", wxOK | wxICON_ERROR, this);

			return false;
		}

		updateUninstallButton();

		QueueEvent(new ModInstallDoneEvent(true));

		wxMessageBox(fmt::format("Stand-alone mod '{}' was successfully installed to: '{}'!", standAloneModGameVersion->getParentModVersion()->getFullName(), destinationDirectoryPath), "Stand-Alone Mod Installed", wxOK | wxICON_INFORMATION, this);

		return true;
	});

	return true;
}

bool ModBrowserPanel::launchGame() {
	if(m_modManager == nullptr || !m_modManager->isInitialized()) {
		wxMessageBox(
			"Mod manager is not initialized!",
			"Launch Failed",
			wxOK | wxICON_ERROR,
			this
		);

		return false;
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

			return false;
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

			return false;
		}
	}

	SettingsManager * settings = SettingsManager::getInstance();
	bool standAlone = false;
	std::shared_ptr<StandAloneMod> standAloneMod;
	std::shared_ptr<StandAloneModCollection> standAloneMods(m_modManager->getStandAloneMods());
	std::shared_ptr<GameVersion> alternateGameVersion;
	std::shared_ptr<ModGameVersion> selectedModGameVersion(m_modManager->getSelectedModGameVersion());
	std::shared_ptr<ModGameVersion> alternateModGameVersion;
	size_t selectedModVersionIndex = m_modManager->getSelectedModVersionIndex();
	size_t selectedModVersionTypeIndex = m_modManager->getSelectedModVersionTypeIndex();
	std::string fullModName;
	std::shared_ptr<GameVersion> selectedGameVersion(m_modManager->getSelectedGameVersion());
	std::shared_ptr<GameVersionCollection> gameVersions(m_modManager->getGameVersions());
	std::shared_ptr<DOSBoxVersion> selectedDOSBoxVersion(m_modManager->getSelectedDOSBoxVersion());

	if(selectedMod != nullptr) {
		if(selectedModGameVersion != nullptr && selectedModGameVersion->isStandAlone()) {
			standAlone = true;
			standAloneMod = standAloneMods->getStandAloneMod(*selectedModGameVersion);

			if(standAloneMod == nullptr) {
				wxMessageBox(fmt::format("Failed to locate installed stand-alone '{}' mod.", selectedModGameVersion->getParentModVersion()->getFullName()), "Missing Stand-Alone Mod", wxOK | wxICON_ERROR, this);

				return false;
			}
		}
		else {
			fullModName = selectedMod->getFullName(selectedModVersionIndex, selectedModVersionTypeIndex);

			if(!m_modManager->isModSupportedOnSelectedGameVersion()) {
				std::vector<std::shared_ptr<ModGameVersion>> * modGameVersions = nullptr;
				std::shared_ptr<ModVersionType> selectedModVersionType(selectedMod->getVersion(selectedModVersionIndex)->getType(selectedModVersionTypeIndex));
				std::vector<std::pair<std::shared_ptr<GameVersion>, std::vector<std::shared_ptr<ModGameVersion>>>> compatibleGameVersions(gameVersions->getGameVersionsCompatibleWith(selectedModVersionType->getGameVersions(), true, true));

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

					return false;
				}
				else if(compatibleGameVersions.size() == 1) {
					alternateGameVersion = compatibleGameVersions[0].first;
					modGameVersions = &compatibleGameVersions[0].second;

					int result = wxMessageBox(
						fmt::format(
							"{}\n"
							"Launching mod using '{}' instead!",
							selectedGameVersion != nullptr
								? fmt::format("'{}' is not supported on '{}'.", fullModName, selectedGameVersion->getLongName())
								: "No game version selected.",
							alternateGameVersion->getLongName()
						),
						"Unsupported Game Version",
						wxOK | wxCANCEL | wxICON_INFORMATION,
						this
					);

					if(result == wxCANCEL) {
						return false;
					}
				}
				else {
					std::vector<std::string> compatibleGameVersionNames;

					for(std::vector<std::pair<std::shared_ptr<GameVersion>, std::vector<std::shared_ptr<ModGameVersion>>>>::const_iterator i = compatibleGameVersions.begin(); i != compatibleGameVersions.end(); ++i) {
						compatibleGameVersionNames.push_back((*i).first->getShortName());
					}

					int selectedCompatibleGameVersionIndex = wxGetSingleChoiceIndex(
						fmt::format("{}\nPlease choose an alternative compatible game version to run:",
							selectedGameVersion != nullptr
								? fmt::format("'{}' is not supported on '{}'.", fullModName, selectedGameVersion->getLongName())
								: "No game version selected."
						),
						"Choose Game Version",
						WXUtilities::createItemWXArrayString(compatibleGameVersionNames),
						0,
						this
					);

					if(selectedCompatibleGameVersionIndex == wxNOT_FOUND) {
						return false;
					}

					alternateGameVersion = compatibleGameVersions[selectedCompatibleGameVersionIndex].first;
					modGameVersions = &compatibleGameVersions[selectedCompatibleGameVersionIndex].second;
				}

				if(modGameVersions->empty()) {
					return false;
				}
				else if(modGameVersions->size() == 1) {
					alternateModGameVersion = (*modGameVersions)[0];
				}
				else {
					std::vector<std::string> modGameVersionNames;

					for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = modGameVersions->begin(); i != modGameVersions->end(); ++i) {
						modGameVersionNames.push_back((*i)->getFullName(true));
					}

					int selectedModGameVersionIndex = wxGetSingleChoiceIndex(
						fmt::format("Choose a '{}' mod game version to run:", fullModName),
						"Choose Mod Game Version",
						WXUtilities::createItemWXArrayString(modGameVersionNames),
						0,
						this
					);

					if(selectedModGameVersionIndex == wxNOT_FOUND) {
						return false;
					}

					alternateModGameVersion = (*modGameVersions)[selectedModGameVersionIndex];
				}

				if(alternateGameVersion == nullptr || alternateModGameVersion == nullptr) {
					spdlog::error("Alternative game version not selected, aborting launch.");
					return false;
				}

				if(selectedGameVersion != nullptr) {
					spdlog::info("Using game version '{}' since '{}' is not supported on '{}'.", alternateGameVersion->getLongName(), fullModName, selectedGameVersion->getLongName());
				}
				else {
					spdlog::info("Using game version '{}' since no game version was selected.", alternateGameVersion->getLongName());
				}
			}
		}
	}

	if(standAlone) {
		m_activeGameVersion = m_modManager->getSelectedGameVersion();
	}
	else {
		m_activeGameVersion = alternateGameVersion != nullptr ? alternateGameVersion : selectedGameVersion;
	}

	if(!m_activeGameVersion->isConfigured()) {
		wxMessageBox(
			fmt::format(
				"Failed to launch{}, game version '{}' is not configured/installed.",
				!fullModName.empty() ? fmt::format(" '{}'", fullModName) : "",
				m_activeGameVersion->getLongName()
			),
			"Launch Failed",
			wxOK | wxICON_ERROR,
			this
		);

		return false;
	}
	else if(m_activeGameVersion->doesRequireDOSBox() && !selectedDOSBoxVersion->isConfigured()) {
		wxMessageBox(
			fmt::format(
				"Failed to launch{}, '{}' is not configured/installed.",
				!fullModName.empty() ? fmt::format(" '{}'", fullModName) : "",
				selectedDOSBoxVersion->getLongName()
			),
			"Launch Failed",
			wxOK | wxICON_ERROR,
			this
		);

		return false;
	}

	std::stringstream dialogMessageStringStream;

	if(!fullModName.empty()) {
		dialogMessageStringStream << '\'' << fullModName << "'";

		if(standAlone) {
			dialogMessageStringStream << " stand-alone";
		}

		dialogMessageStringStream << " mod is currently running in ";
	}

	dialogMessageStringStream << '\'' << m_activeGameVersion->getLongName() << '\'';

	if(fullModName.empty()) {
		dialogMessageStringStream << " is currently running";
	}

	if(m_activeGameVersion->doesRequireDOSBox()) {
		dialogMessageStringStream << " using '" << selectedDOSBoxVersion->getLongName() << '\'';
	}

	if(fullModName.empty()) {
		dialogMessageStringStream << " without any";

		if(standAlone) {
			dialogMessageStringStream << " additional";
		}

		dialogMessageStringStream << " mods";
	}

	dialogMessageStringStream << '.';

	m_gameRunningDialog = new ProcessRunningDialog(this, "Game Running", dialogMessageStringStream.str(), "Close Game");
	m_runSelectedModFuture = m_modManager->runSelectedModAsync(alternateGameVersion, alternateModGameVersion);
	int processExitCode = m_gameRunningDialog->ShowModal();
	m_gameRunningDialog = nullptr;

	return true;
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
		m_modManager->getOrganizedMods()->setSortTypeByIndex(selectedSortTypeIndex);
	}
}

void ModBrowserPanel::onModListSortDirectionSelected(wxCommandEvent & event) {
	int selectedSortDirectionIndex = m_modListSortDirectionComboBox->GetSelection();

	if(selectedSortDirectionIndex != wxNOT_FOUND) {
		m_modManager->getOrganizedMods()->setSortDirection(magic_enum::enum_value<OrganizedModCollection::SortDirection>(selectedSortDirectionIndex));
	}
}

void ModBrowserPanel::onModSelected(wxCommandEvent & event) {
	int selectedItemIndex = m_modListBox->GetSelection();

	if(selectedItemIndex == wxNOT_FOUND) {
		return;
	}

	if(m_searchQuery.empty()) {
		if(!m_modManager->getOrganizedMods()->selectItem(selectedItemIndex)) {
			spdlog::error("Failed to select item '{}' with index: '{}'.", std::string(event.GetString()), selectedItemIndex);
			return;
		}
	}
	else {
		std::shared_ptr<OrganizedModCollection> organizedMods(m_modManager->getOrganizedMods());

		if(organizedMods->shouldDisplayMods()) {
			const ModMatch & modMatch = m_modMatches[selectedItemIndex];
			m_modManager->getOrganizedMods()->setSelectedMod(modMatch.getMod().get());
			m_modManager->setSelectedModFromMatch(modMatch);
		}
		else if(organizedMods->shouldDisplayFavouriteMods()) {
			std::shared_ptr<ModIdentifier> favouriteModMatch(m_favouriteModMatches[selectedItemIndex]);
			m_modManager->getOrganizedMods()->setSelectedFavouriteMod(*favouriteModMatch);
			m_modManager->setSelectedMod(*favouriteModMatch);
		}
		else if(organizedMods->shouldDisplayGameVersions()) {
			m_modManager->getOrganizedMods()->setSelectedGameVersion(m_gameVersionMatches[selectedItemIndex].get());
		}
		else if(organizedMods->shouldDisplayTeams()) {
			m_modManager->getOrganizedMods()->setSelectedTeam(m_modAuthorMatches[selectedItemIndex].get());
		}
		else if(organizedMods->shouldDisplayAuthors()) {
			m_modManager->getOrganizedMods()->setSelectedAuthor(m_modAuthorMatches[selectedItemIndex].get());
		}

		clearSearchResults();
		m_searchQuery.clear();
		m_modSearchTextField->Clear();
		updateModList();
	}
}

void ModBrowserPanel::onModVersionSelected(wxCommandEvent & event) {
	int selectedModVersionIndex = m_modVersionListBox->GetSelection();

	if(selectedModVersionIndex == wxNOT_FOUND || !m_searchQuery.empty()) {
		return;
	}

	if(!m_modManager->setSelectedModVersionIndex(selectedModVersionIndex)) {
		spdlog::error("Failed to select mod version at index: '{}'.", selectedModVersionIndex);
		return;
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
		return;
	}
}

void ModBrowserPanel::onModGameVersionSelected(wxCommandEvent & event) {
	int selectedModGameVersionIndex = m_modGameVersionListBox->GetSelection();

	if(selectedModGameVersionIndex == wxNOT_FOUND || !m_searchQuery.empty()) {
		return;
	}

	if(!m_modManager->setSelectedModGameVersionIndex(selectedModGameVersionIndex)) {
		spdlog::error("Failed to select mod game version at index: '{}'.", selectedModGameVersionIndex);
		return;
	}

	updateUninstallButton();

	std::shared_ptr<ModGameVersion> selectedModGameVersion(m_modManager->getSelectedModGameVersion());

	if(selectedModGameVersion != nullptr && selectedModGameVersion->isStandAlone()) {
		m_modGameVersionListLabel->SetLabelText("Mod Game Versions");
	}
	else {
		m_modGameVersionListLabel->SetLabelText("Compatible Mod Game Versions");
	}

	m_launchButton->SetLabel("Launch Mod");
	m_launchButton->Enable();
}

void ModBrowserPanel::onModListRightClicked(wxMouseEvent & event) {
	std::shared_ptr<OrganizedModCollection> organizedMods(m_modManager->getOrganizedMods());
	std::shared_ptr<FavouriteModCollection> favouriteMods(m_modManager->getFavouriteMods());

	m_modPopupMenuItemIndex = m_modListBox->HitTest(event.GetPosition());

	if(m_modPopupMenuItemIndex != wxNOT_FOUND) {
		if(m_searchQuery.empty()) {
			if(organizedMods->shouldDisplayMods()) {
				std::shared_ptr<Mod> mod(organizedMods->getMod(m_modPopupMenuItemIndex));
				bool hasModFavourited = favouriteMods->hasFavourite(mod->getName());

				m_modListAddFavouriteMenuItem->Enable(!hasModFavourited);
				m_modListRemoveFavouriteMenuItem->Enable(hasModFavourited);
			}
			else if(organizedMods->shouldDisplayFavouriteMods()) {
				m_modListAddFavouriteMenuItem->Enable(false);
				m_modListRemoveFavouriteMenuItem->Enable(true);
			}
			else {
				m_modListAddFavouriteMenuItem->Enable(false);
				m_modListRemoveFavouriteMenuItem->Enable(false);
			}
		}
		else {
			if(organizedMods->shouldDisplayMods() && m_modPopupMenuItemIndex < m_modMatches.size()) {
				bool hasSearchResultFavourited = favouriteMods->hasFavourite(m_modMatches[m_modPopupMenuItemIndex]);

				m_modListAddFavouriteMenuItem->Enable(!hasSearchResultFavourited);
				m_modListRemoveFavouriteMenuItem->Enable(hasSearchResultFavourited);
			}
			else {
				m_modListAddFavouriteMenuItem->Enable(false);
				m_modListRemoveFavouriteMenuItem->Enable(false);
			}
		}
	}
	else {
		m_modListAddFavouriteMenuItem->Enable(false);
		m_modListRemoveFavouriteMenuItem->Enable(false);
	}

	m_modListBox->PopupMenu(m_modListPopupMenu.get());
}

void ModBrowserPanel::onModVersionListRightClicked(wxMouseEvent & event) {
	m_modVersionPopupMenuItemIndex = m_modVersionListBox->HitTest(event.GetPosition());
	m_modVersionListAddFavouriteMenuItem->Enable(m_modVersionPopupMenuItemIndex != wxNOT_FOUND);

	m_modVersionListBox->PopupMenu(m_modVersionListPopupMenu.get());
}

void ModBrowserPanel::onModVersionTypeListRightClicked(wxMouseEvent & event) {
	m_modVersionTypePopupMenuItemIndex = m_modVersionTypeListBox->HitTest(event.GetPosition());
	m_modVersionTypeListAddFavouriteMenuItem->Enable(m_modVersionTypePopupMenuItemIndex != wxNOT_FOUND);

	m_modVersionTypeListBox->PopupMenu(m_modVersionTypeListPopupMenu.get());
}

void ModBrowserPanel::onModPopupMenuItemPressed(wxCommandEvent & event) {
	std::shared_ptr<OrganizedModCollection> organizedMods(m_modManager->getOrganizedMods());
	std::shared_ptr<FavouriteModCollection> favouriteMods(m_modManager->getFavouriteMods());

	if(m_modPopupMenuItemIndex == wxNOT_FOUND) {
		return;
	}

	if(event.GetId() == m_modListAddFavouriteMenuItem->GetId()) {
		if(m_searchQuery.empty()) {
			if(organizedMods->shouldDisplayMods()) {
				if(m_modPopupMenuItemIndex < organizedMods->numberOfMods()) {
					if(favouriteMods->addFavourite(organizedMods->getMod(m_modPopupMenuItemIndex)->getName())) {
						spdlog::info("Added '{}' to favoruite mods list.", organizedMods->getMod(m_modPopupMenuItemIndex)->getName());
					}
				}
			}
		}
		else {
			if(organizedMods->shouldDisplayMods() && m_modPopupMenuItemIndex < m_modMatches.size()) {
				std::string modMatchString(m_modMatches[m_modPopupMenuItemIndex].toString());

				if(favouriteMods->addFavourite(m_modMatches[m_modPopupMenuItemIndex])) {
					spdlog::info("Added '{}' to favourite mods list.", modMatchString);
				}
			}
		}
	}
	else if(event.GetId() == m_modListRemoveFavouriteMenuItem->GetId()) {
		if(m_searchQuery.empty()) {
			if(organizedMods->shouldDisplayMods()) {
				if(m_modPopupMenuItemIndex < organizedMods->numberOfMods()) {
					if(favouriteMods->removeFavourite(organizedMods->getMod(m_modPopupMenuItemIndex)->getName())) {
						spdlog::info("Removed '{}' from favourite mods list.", organizedMods->getMod(m_modPopupMenuItemIndex)->getName());
					}
				}
			}
			else if(organizedMods->shouldDisplayFavouriteMods()) {
				if(m_modPopupMenuItemIndex < organizedMods->numberOfFavouriteMods()) {
					std::shared_ptr<ModIdentifier> favouriteMod(organizedMods->getFavouriteMod(m_modPopupMenuItemIndex));

					if(favouriteMods->removeFavourite(*favouriteMod)) {
						spdlog::info("Removed '{}' from favourite mods list.", favouriteMod->toString());
					}
				}
			}
		}
		else {
			if(organizedMods->shouldDisplayMods() && m_modPopupMenuItemIndex < m_modMatches.size()) {
				std::string modMatchString(m_modMatches[m_modPopupMenuItemIndex].toString());

				if(favouriteMods->removeFavourite(m_modMatches[m_modPopupMenuItemIndex])) {
					spdlog::info("Removed '{}' from favourite mods list.", modMatchString);
				}
			}
		}
	}
}

void ModBrowserPanel::onModVersionPopupMenuItemPressed(wxCommandEvent & event) {
	if(event.GetId() == m_modVersionListAddFavouriteMenuItem->GetId()) {
		std::shared_ptr<Mod> selectedMod(m_modManager->getSelectedMod());

		if(selectedMod == nullptr || m_modVersionPopupMenuItemIndex == wxNOT_FOUND || m_modVersionPopupMenuItemIndex >= selectedMod->numberOfVersions()) {
			return;
		}

		m_modManager->getFavouriteMods()->addFavourite(ModIdentifier(selectedMod->getName(), selectedMod->getVersion(m_modVersionPopupMenuItemIndex)->getVersion()));
	}
}

void ModBrowserPanel::onModVersionTypePopupMenuItemPressed(wxCommandEvent & event) {
	if(event.GetId() == m_modVersionTypeListAddFavouriteMenuItem->GetId()) {
		std::shared_ptr<Mod> selectedMod(m_modManager->getSelectedMod());

		if(selectedMod == nullptr) {
			return;
		}

		std::shared_ptr<ModVersion> selectedModVersion(m_modManager->getSelectedModVersion());

		if(selectedModVersion == nullptr || m_modVersionTypePopupMenuItemIndex == wxNOT_FOUND || m_modVersionTypePopupMenuItemIndex >= selectedModVersion->numberOfTypes()) {
			return;
		}

		m_modManager->getFavouriteMods()->addFavourite(ModIdentifier(selectedMod->getName(), selectedModVersion->getVersion(), selectedModVersion->getType(m_modVersionTypePopupMenuItemIndex)->getType()));
	}
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

void ModBrowserPanel::onUninstallButtonPressed(wxCommandEvent & event) {
	std::shared_ptr<ModGameVersion> selectedModGameVersion(m_modManager->getSelectedModGameVersion());

	if(selectedModGameVersion == nullptr) {
		return;
	}

	if(selectedModGameVersion->isStandAlone()) {
		std::shared_ptr<StandAloneMod> standAloneMod(m_modManager->getStandAloneMods()->getStandAloneMod(*selectedModGameVersion));

		std::string standAloneModFullName(standAloneMod->getLongName() + (standAloneMod->getVersion().empty() ? "" : " " + standAloneMod->getVersion()));

		if(!standAloneMod->isConfigured()) {
			spdlog::error("Stand-alone '{}' mod is not configured.", standAloneModFullName);
			return;
		}

		int uninstallConfirmationResult = wxMessageBox(fmt::format("Are you sure you want to uninstall stand-alone '{}' mod from '{}'? This will remove all files located in the directory, so be sure to back up any valuable data!", standAloneModFullName, standAloneMod->getGamePath()), "Uninstall Confirmation", wxYES_NO | wxCANCEL | wxICON_QUESTION, this);

		if(uninstallConfirmationResult != wxYES) {
			return;
		}
	}

	if(m_modManager->uninstallModGameVersion(*selectedModGameVersion)) {
		updateUninstallButton();
	}
}

void ModBrowserPanel::onLaunchButtonPressed(wxCommandEvent & event) {
	std::shared_ptr<ModGameVersion> selectedModGameVersion(m_modManager->getSelectedModGameVersion());

	if(selectedModGameVersion != nullptr &&
	   selectedModGameVersion->isStandAlone() &&
	   !m_modManager->getStandAloneMods()->hasStandAloneMod(*selectedModGameVersion)) {
		installStandAloneMod(selectedModGameVersion);
		return;
	}

	launchGame();
}

void ModBrowserPanel::onLaunched() {
	updateUninstallButton();

	m_gameRunningDialog->setProcess(m_modManager->getGameProcess());
}

void ModBrowserPanel::onLaunchStatus(const std::string & statusMessage) {
	if(m_gameRunningDialog == nullptr) {
		return;
	}

	m_gameRunningDialog->setStatus(statusMessage);
}

void ModBrowserPanel::onLaunchError(const std::string & errorMessage) {
	std::shared_ptr<Mod> selectedMod(m_modManager->getSelectedMod());
	std::shared_ptr<ModGameVersion> selectedModGameVersion(m_modManager->getSelectedModGameVersion());

	std::string fullModName(selectedMod != nullptr ? selectedMod->getFullName(m_modManager->getSelectedModVersionIndex(), m_modManager->getSelectedModVersionTypeIndex()) : "");

	wxMessageBox(
		fmt::format(
			"Failed to launch '{}'{}!\n"
			"\n"
			"{}\n"
			"\n"
			"See console for more details.",
			m_activeGameVersion->getLongName(),
			!fullModName.empty() && selectedModGameVersion != nullptr && !selectedModGameVersion->isStandAlone() ? fmt::format(" with mod '{}'", fullModName) : "",
			errorMessage
		),
		"Launch Failed",
		wxOK | wxICON_ERROR,
		this
	);

	m_activeGameVersion.reset();

	QueueEvent(new LaunchFailedEvent());
}

void ModBrowserPanel::onGameProcessTerminated(uint64_t nativeExitCode, bool forceTerminated) {
	m_activeGameVersion.reset();

	QueueEvent(new GameProcessTerminatedEvent());
}

void ModBrowserPanel::onLaunchFailed(LaunchFailedEvent & launchFailedEvent) {
	m_gameRunningDialog->Destroy();
	m_gameRunningDialog = nullptr;
}

void ModBrowserPanel::onGameProcessEnded(GameProcessTerminatedEvent & gameProcessTerminatedEvent) { }

void ModBrowserPanel::onModInstallProgress(ModInstallProgressEvent & event) {
	if(m_installModProgressDialog == nullptr) {
		return;
	}

	bool updateResult = m_installModProgressDialog->Update(event.getValue(), event.getMessage());
	m_installModProgressDialog->Fit();

	if(!updateResult) {
		m_modInstallationCancelled = true;
	}
}

void ModBrowserPanel::onModInstallDone(ModInstallDoneEvent & event) {
	if(m_installModProgressDialog == nullptr) {
		return;
	}

	m_installModProgressDialog->Destroy();
	m_installModProgressDialog = nullptr;

	if(event.wasSuccessful()) {
		// Note: Directly launching the game from here results in the process running dialog not longer blocking user input into the main window.

		updateLaunchButton();
	}
}

void ModBrowserPanel::onModSelectionChanged(std::shared_ptr<Mod> mod, size_t modVersionIndex, size_t modVersionTypeIndex, size_t modGameVersionIndex) {
	updateModSelection();
	updateModListSortOptions();

	m_clearButton->Enable();
}

void ModBrowserPanel::onGameTypeChanged(GameType gameType) {
	updateModGameType();
	updateDOSBoxServerSettings();
}

void ModBrowserPanel::onPreferredDOSBoxVersionChanged(std::shared_ptr<DOSBoxVersion> dosboxVersion) {
	updatePreferredDOSBoxVersion();
}

void ModBrowserPanel::onPreferredGameVersionChanged(std::shared_ptr<GameVersion> gameVersion) {
	updatePreferredGameVersion();
}

void ModBrowserPanel::onDOSBoxServerIPAddressChanged(std::string ipAddress) {
	m_ipAddressTextField->SetValue(ipAddress);
}

void ModBrowserPanel::onDOSBoxLocalServerPortChanged(uint16_t port) {
	if(m_modManager->getGameType() == GameType::Server) {
		m_portTextField->SetValue(std::to_string(port));
	}
}

void ModBrowserPanel::onDOSBoxRemoteServerPortChanged(uint16_t port) {
	if(m_modManager->getGameType() == GameType::Client) {
		m_portTextField->SetValue(std::to_string(port));
	}
}

void ModBrowserPanel::onFilterTypeChanged(OrganizedModCollection::FilterType filterType) {
	m_modManager->getOrganizedMods()->clearSelectedItems();

	updateModListFilterType();
	updateModListSortOptions();
	updateModList();
}

void ModBrowserPanel::onSortOptionsChanged(OrganizedModCollection::SortType sortType, OrganizedModCollection::SortDirection sortDirection) {
	updateModListSortOptions();
	updateModList();
}

void ModBrowserPanel::onSelectedModChanged(std::shared_ptr<Mod> mod) {
	updateModSelection();
	updateModListSortOptions();
}

void ModBrowserPanel::onSelectedFavouriteModChanged(std::shared_ptr<ModIdentifier> favouriteMod) {
	updateModSelection();
	updateModListSortOptions();
}

void ModBrowserPanel::onSelectedGameVersionChanged(std::shared_ptr<GameVersion> gameVersion) {
	updateModSelection();
	updateModListSortOptions();
}

void ModBrowserPanel::onSelectedTeamChanged(std::shared_ptr<ModAuthorInformation> team) {
	updateModSelection();
	updateModListSortOptions();
}

void ModBrowserPanel::onSelectedAuthorChanged(std::shared_ptr<ModAuthorInformation> author) {
	updateModSelection();
	updateModListSortOptions();
}

void ModBrowserPanel::onOrganizedModCollectionChanged(const std::vector<std::shared_ptr<Mod>> & organizedMods) {
	updateModList();
}

void ModBrowserPanel::onOrganizedFavouriteModCollectionChanged(const std::vector<std::shared_ptr<ModIdentifier>> & organizedFavouriteMods) {
	updateModList();
}

void ModBrowserPanel::onOrganizedModGameVersionCollectionChanged(const std::vector<std::shared_ptr<GameVersion>> & organizedGameVersions) {
	updateModList();
}

void ModBrowserPanel::onOrganizedModTeamCollectionChanged(const std::vector<std::shared_ptr<ModAuthorInformation>> & organizedTeams) {
	updateModList();
}

void ModBrowserPanel::onOrganizedModAuthorCollectionChanged(const std::vector<std::shared_ptr<ModAuthorInformation>> & organizedAuthors) {
	updateModList();
}

void ModBrowserPanel::onDOSBoxVersionCollectionSizeChanged(DOSBoxVersionCollection & dosboxVersionCollection) {
	updatePreferredDOSBoxVersionList();
}

void ModBrowserPanel::onDOSBoxVersionCollectionItemModified(DOSBoxVersionCollection & dosboxVersionCollection, DOSBoxVersion & dosboxVersion) {
	updatePreferredDOSBoxVersionList();
}

void ModBrowserPanel::onGameVersionCollectionSizeChanged(GameVersionCollection & gameVersionCollection) {
	updatePreferredGameVersionList();
}

void ModBrowserPanel::onGameVersionCollectionItemModified(GameVersionCollection & gameVersionCollection, GameVersion & gameVersion) {
	updatePreferredGameVersionList();
}

void ModBrowserPanel::onModSelectionRequested(const std::string & modID) {
	clearSearch();

	std::shared_ptr<OrganizedModCollection> organizedMods(m_modManager->getOrganizedMods());
	organizedMods->clearSelectedItems();
	organizedMods->setFilterType(OrganizedModCollection::FilterType::None);

	if(!organizedMods->setSelectedModByID(modID)) {
		spdlog::error("Failed to select mod with ID: '{}'.", modID);
	}
}

void ModBrowserPanel::onModTeamSelectionRequested(const std::string & modTeamName) {
	clearSearch();

	std::shared_ptr<OrganizedModCollection> organizedMods(m_modManager->getOrganizedMods());
	organizedMods->clearSelectedItems();
	organizedMods->setFilterType(OrganizedModCollection::FilterType::Teams);

	if(!organizedMods->setSelectedTeam(modTeamName)) {
		spdlog::error("Failed to select mod team with name: '{}'.", modTeamName);
	}
}

void ModBrowserPanel::onModTeamMemberSelectionRequested(const std::string & modTeamMemberName) {
	clearSearch();

	std::shared_ptr<OrganizedModCollection> organizedMods(m_modManager->getOrganizedMods());
	organizedMods->clearSelectedItems();
	organizedMods->setFilterType(OrganizedModCollection::FilterType::Authors);

	if(!organizedMods->setSelectedAuthor(modTeamMemberName)) {
		spdlog::error("Failed to select mod team member with name: '{}'.", modTeamMemberName);
	}
}

void ModBrowserPanel::onModVersionTypeSelectionRequested(const std::string & modID, const std::string & modVersion, const std::string & modVersionType) {
	clearSearch();

	std::shared_ptr<OrganizedModCollection> organizedMods(m_modManager->getOrganizedMods());
	organizedMods->clearSelectedItems();
	organizedMods->setFilterType(OrganizedModCollection::FilterType::None);

	if(!m_modManager->setSelectedMod(modID, modVersion, modVersionType) || !organizedMods->setSelectedModByID(modID)) {
		spdlog::error("Failed to select mod version type with ID: '{}', version: '{}', and type: '{}'.", modID, modVersion, modVersionType);
	}
}
