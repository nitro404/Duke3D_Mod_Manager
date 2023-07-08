#include "DOSBoxManagerPanel.h"

#include "DOSBox/DOSBoxManager.h"
#include "Manager/SettingsManager.h"
#include "SettingPanel.h"
#include "WXUtilities.h"

#include <Analytics/Segment/SegmentAnalytics.h>
#include <Network/HTTPRequest.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/gbsizer.h>
#include <wx/progdlg.h>
#include <wx/textdlg.h>
#include <wx/wrapsizer.h>

#include <any>
#include <map>
#include <filesystem>
#include <sstream>

wxDECLARE_EVENT(EVENT_DOSBOX_INSTALL_PROGRESS, DOSBoxInstallProgressEvent);

class DOSBoxInstallProgressEvent final : public wxEvent {
public:
	DOSBoxInstallProgressEvent(int value = 0, const std::string & message = {})
		: wxEvent(0, EVENT_DOSBOX_INSTALL_PROGRESS)
		, m_value(value)
		, m_message(message) { }

	virtual ~DOSBoxInstallProgressEvent() { }

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
		return new DOSBoxInstallProgressEvent(*this);
	}

	DECLARE_DYNAMIC_CLASS(DOSBoxInstallProgressEvent);

private:
	int m_value;
	std::string m_message;
};

IMPLEMENT_DYNAMIC_CLASS(DOSBoxInstallProgressEvent, wxEvent);

wxDECLARE_EVENT(EVENT_DOSBOX_INSTALL_DONE, DOSBoxInstallDoneEvent);

class DOSBoxInstallDoneEvent final : public wxEvent {
public:
	DOSBoxInstallDoneEvent()
		: wxEvent(0, EVENT_DOSBOX_INSTALL_DONE) { }

	virtual ~DOSBoxInstallDoneEvent() { }

	virtual wxEvent * Clone() const override {
		return new DOSBoxInstallDoneEvent(*this);
	}

	DECLARE_DYNAMIC_CLASS(DOSBoxInstallDoneEvent);
};

IMPLEMENT_DYNAMIC_CLASS(DOSBoxInstallDoneEvent, wxEvent);

wxDEFINE_EVENT(EVENT_DOSBOX_INSTALL_PROGRESS, DOSBoxInstallProgressEvent);
wxDEFINE_EVENT(EVENT_DOSBOX_INSTALL_DONE, DOSBoxInstallDoneEvent);

