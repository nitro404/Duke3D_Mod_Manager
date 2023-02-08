#include "DOSBoxManagerPanel.h"

#include "DOSBox/DOSBoxManager.h"
#include "Manager/SettingsManager.h"
#include "SettingPanel.h"
#include "WXUtilities.h"

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

#include <filesystem>
#include <sstream>

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
	m_dosboxSettingsPanel->addListener(*this);

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
		dosboxVersionPanel->addListener(*this);

		addDOSBoxVersionPanel(dosboxVersionPanel);
	}

	update();
}

DOSBoxManagerPanel::~DOSBoxManagerPanel() {
	m_dosboxSettingsPanel->removeListener(*this);
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

bool DOSBoxManagerPanel::hasPanelWithDOSBoxVersionName(const std::string & name) const {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(getDOSBoxVersion(i)->getName(), name)) {
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

size_t DOSBoxManagerPanel::indexOfPanelWithDOSBoxVersionName(const std::string & name) const {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(getDOSBoxVersion(i)->getName(), name)) {
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

DOSBoxVersionPanel * DOSBoxManagerPanel::getPanelWithDOSBoxVersionName(const std::string & name) const {
	return getDOSBoxVersionPanel(indexOfPanelWithDOSBoxVersionName(name));
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

	return true;
}

bool DOSBoxManagerPanel::selectPanelWithDOSBoxVersion(const DOSBoxVersion & dosboxVersion) {
	size_t dosboxVersionPanelIndex = indexOfPanelWithDOSBoxVersion(&dosboxVersion);

	if(dosboxVersionPanelIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	m_notebook->ChangeSelection(dosboxVersionPanelIndex);

	return true;
}

bool DOSBoxManagerPanel::selectPanelWithDOSBoxVersionName(const std::string & name) {
	size_t dosboxVersionPanelIndex = indexOfPanelWithDOSBoxVersionName(name);

	if(dosboxVersionPanelIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	m_notebook->ChangeSelection(dosboxVersionPanelIndex);

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
	notifyDOSBoxSettingsReset();
}

void DOSBoxManagerPanel::discardDOSBoxSettings() {
	m_dosboxSettingsPanel->discardSettings();

	m_modified = false;

	updateButtons();
	notifyDOSBoxSettingsDiscarded();
}

bool DOSBoxManagerPanel::saveDOSBoxSettings() {
	if(!m_dosboxSettingsPanel->saveSettings()) {
		return false;
	}

	m_modified = false;

	updateButtons();
	notifyDOSBoxSettingsSaved();

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

	m_notebook->SetPageText(dosboxVersionPanelIndex, dosboxVersionPanel->getPanelName());

	return true;
}

void DOSBoxManagerPanel::updateButtons() {
	DOSBoxVersionPanel * dosboxVersionPanel = getCurrentDOSBoxVersionPanel();
	std::shared_ptr<DOSBoxVersion> dosboxVersion = dosboxVersionPanel != nullptr ? dosboxVersionPanel->getDOSBoxVersion() : nullptr;

	bool isDOSBoxVersionModified = dosboxVersionPanel != nullptr ? dosboxVersionPanel->isModified() : false;
	bool isDOSBoxVersionInstallable = dosboxVersion != nullptr ? DOSBoxManager::isDOSBoxVersionDownloadable(dosboxVersion->getName()) : false;
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

	dosboxVersionPanel->addListener(*this);
	m_notebook->AddPage(dosboxVersionPanel, dosboxVersionPanel->getPanelName());
	m_notebook->ChangeSelection(m_notebook->GetPageCount() - 1);

	updateButtons();

	return true;
}

bool DOSBoxManagerPanel::newDOSBoxVersion() {
	return addDOSBoxVersionPanel(new DOSBoxVersionPanel(nullptr, m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL));
}

bool DOSBoxManagerPanel::installDOSBoxVersion(size_t index) {
	DOSBoxVersionPanel * dosboxVersionPanel = getDOSBoxVersionPanel(index);

	if(dosboxVersionPanel == nullptr) {
		return false;
	}

	std::shared_ptr<DOSBoxManager> dosboxManager(m_modManager->getDOSBoxManager());
	std::shared_ptr<DOSBoxVersion> dosboxVersion(dosboxVersionPanel->getDOSBoxVersion());

	if(dosboxVersion->hasDirectoryPath() || !dosboxVersion->hasName() || !DOSBoxManager::isDOSBoxVersionDownloadable(dosboxVersion->getName()) || !dosboxManager->isInitialized()) {
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
		int installToNonEmptyDirectoryResult = wxMessageBox(fmt::format("DOSBox installation installation directory '{}' is not empty!\n\nInstalling to a directory which already contains files may cause issues. Are you sure you would like to install '{}' to this directory?", destinationDirectoryPath, dosboxVersion->getName()), "Non-Empty Installation Directory", wxYES_NO | wxCANCEL | wxICON_WARNING, this);

		if(installToNonEmptyDirectoryResult != wxYES) {
			return false;
		}

		break;
	}

	wxProgressDialog * installingProgressDialog = new wxProgressDialog("Installing", fmt::format("Installing '{}', please wait...", dosboxVersion->getName()), 1, this, wxPD_AUTO_HIDE);
	installingProgressDialog->SetIcon(wxICON(D3DMODMGR_ICON));

	if(!dosboxManager->installLatestDOSBoxVersion(*dosboxVersion, destinationDirectoryPath)) {
		installingProgressDialog->Destroy();

		wxMessageBox(fmt::format("Failed to install '{}' to '{}'!\n\nCheck console for details.", dosboxVersion->getName(), destinationDirectoryPath), "Installation Failed", wxOK | wxICON_ERROR, this);

		return false;
	}

	dosboxVersionPanel->discardDOSBoxPathChanges();
	dosboxVersion->setDirectoryPath(destinationDirectoryPath);

	saveDOSBoxVersions();

	updateButtons();

	installingProgressDialog->Update(1);
	installingProgressDialog->Destroy();

	wxMessageBox(fmt::format("'{}' was successfully installed to: '{}'!", dosboxVersion->getName(), destinationDirectoryPath), "DOSBox Installed", wxOK | wxICON_INFORMATION, this);

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

	std::shared_ptr<DOSBoxVersion> dosboxVersion = dosboxVersionPanel->getDOSBoxVersion();

	if(!dosboxVersion->hasName() || !dosboxVersion->hasDirectoryPath()) {
		return false;
	}

	if(!std::filesystem::is_directory(std::filesystem::path(dosboxVersion->getDirectoryPath()))) {
		wxMessageBox(fmt::format("'{}' path '{}' is not a valid directory.", dosboxVersion->getName(), dosboxVersion->getDirectoryPath()), "Uninstall Failed", wxOK | wxICON_WARNING, this);

		return false;
	}

	int uninstallConfirmationResult = wxMessageBox(fmt::format("Are you sure you want to uninstall '{}' from '{}'? This will remove all files located in this directory, so be sure to back up any valuable data!", dosboxVersion->getName(), dosboxVersion->getDirectoryPath()), "Uninstall Confirmation", wxYES_NO | wxCANCEL | wxICON_QUESTION, this);

	if(uninstallConfirmationResult != wxYES) {
		return false;
	}

	std::error_code errorCode;
	std::filesystem::remove_all(std::filesystem::path(dosboxVersion->getDirectoryPath()), errorCode);

	if(errorCode) {
		wxMessageBox(fmt::format("Failed to remove '{}' dosbox directory '{}': {}", dosboxVersion->getName(), dosboxVersion->getDirectoryPath(), errorCode.message()), "Directory Removal Failed", wxOK | wxICON_ERROR, this);
		return false;
	}

	dosboxVersionPanel->discardDOSBoxPathChanges();
	dosboxVersion->setDirectoryPath("");

	saveDOSBoxVersions();

	updateButtons();

	wxMessageBox(fmt::format("'{}' was successfully uninstalled!", dosboxVersion->getName()), "DOSBox Uninstalled", wxOK | wxICON_INFORMATION, this);

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

	std::shared_ptr<DOSBoxVersion> dosboxVersion = dosboxVersionPanel->getDOSBoxVersion();

	if(!dosboxVersionPanel->save()) {
		return false;
	}

	if(!dosboxVersion->isValid()) {
		wxMessageBox(fmt::format("'{}' dosbox version is not valid!", dosboxVersion->getName()), "Invalid DOSBox", wxOK | wxICON_WARNING, this);

		return false;
	}

	std::shared_ptr<DOSBoxVersionCollection> dosboxVersions(m_modManager->getDOSBoxVersions());

	if(dosboxVersions->addDOSBoxVersion(dosboxVersion)) {
		spdlog::info("Added new dosbox version to collection with name '{}'.", dosboxVersion->getName());
	}

	return saveDOSBoxVersions();
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

	std::shared_ptr<DOSBoxVersion> dosboxVersion = dosboxVersionPanel->getDOSBoxVersion();

	int resetResult = wxMessageBox(fmt::format("Are you sure you want to reset the '{}' dosbox version to its default configuration?", dosboxVersion->hasName() ? dosboxVersion->getName() : "NEW"), "Reset DOSBox", wxICON_QUESTION | wxYES_NO | wxCANCEL, this);

	if(resetResult != wxYES) {
		return false;
	}

	dosboxVersionPanel->reset();

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

	int removeResult = wxMessageBox(fmt::format("Are you sure you want to remove the '{}' dosbox version?", dosboxVersion->hasName() ? dosboxVersion->getName() : "NEW"), "Remove DOSBox", wxICON_QUESTION | wxYES_NO | wxCANCEL, this);

	if(removeResult != wxYES) {
		return false;
	}

	m_modManager->getDOSBoxVersions()->removeDOSBoxVersion(*dosboxVersion);

	saveDOSBoxVersions();

	if(m_notebook->GetPageCount() == 1) {
		m_newDOSBoxVersionButton->SetFocus();
	}

	DOSBoxVersionPanel * dosboxVersionPanel = getDOSBoxVersionPanel(dosboxVersionPanelIndex);
	dosboxVersionPanel->removeListener(*this);
	m_notebook->RemovePage(dosboxVersionPanelIndex);
	delete dosboxVersionPanel;

	updateButtons();

	return true;
}

bool DOSBoxManagerPanel::removeCurrentDOSBoxVersion() {
	return removeDOSBoxVersion(m_notebook->GetSelection());
}

DOSBoxManagerPanel::Listener::~Listener() { }

size_t DOSBoxManagerPanel::numberOfListeners() const {
	return m_listeners.size();
}

bool DOSBoxManagerPanel::hasListener(const Listener & listener) const {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			return true;
		}
	}

	return false;
}

size_t DOSBoxManagerPanel::indexOfListener(const Listener & listener) const {
	for(size_t i = 0; i < m_listeners.size(); i++) {
		if(m_listeners[i] == &listener) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

DOSBoxManagerPanel::Listener * DOSBoxManagerPanel::getListener(size_t index) const {
	if(index >= m_listeners.size()) {
		return nullptr;
	}

	return m_listeners[index];
}

bool DOSBoxManagerPanel::addListener(Listener & listener) {
	if(!hasListener(listener)) {
		m_listeners.push_back(&listener);

		return true;
	}

	return false;
}

bool DOSBoxManagerPanel::removeListener(size_t index) {
	if(index >= m_listeners.size()) {
		return false;
	}

	m_listeners.erase(m_listeners.cbegin() + index);

	return true;
}

bool DOSBoxManagerPanel::removeListener(const Listener & listener) {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			m_listeners.erase(i);

			return true;
		}
	}

	return false;
}

void DOSBoxManagerPanel::clearListeners() {
	m_listeners.clear();
}

void DOSBoxManagerPanel::notifyDOSBoxSettingsChanged() {
	for(Listener * listener : m_listeners) {
		listener->dosboxSettingsChanged();
	}
}

void DOSBoxManagerPanel::notifyDOSBoxSettingsReset() {
	for(Listener * listener : m_listeners) {
		listener->dosboxSettingsReset();
	}
}

void DOSBoxManagerPanel::notifyDOSBoxSettingsDiscarded() {
	for(Listener * listener : m_listeners) {
		listener->dosboxSettingsDiscarded();
	}
}

void DOSBoxManagerPanel::notifyDOSBoxSettingsSaved() {
	for(Listener * listener : m_listeners) {
		listener->dosboxSettingsSaved();
	}
}

void DOSBoxManagerPanel::dosboxSettingModified(SettingPanel & settingPanel) {
	if(settingPanel.isModified()) {
		m_modified = true;

		updateButtons();
		notifyDOSBoxSettingsChanged();
	}
}

void DOSBoxManagerPanel::onNotebookPageChanged(wxBookCtrlEvent & event) {
	updateButtons();
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

void DOSBoxManagerPanel::dosboxVersionChangesDiscarded(DOSBoxVersionPanel & dosboxVersionPanel) {
	updateDOSBoxVersionPanel(indexOfDOSBoxVersionPanel(&dosboxVersionPanel));
}

void DOSBoxManagerPanel::dosboxVersionSettingChanged(DOSBoxVersionPanel & dosboxVersionPanel, SettingPanel & settingPanel) {
	updateDOSBoxVersionPanel(indexOfDOSBoxVersionPanel(&dosboxVersionPanel));
}

void DOSBoxManagerPanel::dosboxVersionReset(DOSBoxVersionPanel & dosboxVersionPanel) {
	updateDOSBoxVersionPanel(indexOfDOSBoxVersionPanel(&dosboxVersionPanel));
}

void DOSBoxManagerPanel::dosboxVersionSaved(DOSBoxVersionPanel & dosboxVersionPanel) {
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