DOSBoxManagerPanel::DOSBoxManagerPanel(std::shared_ptr<ModManager> modManager, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "DOSBox Manager")
	, m_modManager(modManager)
	, m_notebook(nullptr)
	, m_newDOSBoxVersionButton(nullptr)
	, m_installDOSBoxVersionButton(nullptr)
	, m_uninstallDOSBoxVersionButton(nullptr)
	, m_saveDOSBoxVersionButton(nullptr)
	, m_discardDOSBoxVersionChangesButton(nullptr)
	, m_resetDOSBoxVersionButton(nullptr)
	, m_removeDOSBoxVersionButton(nullptr)
	, m_saveDOSBoxSettingsButton(nullptr)
	, m_discardDOSBoxSettingsChangesButton(nullptr)
	, m_dosboxSettingsPanel(nullptr)
	, m_installProgressDialog(nullptr)
	, m_modified(false) {
	m_notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP | wxNB_MULTILINE, "DOSBox Versions");
	m_notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &DOSBoxManagerPanel::onNotebookPageChanged, this);

	std::shared_ptr<DOSBoxManager> dosboxManager(m_modManager->getDOSBoxManager());

	wxPanel * dosboxVersionActionsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "DOSBox Version Actions");

	m_newDOSBoxVersionButton = new wxButton(dosboxVersionActionsPanel, wxID_ANY, "New", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Create New DOSBox Version");
	m_newDOSBoxVersionButton->Bind(wxEVT_BUTTON, &DOSBoxManagerPanel::onNewDOSBoxVersionButtonPressed, this);

	m_installDOSBoxVersionButton = new wxButton(dosboxVersionActionsPanel, wxID_ANY, "Install", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Install DOSBox Version");
	m_installDOSBoxVersionButton->Bind(wxEVT_BUTTON, &DOSBoxManagerPanel::onInstallDOSBoxVersionButtonPressed, this);
	m_installDOSBoxVersionButton->Disable();

	m_uninstallDOSBoxVersionButton = new wxButton(dosboxVersionActionsPanel, wxID_ANY, "Uninstall", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Uninstall DOSBox Version");
	m_uninstallDOSBoxVersionButton->Bind(wxEVT_BUTTON, &DOSBoxManagerPanel::onUninstallDOSBoxVersionButtonPressed, this);
	m_uninstallDOSBoxVersionButton->Disable();

	m_resetDOSBoxVersionButton = new wxButton(dosboxVersionActionsPanel, wxID_ANY, "Reset", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Reset DOSBox Version Settings");
	m_resetDOSBoxVersionButton->Bind(wxEVT_BUTTON, &DOSBoxManagerPanel::onResetDOSBoxVersionButtonPressed, this);
	m_resetDOSBoxVersionButton->Disable();

	m_saveDOSBoxVersionButton = new wxButton(dosboxVersionActionsPanel, wxID_ANY, "Save", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Save DOSBox Version Configuration");
	m_saveDOSBoxVersionButton->Bind(wxEVT_BUTTON, &DOSBoxManagerPanel::onSaveDOSBoxVersionButtonPressed, this);
	m_saveDOSBoxVersionButton->Disable();

	m_discardDOSBoxVersionChangesButton = new wxButton(dosboxVersionActionsPanel, wxID_ANY, "Discard Changes", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Discard DOSBox Version Configuration Changes");
	m_discardDOSBoxVersionChangesButton->Bind(wxEVT_BUTTON, &DOSBoxManagerPanel::onDiscardDOSBoxVersionChangesButtonPressed, this);
	m_discardDOSBoxVersionChangesButton->Disable();

	m_removeDOSBoxVersionButton = new wxButton(dosboxVersionActionsPanel, wxID_ANY, "Remove", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Remove DOSBox Version");
	m_removeDOSBoxVersionButton->Bind(wxEVT_BUTTON, &DOSBoxManagerPanel::onRemoveDOSBoxVersionButtonPressed, this);
	m_removeDOSBoxVersionButton->Disable();

	m_dosboxSettingsPanel = new DOSBoxSettingsPanel(modManager, this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "General DOSBox Settings");
	m_dosboxSettingModifiedConnection = m_dosboxSettingsPanel->dosboxSettingModified.connect(std::bind(&DOSBoxManagerPanel::onDOSBoxSettingModified, this, std::placeholders::_1));

	wxPanel * dosboxSettingsActionsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "DOSBox Settings Actions");

	m_saveDOSBoxSettingsButton = new wxButton(dosboxSettingsActionsPanel, wxID_ANY, "Save Settings", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Save Settings");
	m_saveDOSBoxSettingsButton->Bind(wxEVT_BUTTON, &DOSBoxManagerPanel::onSaveDOSBoxSettingsButtonPressed, this);
	m_saveDOSBoxSettingsButton->Disable();

	m_discardDOSBoxSettingsChangesButton = new wxButton(dosboxSettingsActionsPanel, wxID_ANY, "Discard Changes", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Discard Changes");
	m_discardDOSBoxSettingsChangesButton->Bind(wxEVT_BUTTON, &DOSBoxManagerPanel::onDiscardDOSBoxSettingsChangesButtonPressed, this);
	m_discardDOSBoxSettingsChangesButton->Disable();

	wxButton * resetDefaultDOSBoxSettingsButton = new wxButton(dosboxSettingsActionsPanel, wxID_ANY, "Reset Defaults", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Reset Defaults");
	resetDefaultDOSBoxSettingsButton->Bind(wxEVT_BUTTON, &DOSBoxManagerPanel::onResetDefaultDOSBoxSettingsButtonPressed, this);

	int border = 2;

	wxWrapSizer * dosboxVersionActionsPanelSizer = new wxWrapSizer(wxHORIZONTAL);
	dosboxVersionActionsPanelSizer->Add(m_newDOSBoxVersionButton, 1, wxEXPAND | wxHORIZONTAL, border);
	dosboxVersionActionsPanelSizer->Add(m_installDOSBoxVersionButton, 1, wxEXPAND | wxHORIZONTAL, border);
	dosboxVersionActionsPanelSizer->Add(m_uninstallDOSBoxVersionButton, 1, wxEXPAND | wxHORIZONTAL, border);
	dosboxVersionActionsPanelSizer->Add(m_resetDOSBoxVersionButton, 1, wxEXPAND | wxHORIZONTAL, border);
	dosboxVersionActionsPanelSizer->Add(m_saveDOSBoxVersionButton, 1, wxEXPAND | wxHORIZONTAL, border);
	dosboxVersionActionsPanelSizer->Add(m_discardDOSBoxVersionChangesButton, 1, wxEXPAND | wxHORIZONTAL, border);
	dosboxVersionActionsPanelSizer->Add(m_removeDOSBoxVersionButton, 1, wxEXPAND | wxHORIZONTAL, border);
	dosboxVersionActionsPanel->SetSizerAndFit(dosboxVersionActionsPanelSizer);

	wxWrapSizer * dosboxSettingsActionsPanelSizer = new wxWrapSizer(wxHORIZONTAL);
	dosboxSettingsActionsPanelSizer->Add(m_saveDOSBoxSettingsButton, 1, wxEXPAND | wxHORIZONTAL, border);
	dosboxSettingsActionsPanelSizer->Add(m_discardDOSBoxSettingsChangesButton, 1, wxEXPAND | wxHORIZONTAL, border);
	dosboxSettingsActionsPanelSizer->Add(resetDefaultDOSBoxSettingsButton, 1, wxEXPAND | wxHORIZONTAL, border);
	dosboxSettingsActionsPanel->SetSizerAndFit(dosboxSettingsActionsPanelSizer);

	wxGridBagSizer * dosboxManagerPanelSizer = new wxGridBagSizer(border, border);
	dosboxManagerPanelSizer->Add(m_notebook, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	dosboxManagerPanelSizer->Add(dosboxVersionActionsPanel, wxGBPosition(1, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	dosboxManagerPanelSizer->Add(m_dosboxSettingsPanel, wxGBPosition(2, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	dosboxManagerPanelSizer->Add(dosboxSettingsActionsPanel, wxGBPosition(3, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	dosboxManagerPanelSizer->AddGrowableCol(0, 1);
	dosboxManagerPanelSizer->AddGrowableRow(0, 1);
	dosboxManagerPanelSizer->AddGrowableRow(2, 1);
	SetSizerAndFit(dosboxManagerPanelSizer);

	DOSBoxVersionPanel * dosboxVersionPanel = nullptr;

	for(const std::shared_ptr<DOSBoxVersion> & dosboxVersion : dosboxManager->getDOSBoxVersions()->getDOSBoxVersions()) {
		dosboxVersionPanel = new DOSBoxVersionPanel(dosboxVersion, m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

		m_dosboxVersionPanelSignalConnectionGroups.push_back(SignalConnectionGroup(
			dosboxVersionPanel->dosboxVersionSettingChanged.connect(std::bind(&DOSBoxManagerPanel::onDOSBoxVersionSettingChanged, this, std::placeholders::_1, std::placeholders::_2)),
			dosboxVersionPanel->dosboxVersionChangesDiscarded.connect(std::bind(&DOSBoxManagerPanel::onDOSBoxVersionChangesDiscarded, this, std::placeholders::_1)),
			dosboxVersionPanel->dosboxVersionReset.connect(std::bind(&DOSBoxManagerPanel::onDOSBoxVersionReset, this, std::placeholders::_1)),
			dosboxVersionPanel->dosboxVersionSaved.connect(std::bind(&DOSBoxManagerPanel::onDOSBoxVersionSaved, this, std::placeholders::_1))
		));

		addDOSBoxVersionPanel(dosboxVersionPanel);
	}

	update();

	Bind(EVENT_DOSBOX_INSTALL_PROGRESS, &DOSBoxManagerPanel::onInstallProgress, this);
	Bind(EVENT_DOSBOX_INSTALL_DONE, &DOSBoxManagerPanel::onInstallDone, this);
}

DOSBoxManagerPanel::~DOSBoxManagerPanel() {
	m_dosboxSettingModifiedConnection.disconnect();

	for(SignalConnectionGroup & dosboxVersionPanelSignalConnectionGroup : m_dosboxVersionPanelSignalConnectionGroups) {
		dosboxVersionPanelSignalConnectionGroup.disconnect();
	}
}

bool DOSBoxManagerPanel::hasDOSBoxVersionPanel(const DOSBoxVersionPanel * dosboxVersionPanel) const {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(m_notebook->GetPage(i) == dosboxVersionPanel) {
			return true;
		}
	}

	return false;
}

bool DOSBoxManagerPanel::hasPanelWithDOSBoxVersion(const DOSBoxVersion * dosboxVersion) const {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(getDOSBoxVersion(i).get() == dosboxVersion) {
			return true;
		}
	}

	return false;
}

bool DOSBoxManagerPanel::hasPanelWithDOSBoxVersionID(const std::string & dosboxVersionID) const {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(getDOSBoxVersion(i)->getID(), dosboxVersionID)) {
			return true;
		}
	}

	return false;
}

size_t DOSBoxManagerPanel::indexOfDOSBoxVersionPanel(const DOSBoxVersionPanel * dosboxVersionPanel) const {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(m_notebook->GetPage(i) == dosboxVersionPanel) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t DOSBoxManagerPanel::indexOfPanelWithDOSBoxVersion(const DOSBoxVersion * dosboxVersion) const {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(getDOSBoxVersion(i).get() == dosboxVersion) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t DOSBoxManagerPanel::indexOfPanelWithDOSBoxVersionID(const std::string & dosboxVersionID) const {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(getDOSBoxVersion(i)->getID(), dosboxVersionID)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

DOSBoxVersionPanel * DOSBoxManagerPanel::getDOSBoxVersionPanel(size_t dosboxVersionPanelIndex) const {
	if(dosboxVersionPanelIndex >= m_notebook->GetPageCount()) {
		return nullptr;
	}

	return dynamic_cast<DOSBoxVersionPanel *>(m_notebook->GetPage(dosboxVersionPanelIndex));
}

DOSBoxVersionPanel * DOSBoxManagerPanel::getPanelWithDOSBoxVersion(const DOSBoxVersion * dosboxVersion) const {
	return getDOSBoxVersionPanel(indexOfPanelWithDOSBoxVersion(dosboxVersion));
}

DOSBoxVersionPanel * DOSBoxManagerPanel::getPanelWithDOSBoxVersionID(const std::string & dosboxVersionID) const {
	return getDOSBoxVersionPanel(indexOfPanelWithDOSBoxVersionID(dosboxVersionID));
}

DOSBoxVersionPanel * DOSBoxManagerPanel::getCurrentDOSBoxVersionPanel() const {
	return dynamic_cast<DOSBoxVersionPanel *>(m_notebook->GetCurrentPage());
}

std::shared_ptr<DOSBoxVersion> DOSBoxManagerPanel::getDOSBoxVersion(size_t dosboxVersionPanelIndex) const {
	DOSBoxVersionPanel * dosboxVersionPanel = getDOSBoxVersionPanel(dosboxVersionPanelIndex);

	if(dosboxVersionPanel == nullptr) {
		return nullptr;
	}

	return dosboxVersionPanel->getDOSBoxVersion();
}

std::shared_ptr<DOSBoxVersion> DOSBoxManagerPanel::getCurrentDOSBoxVersion() const {
	DOSBoxVersionPanel * dosboxVersionPanel = getCurrentDOSBoxVersionPanel();

	if(dosboxVersionPanel == nullptr) {
		return nullptr;
	}

	return dosboxVersionPanel->getDOSBoxVersion();
}

bool DOSBoxManagerPanel::selectDOSBoxVersionPanel(size_t dosboxVersionPanelIndex) {
	if(dosboxVersionPanelIndex >= m_notebook->GetPageCount()) {
		return false;
	}

	m_notebook->ChangeSelection(dosboxVersionPanelIndex);

	updateButtons();

	return true;
}

bool DOSBoxManagerPanel::selectPanelWithDOSBoxVersion(const DOSBoxVersion & dosboxVersion) {
	size_t dosboxVersionPanelIndex = indexOfPanelWithDOSBoxVersion(&dosboxVersion);

	if(dosboxVersionPanelIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	m_notebook->ChangeSelection(dosboxVersionPanelIndex);

	updateButtons();

	return true;
}

bool DOSBoxManagerPanel::selectPanelWithDOSBoxVersionID(const std::string & dosboxVersionID) {
	size_t dosboxVersionPanelIndex = indexOfPanelWithDOSBoxVersionID(dosboxVersionID);

	if(dosboxVersionPanelIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	m_notebook->ChangeSelection(dosboxVersionPanelIndex);

	updateButtons();

	return true;
}

bool DOSBoxManagerPanel::saveDOSBoxVersions() {
	SettingsManager * settings = SettingsManager::getInstance();

	if(!m_modManager->getDOSBoxVersions()->saveTo(settings->dosboxVersionsListFilePath)) {
		spdlog::error("Failed to save dosbox versions to: '{}'!", settings->dosboxVersionsListFilePath);
		return false;
	}

	spdlog::info("DOSBox versions saved to: '{}'.", settings->dosboxVersionsListFilePath);

	return true;
}

bool DOSBoxManagerPanel::areDOSBoxSettingsModified() const {
	return m_dosboxSettingsPanel->areSettingsModified();
}

bool DOSBoxManagerPanel::areDOSBOxSettingsValid() const {
	return m_dosboxSettingsPanel->areSettingsValid();
}

void DOSBoxManagerPanel::resetDefaultDOSBoxSettings() {
	m_dosboxSettingsPanel->resetSettings();

	m_modified = false;

	updateButtons();
	dosboxSettingsReset();

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		SegmentAnalytics::getInstance()->track("DOSBox Settings Reset");
	}
}

void DOSBoxManagerPanel::discardDOSBoxSettings() {
	m_dosboxSettingsPanel->discardSettings();

	m_modified = false;

	updateButtons();
	dosboxSettingsDiscarded();
}

bool DOSBoxManagerPanel::saveDOSBoxSettings() {
	if(!m_dosboxSettingsPanel->saveSettings()) {
		return false;
	}

	m_modified = false;

	updateButtons();
	dosboxSettingsSaved();

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		SegmentAnalytics::getInstance()->track("DOSBox Settings Saved");
	}

	return true;
}

void DOSBoxManagerPanel::update() {
	updateButtons();
	updateDOSBoxVersionPanelNames();
}

bool DOSBoxManagerPanel::updateDOSBoxVersionPanel(size_t dosboxVersionPanelIndex) {
	DOSBoxVersionPanel * dosboxVersionPanel = getDOSBoxVersionPanel(dosboxVersionPanelIndex);

	if(dosboxVersionPanel == nullptr) {
		return false;
	}

	updateButtons();
	updateDOSBoxVersionPanelName(dosboxVersionPanelIndex);

	return true;
}

void DOSBoxManagerPanel::updateDOSBoxVersionPanelNames() {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		updateDOSBoxVersionPanelName(i);
	}
}

bool DOSBoxManagerPanel::updateDOSBoxVersionPanelName(size_t dosboxVersionPanelIndex) {
	DOSBoxVersionPanel * dosboxVersionPanel = getDOSBoxVersionPanel(dosboxVersionPanelIndex);

	if(dosboxVersionPanel == nullptr) {
		return false;
	}

	m_notebook->SetPageText(dosboxVersionPanelIndex, wxString::FromUTF8(dosboxVersionPanel->getPanelName()));

	return true;
}

void DOSBoxManagerPanel::updateButtons() {
	DOSBoxVersionPanel * dosboxVersionPanel = getCurrentDOSBoxVersionPanel();
	std::shared_ptr<DOSBoxVersion> dosboxVersion = dosboxVersionPanel != nullptr ? dosboxVersionPanel->getDOSBoxVersion() : nullptr;

	bool isDOSBoxVersionModified = dosboxVersionPanel != nullptr ? dosboxVersionPanel->isModified() : false;
	bool isDOSBoxVersionInstallable = dosboxVersion != nullptr ? DOSBoxManager::isDOSBoxVersionDownloadable(*dosboxVersion) : false;
	bool isDOSBoxVersionInstalled = dosboxVersion != nullptr ? dosboxVersion->hasDirectoryPath() : false;
	bool isDOSBoxVersionRemovable = dosboxVersion != nullptr ? dosboxVersion->isRemovable() : false;

	WXUtilities::setButtonEnabled(m_installDOSBoxVersionButton, isDOSBoxVersionInstallable && !isDOSBoxVersionInstalled);
	WXUtilities::setButtonEnabled(m_uninstallDOSBoxVersionButton, isDOSBoxVersionInstalled);
	WXUtilities::setButtonEnabled(m_saveDOSBoxVersionButton, isDOSBoxVersionModified);
	WXUtilities::setButtonEnabled(m_discardDOSBoxVersionChangesButton, isDOSBoxVersionModified);
	WXUtilities::setButtonEnabled(m_resetDOSBoxVersionButton, dosboxVersionPanel != nullptr);
	WXUtilities::setButtonEnabled(m_removeDOSBoxVersionButton, isDOSBoxVersionRemovable);

	WXUtilities::setButtonEnabled(m_discardDOSBoxSettingsChangesButton, m_modified);
	WXUtilities::setButtonEnabled(m_saveDOSBoxSettingsButton, m_modified);
}

bool DOSBoxManagerPanel::addDOSBoxVersionPanel(DOSBoxVersionPanel * dosboxVersionPanel) {
	if(dosboxVersionPanel == nullptr || hasDOSBoxVersionPanel(dosboxVersionPanel)) {
		return false;
	}

	m_dosboxVersionPanelSignalConnectionGroups.push_back(SignalConnectionGroup(
		dosboxVersionPanel->dosboxVersionSettingChanged.connect(std::bind(&DOSBoxManagerPanel::onDOSBoxVersionSettingChanged, this, std::placeholders::_1, std::placeholders::_2)),
		dosboxVersionPanel->dosboxVersionChangesDiscarded.connect(std::bind(&DOSBoxManagerPanel::onDOSBoxVersionChangesDiscarded, this, std::placeholders::_1)),
		dosboxVersionPanel->dosboxVersionReset.connect(std::bind(&DOSBoxManagerPanel::onDOSBoxVersionReset, this, std::placeholders::_1)),
		dosboxVersionPanel->dosboxVersionSaved.connect(std::bind(&DOSBoxManagerPanel::onDOSBoxVersionSaved, this, std::placeholders::_1))
	));

	m_notebook->AddPage(dosboxVersionPanel, wxString::FromUTF8(dosboxVersionPanel->getPanelName()));
	m_notebook->ChangeSelection(m_notebook->GetPageCount() - 1);

	updateButtons();

	return true;
}

bool DOSBoxManagerPanel::newDOSBoxVersion() {
	if(!addDOSBoxVersionPanel(new DOSBoxVersionPanel(nullptr, m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL))) {
		return false;
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		SegmentAnalytics::getInstance()->track("New DOSBox Version Created");
	}

	return true;
}

bool DOSBoxManagerPanel::installDOSBoxVersion(size_t index) {
	DOSBoxVersionPanel * dosboxVersionPanel = getDOSBoxVersionPanel(index);

	if(dosboxVersionPanel == nullptr) {
		return false;
	}

	std::shared_ptr<DOSBoxManager> dosboxManager(m_modManager->getDOSBoxManager());
	std::shared_ptr<DOSBoxVersion> dosboxVersion(dosboxVersionPanel->getDOSBoxVersion());

	if(dosboxVersion->hasDirectoryPath() || !dosboxVersion->hasID() || !dosboxVersion->hasLongName() || !dosboxVersion->hasShortName() || !DOSBoxManager::isDOSBoxVersionDownloadable(*dosboxVersion) || !dosboxManager->isInitialized()) {
		return false;
	}

	wxDirDialog selectInstallDirectoryDialog(this, "Install DOSBox to Directory", std::filesystem::current_path().string(), wxDD_DIR_MUST_EXIST, wxDefaultPosition, wxDefaultSize, "Install DOSBox");
	int selectInstallDirectoryResult = selectInstallDirectoryDialog.ShowModal();

	if(selectInstallDirectoryResult == wxID_CANCEL) {
		return false;
	}

	std::string destinationDirectoryPath(selectInstallDirectoryDialog.GetPath());

	if(Utilities::areStringsEqual(Utilities::getAbsoluteFilePath(destinationDirectoryPath), Utilities::getAbsoluteFilePath(std::filesystem::current_path().string()))) {
		wxMessageBox("Cannot install dosbox directly into mod manager directory!", "Invalid Installation Directory", wxOK | wxICON_ERROR, this);
		return false;
	}

	if(!std::filesystem::is_directory(std::filesystem::path(destinationDirectoryPath))) {
		wxMessageBox("Invalid dosbox installation directory selected!", "Invalid Installation Directory", wxOK | wxICON_ERROR, this);
		return false;
	}

	for(const std::filesystem::directory_entry & entry : std::filesystem::directory_iterator(std::filesystem::path(destinationDirectoryPath))) {
		int installToNonEmptyDirectoryResult = wxMessageBox(fmt::format("DOSBox installation installation directory '{}' is not empty!\n\nInstalling to a directory which already contains files may cause issues. Are you sure you would like to install '{}' to this directory?", destinationDirectoryPath, dosboxVersion->getLongName()), "Non-Empty Installation Directory", wxYES_NO | wxCANCEL | wxICON_WARNING, this);

		if(installToNonEmptyDirectoryResult != wxYES) {
			return false;
		}

		break;
	}

	m_installProgressDialog = new wxProgressDialog("Installing", fmt::format("Installing '{}', please wait...", dosboxVersion->getLongName()), 100, this, wxPD_REMAINING_TIME);
	m_installProgressDialog->SetIcon(wxICON(D3DMODMGR_ICON));

	boost::signals2::connection installStatusChangedConnection(dosboxManager->installStatusChanged.connect([this](const std::string & statusMessage) {
		QueueEvent(new DOSBoxInstallProgressEvent(m_installProgressDialog->GetValue(), statusMessage));
	}));

	boost::signals2::connection dosboxDownloadProgressConnection(dosboxManager->dosboxDownloadProgress.connect([this](DOSBoxVersion & dosboxVersion, HTTPRequest & request, size_t numberOfBytesDownloaded, size_t totalNumberOfBytes) {
		QueueEvent(new DOSBoxInstallProgressEvent(
			static_cast<uint8_t>(static_cast<double>(numberOfBytesDownloaded) / static_cast<double>(totalNumberOfBytes) * 100.0),
			fmt::format("Downloaded {} / {} of '{}' application files from: '{}'.", Utilities::fileSizeToString(numberOfBytesDownloaded), Utilities::fileSizeToString(totalNumberOfBytes), dosboxVersion.getLongName(), request.getUrl())
		));
	}));

	m_installDOSBoxFuture = std::async(std::launch::async, [this, dosboxManager, dosboxVersion, dosboxVersionPanel, destinationDirectoryPath, installStatusChangedConnection, dosboxDownloadProgressConnection]() {
		bool dosboxInstalled = dosboxManager->installLatestDOSBoxVersion(dosboxVersion->getID(), destinationDirectoryPath);

		installStatusChangedConnection.disconnect();
		dosboxDownloadProgressConnection.disconnect();

		if(!dosboxInstalled) {
			QueueEvent(new DOSBoxInstallDoneEvent());

			wxMessageBox(fmt::format("Failed to install '{}' to '{}'!\n\nCheck console for details.", dosboxVersion->getLongName(), destinationDirectoryPath), "Installation Failed", wxOK | wxICON_ERROR, this);

			return false;
		}

		dosboxVersionPanel->discardDOSBoxPathChanges();
		dosboxVersion->setDirectoryPath(destinationDirectoryPath);

		saveDOSBoxVersions();

		updateButtons();

		QueueEvent(new DOSBoxInstallDoneEvent());

		wxMessageBox(fmt::format("'{}' was successfully installed to: '{}'!", dosboxVersion->getLongName(), destinationDirectoryPath), "DOSBox Installed", wxOK | wxICON_INFORMATION, this);

		return true;
	});

	return true;
}

bool DOSBoxManagerPanel::installCurrentDOSBoxVersion() {
	return installDOSBoxVersion(m_notebook->GetSelection());
}

bool DOSBoxManagerPanel::uninstallDOSBoxVersion(size_t index) {
	DOSBoxVersionPanel * dosboxVersionPanel = getDOSBoxVersionPanel(index);

	if(dosboxVersionPanel == nullptr) {
		return false;
	}

	std::shared_ptr<DOSBoxVersion> dosboxVersion(dosboxVersionPanel->getDOSBoxVersion());

	if(!dosboxVersion->hasID() || !dosboxVersion->hasDirectoryPath()) {
		return false;
	}

	if(!std::filesystem::is_directory(std::filesystem::path(dosboxVersion->getDirectoryPath()))) {
		wxMessageBox(fmt::format("'{}' path '{}' is not a valid directory.", dosboxVersion->getLongName(), dosboxVersion->getDirectoryPath()), "Uninstall Failed", wxOK | wxICON_WARNING, this);

		return false;
	}

	int uninstallConfirmationResult = wxMessageBox(fmt::format("Are you sure you want to uninstall '{}' from '{}'? This will remove all files located in this directory, so be sure to back up any valuable data!", dosboxVersion->getLongName(), dosboxVersion->getDirectoryPath()), "Uninstall Confirmation", wxYES_NO | wxCANCEL | wxICON_QUESTION, this);

	if(uninstallConfirmationResult != wxYES) {
		return false;
	}

	std::error_code errorCode;
	std::filesystem::remove_all(std::filesystem::path(dosboxVersion->getDirectoryPath()), errorCode);

	if(errorCode) {
		wxMessageBox(fmt::format("Failed to remove '{}' dosbox directory '{}': {}", dosboxVersion->getLongName(), dosboxVersion->getDirectoryPath(), errorCode.message()), "Directory Removal Failed", wxOK | wxICON_ERROR, this);
		return false;
	}

	dosboxVersionPanel->discardDOSBoxPathChanges();
	dosboxVersion->setDirectoryPath("");

	saveDOSBoxVersions();

	updateButtons();

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		dosboxVersion->addMetadata(properties);

		SegmentAnalytics::getInstance()->track("Uninstalled DOSBox", properties);
	}

	wxMessageBox(fmt::format("'{}' was successfully uninstalled!", dosboxVersion->getLongName()), "DOSBox Uninstalled", wxOK | wxICON_INFORMATION, this);

	return true;
}

bool DOSBoxManagerPanel::uninstallCurrentDOSBoxVersion() {
	return uninstallDOSBoxVersion(m_notebook->GetSelection());
}

bool DOSBoxManagerPanel::saveDOSBoxVersion(size_t index) {
	DOSBoxVersionPanel * dosboxVersionPanel = getDOSBoxVersionPanel(index);

	if(dosboxVersionPanel == nullptr) {
		return false;
	}

	std::shared_ptr<DOSBoxVersion> dosboxVersion(dosboxVersionPanel->getDOSBoxVersion());

	if(!dosboxVersionPanel->save()) {
		return false;
	}

	if(!dosboxVersion->isValid()) {
		wxMessageBox(fmt::format("'{}' dosbox version is not valid!", dosboxVersion->getLongName()), "Invalid DOSBox", wxOK | wxICON_WARNING, this);

		return false;
	}

	std::shared_ptr<DOSBoxVersionCollection> dosboxVersions(m_modManager->getDOSBoxVersions());

	if(dosboxVersions->addDOSBoxVersion(dosboxVersion)) {
		spdlog::info("Added new dosbox version to collection with name '{}'.", dosboxVersion->getLongName());
	}

	if(!saveDOSBoxVersions()) {
		return false;
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		dosboxVersion->addMetadata(properties);

		SegmentAnalytics::getInstance()->track("Saved DOSBox Configuration", properties);
	}

	return true;
}

bool DOSBoxManagerPanel::saveCurrentDOSBoxVersion() {
	return saveDOSBoxVersion(m_notebook->GetSelection());
}

bool DOSBoxManagerPanel::discardDOSBoxVersionChanges(size_t index) {
	DOSBoxVersionPanel * dosboxVersionPanel = getDOSBoxVersionPanel(index);

	if(dosboxVersionPanel == nullptr) {
		return false;
	}

	dosboxVersionPanel->discard();

	return true;
}

bool DOSBoxManagerPanel::discardCurrentDOSBoxVersionChanges() {
	return discardDOSBoxVersionChanges(m_notebook->GetSelection());
}

bool DOSBoxManagerPanel::resetDOSBoxVersion(size_t dosboxVersionPanelIndex) {
	DOSBoxVersionPanel * dosboxVersionPanel = getDOSBoxVersionPanel(dosboxVersionPanelIndex);

	if(dosboxVersionPanel == nullptr) {
		return false;
	}

	std::shared_ptr<DOSBoxVersion> dosboxVersion(dosboxVersionPanel->getDOSBoxVersion());

	int resetResult = wxMessageBox(fmt::format("Are you sure you want to reset the '{}' dosbox version to its default configuration?", dosboxVersion->hasLongName() ? dosboxVersion->getLongName() : "NEW"), "Reset DOSBox", wxICON_QUESTION | wxYES_NO | wxCANCEL, this);

	if(resetResult != wxYES) {
		return false;
	}

	dosboxVersionPanel->reset();

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["dosboxID"] = dosboxVersion->getID();

		SegmentAnalytics::getInstance()->track("Reset DOSBox Configuration", properties);
	}

	return true;
}

bool DOSBoxManagerPanel::resetCurrentDOSBoxVersion() {
	return resetDOSBoxVersion(m_notebook->GetSelection());
}

bool DOSBoxManagerPanel::removeDOSBoxVersion(size_t dosboxVersionPanelIndex) {
	std::shared_ptr<DOSBoxVersion> dosboxVersion = getDOSBoxVersion(dosboxVersionPanelIndex);

	if(dosboxVersion == nullptr || !dosboxVersion->isRemovable()) {
		return false;
	}

	int removeResult = wxMessageBox(fmt::format("Are you sure you want to remove the '{}' dosbox version?", dosboxVersion->hasLongName() ? dosboxVersion->getLongName() : "NEW"), "Remove DOSBox", wxICON_QUESTION | wxYES_NO | wxCANCEL, this);

	if(removeResult != wxYES) {
		return false;
	}

	m_modManager->getDOSBoxVersions()->removeDOSBoxVersion(*dosboxVersion);

	saveDOSBoxVersions();

	if(m_notebook->GetPageCount() == 1) {
		m_newDOSBoxVersionButton->SetFocus();
	}

	m_dosboxVersionPanelSignalConnectionGroups[dosboxVersionPanelIndex].disconnect();
	m_dosboxVersionPanelSignalConnectionGroups.erase(m_dosboxVersionPanelSignalConnectionGroups.begin() + dosboxVersionPanelIndex);

	DOSBoxVersionPanel * dosboxVersionPanel = getDOSBoxVersionPanel(dosboxVersionPanelIndex);
	m_notebook->RemovePage(dosboxVersionPanelIndex);
	delete dosboxVersionPanel;

	updateButtons();

	return true;
}

bool DOSBoxManagerPanel::removeCurrentDOSBoxVersion() {
	return removeDOSBoxVersion(m_notebook->GetSelection());
}

void DOSBoxManagerPanel::onDOSBoxSettingModified(SettingPanel & settingPanel) {
	if(settingPanel.isModified()) {
		m_modified = true;

		updateButtons();
		dosboxSettingsChanged();
	}
}

void DOSBoxManagerPanel::onNotebookPageChanged(wxBookCtrlEvent & event) {
	updateButtons();

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::shared_ptr<DOSBoxVersion> currentDOSBoxVersion(getCurrentDOSBoxVersion());

		if(currentDOSBoxVersion != nullptr && currentDOSBoxVersion->hasID() && currentDOSBoxVersion->hasLongName()) {
			std::map<std::string, std::any> properties;
			currentDOSBoxVersion->addMetadata(properties);

			SegmentAnalytics::getInstance()->screen(currentDOSBoxVersion->getLongName(), "DOSBox Manager", properties);
		}
	}
}

void DOSBoxManagerPanel::onNewDOSBoxVersionButtonPressed(wxCommandEvent & event) {
	newDOSBoxVersion();
}

void DOSBoxManagerPanel::onInstallDOSBoxVersionButtonPressed(wxCommandEvent & event) {
	installCurrentDOSBoxVersion();
}

void DOSBoxManagerPanel::onUninstallDOSBoxVersionButtonPressed(wxCommandEvent & event) {
	uninstallCurrentDOSBoxVersion();
}

void DOSBoxManagerPanel::onSaveDOSBoxVersionButtonPressed(wxCommandEvent & event) {
	saveCurrentDOSBoxVersion();
}

void DOSBoxManagerPanel::onDiscardDOSBoxVersionChangesButtonPressed(wxCommandEvent & event) {
	discardCurrentDOSBoxVersionChanges();
}

void DOSBoxManagerPanel::onResetDOSBoxVersionButtonPressed(wxCommandEvent & event) {
	resetCurrentDOSBoxVersion();
}

void DOSBoxManagerPanel::onRemoveDOSBoxVersionButtonPressed(wxCommandEvent & event) {
	removeCurrentDOSBoxVersion();
}

void DOSBoxManagerPanel::onInstallProgress(DOSBoxInstallProgressEvent & event) {
	if(m_installProgressDialog == nullptr) {
		return;
	}

	m_installProgressDialog->Update(event.getValue(), event.getMessage());
	m_installProgressDialog->Fit();
}

void DOSBoxManagerPanel::onInstallDone(DOSBoxInstallDoneEvent & event) {
	if(m_installProgressDialog == nullptr) {
		return;
	}

	m_installProgressDialog->Destroy();
	m_installProgressDialog = nullptr;
}

void DOSBoxManagerPanel::onDOSBoxVersionChangesDiscarded(DOSBoxVersionPanel & dosboxVersionPanel) {
	updateDOSBoxVersionPanel(indexOfDOSBoxVersionPanel(&dosboxVersionPanel));
}

void DOSBoxManagerPanel::onDOSBoxVersionSettingChanged(DOSBoxVersionPanel & dosboxVersionPanel, SettingPanel & settingPanel) {
	updateDOSBoxVersionPanel(indexOfDOSBoxVersionPanel(&dosboxVersionPanel));
}

void DOSBoxManagerPanel::onDOSBoxVersionReset(DOSBoxVersionPanel & dosboxVersionPanel) {
	updateDOSBoxVersionPanel(indexOfDOSBoxVersionPanel(&dosboxVersionPanel));
}

void DOSBoxManagerPanel::onDOSBoxVersionSaved(DOSBoxVersionPanel & dosboxVersionPanel) {
	updateDOSBoxVersionPanel(indexOfDOSBoxVersionPanel(&dosboxVersionPanel));
}

void DOSBoxManagerPanel::onSaveDOSBoxSettingsButtonPressed(wxCommandEvent & event) {
	saveDOSBoxSettings();
}

void DOSBoxManagerPanel::onDiscardDOSBoxSettingsChangesButtonPressed(wxCommandEvent & event) {
	discardDOSBoxSettings();
}

void DOSBoxManagerPanel::onResetDefaultDOSBoxSettingsButtonPressed(wxCommandEvent & event) {
	int resetDefaultsResult = wxMessageBox("Are you sure you want to reset general DOSBox settings to default values?", "Reset General DOSBox Settings", wxYES_NO | wxCANCEL | wxICON_WARNING, this);

	if(resetDefaultsResult == wxYES) {
		resetDefaultDOSBoxSettings();
	}
}
