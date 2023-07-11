#include "GroupEditorPanel.h"

#include "../WXUtilities.h"
#include "Game/File/Group/GRP/GroupGRP.h"
#include "Game/File/Group/SSI/GroupSSI.h"
#include "Manager/SettingsManager.h"

#include <Analytics/Segment/SegmentAnalytics.h>
#include <Signal/SignalConnectionGroup.h>
#include <Utilities/FileUtilities.h>

#include <spdlog/spdlog.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/gbsizer.h>
#include <wx/textdlg.h>
#include <wx/wrapsizer.h>

#include <any>
#include <map>
#include <filesystem>
#include <sstream>

static const std::string DEFAULT_NEW_GRP_FILE_NAME("NEW.GRP");
static const std::string DEFAULT_NEW_SSI_FILE_NAME("NEW.SSI");

static const std::string FILE_DIALOG_GRP_FILE_TYPE("Build Engine Group Files (*.grp)|*.grp");
static const std::string FILE_DIALOG_SSI_FILE_TYPE("Sunstorm Interactive Files (*.ssi)|*.ssi");
static const std::string FILE_DIALOG_SEPARATE_FILE_TYPES(FILE_DIALOG_GRP_FILE_TYPE + "|" + FILE_DIALOG_SSI_FILE_TYPE);
static const std::string FILE_DIALOG_GROUPED_FILE_TYPES("Group / SSI Files (*.grp;*.ssi)|*.grp;*.ssi");
static const std::string FILE_DIALOG_ALL_FILES("All Files (*.*)|*.*");

GroupEditorPanel::GroupEditorPanel(wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Group Editor")
	, m_notebook(nullptr)
	, m_newButton(nullptr)
	, m_openButton(nullptr)
	, m_saveButton(nullptr)
	, m_saveAsButton(nullptr)
	, m_addFilesButton(nullptr)
	, m_removeFilesButton(nullptr)
	, m_replaceFileButton(nullptr)
	, m_renameFileButton(nullptr)
	, m_extractFilesButton(nullptr)
	, m_extractAllFilesButton(nullptr)
	, m_closeButton(nullptr)
	, m_closeAllButton(nullptr) {

	m_notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP | wxNB_MULTILINE, "Groups");
	m_notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &GroupEditorPanel::onNotebookPageChanged, this);

	wxScrolledWindow * actionsPanel = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "Group Actions");
	actionsPanel->SetScrollRate(5, 5);

	m_newButton = new wxButton(actionsPanel, wxID_ANY, "New", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Create New Group");
	m_newButton->Bind(wxEVT_BUTTON, &GroupEditorPanel::onNewButtonPressed, this);

	m_openButton = new wxButton(actionsPanel, wxID_ANY, "Open", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Open Group");
	m_openButton->Bind(wxEVT_BUTTON, &GroupEditorPanel::onOpenButtonPressed, this);

	m_createFromButton = new wxButton(actionsPanel, wxID_ANY, "Create From", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Create New Group from Directory");
	m_createFromButton->Bind(wxEVT_BUTTON, &GroupEditorPanel::onCreateFromButtonPressed, this);

	m_saveButton = new wxButton(actionsPanel, wxID_ANY, "Save", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Save Group");
	m_saveButton->Bind(wxEVT_BUTTON, &GroupEditorPanel::onSaveButtonPressed, this);
	m_saveButton->Disable();

	m_saveAsButton = new wxButton(actionsPanel, wxID_ANY, "Save As", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Save Group As");
	m_saveAsButton->Bind(wxEVT_BUTTON, &GroupEditorPanel::onSaveAsButtonPressed, this);
	m_saveAsButton->Disable();

	m_addFilesButton = new wxButton(actionsPanel, wxID_ANY, "Add Files", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Add Files to Group");
	m_addFilesButton->Bind(wxEVT_BUTTON, &GroupEditorPanel::onAddFilesButtonPressed, this);
	m_addFilesButton->Disable();

	m_removeFilesButton = new wxButton(actionsPanel, wxID_ANY, "Remove Files", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Remove Files from Group");
	m_removeFilesButton->Bind(wxEVT_BUTTON, &GroupEditorPanel::onRemoveFilesButtonPressed, this);
	m_removeFilesButton->Disable();

	m_replaceFileButton = new wxButton(actionsPanel, wxID_ANY, "Replace File", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Replace File in Group");
	m_replaceFileButton->Bind(wxEVT_BUTTON, &GroupEditorPanel::onReplaceFileButtonPressed, this);
	m_replaceFileButton->Disable();

	m_renameFileButton = new wxButton(actionsPanel, wxID_ANY, "Rename File", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Rename File in Group");
	m_renameFileButton->Bind(wxEVT_BUTTON, &GroupEditorPanel::onRenameFileButtonPressed, this);
	m_renameFileButton->Disable();

	m_extractFilesButton = new wxButton(actionsPanel, wxID_ANY, "Extract Files", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Extract Files from Group");
	m_extractFilesButton->Bind(wxEVT_BUTTON, &GroupEditorPanel::onExtractFilesButtonPressed, this);
	m_extractFilesButton->Disable();

	m_extractAllFilesButton = new wxButton(actionsPanel, wxID_ANY, "Extract All Files", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Extract All Files from Group");
	m_extractAllFilesButton->Bind(wxEVT_BUTTON, &GroupEditorPanel::onExtractAllFilesButtonPressed, this);
	m_extractAllFilesButton->Disable();

	m_closeButton = new wxButton(actionsPanel, wxID_ANY, "Close", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Close Group");
	m_closeButton->Bind(wxEVT_BUTTON, &GroupEditorPanel::onCloseButtonPressed, this);
	m_closeButton->Disable();

	m_closeAllButton = new wxButton(actionsPanel, wxID_ANY, "Close All", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Close All Groups");
	m_closeAllButton->Bind(wxEVT_BUTTON, &GroupEditorPanel::onCloseAllButtonPressed, this);
	m_closeAllButton->Disable();

	int border = 2;

	wxWrapSizer * actionsPanelSizer = new wxWrapSizer(wxVERTICAL);
	actionsPanelSizer->Add(m_newButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanelSizer->Add(m_openButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanelSizer->Add(m_createFromButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanelSizer->Add(m_saveButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanelSizer->Add(m_saveAsButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanelSizer->Add(m_addFilesButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanelSizer->Add(m_removeFilesButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanelSizer->Add(m_replaceFileButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanelSizer->Add(m_renameFileButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanelSizer->Add(m_extractFilesButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanelSizer->Add(m_extractAllFilesButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanelSizer->Add(m_closeButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanelSizer->Add(m_closeAllButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanel->SetSizerAndFit(actionsPanelSizer);

	wxGridBagSizer * groupEditorPanelSizer = new wxGridBagSizer(border, border);
	groupEditorPanelSizer->Add(actionsPanel, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	groupEditorPanelSizer->Add(m_notebook, wxGBPosition(0, 1), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	groupEditorPanelSizer->AddGrowableRow(0, 0);
	groupEditorPanelSizer->AddGrowableCol(1, 0);
	SetSizerAndFit(groupEditorPanelSizer);

	update();
}

GroupEditorPanel::~GroupEditorPanel() { }

bool GroupEditorPanel::hasGroupPanel(const GroupPanel * groupPanel) const {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(m_notebook->GetPage(i) == groupPanel) {
			return true;
		}
	}

	return false;
}

bool GroupEditorPanel::hasPanelWithGroup(const Group * group) const {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(getGroup(i) == group) {
			return true;
		}
	}

	return false;
}

size_t GroupEditorPanel::indexOfGroupPanel(const GroupPanel * groupPanel) const {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(m_notebook->GetPage(i) == groupPanel) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t GroupEditorPanel::indexOfPanelWithGroup(const Group * group) const {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(getGroup(i) == group) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t GroupEditorPanel::indexOfPanelWithGroupFilePath(const std::string & filePath) const {
	std::error_code errorCode;
	std::filesystem::path actualFilePath(filePath);

	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(std::filesystem::equivalent(std::filesystem::path(getGroup(i)->getFilePath()), actualFilePath, errorCode)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

GroupPanel * GroupEditorPanel::getGroupPanel(size_t groupPanelIndex) const {
	if(groupPanelIndex >= m_notebook->GetPageCount()) {
		return nullptr;
	}

	return dynamic_cast<GroupPanel *>(m_notebook->GetPage(groupPanelIndex));
}

GroupPanel * GroupEditorPanel::getPanelWithGroup(const Group * group) const {
	return getGroupPanel(indexOfPanelWithGroup(group));
}

GroupPanel * GroupEditorPanel::getPanelWithGroupFilePath(const std::string & filePath) const {
	return getGroupPanel(indexOfPanelWithGroupFilePath(filePath));
}

GroupPanel * GroupEditorPanel::getCurrentGroupPanel() const {
	return dynamic_cast<GroupPanel *>(m_notebook->GetCurrentPage());
}

Group * GroupEditorPanel::getGroup(size_t groupPanelIndex) const {
	GroupPanel * groupPanel = getGroupPanel(groupPanelIndex);

	if(groupPanel == nullptr) {
		return nullptr;
	}

	return groupPanel->getGroup();
}

Group * GroupEditorPanel::getCurrentGroup() const {
	GroupPanel * groupPanel = getCurrentGroupPanel();

	if(groupPanel == nullptr) {
		return nullptr;
	}

	return groupPanel->getGroup();
}

void GroupEditorPanel::update() {
	updateButtons();
	updateGroupPanelNames();
}

bool GroupEditorPanel::updateGroupPanel(size_t groupPanelIndex) {
	GroupPanel * groupPanel = getGroupPanel(groupPanelIndex);

	if(groupPanel == nullptr) {
		return false;
	}

	updateButtons();
	updateGroupPanelName(groupPanelIndex);

	return true;
}

void GroupEditorPanel::updateGroupPanelNames() {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		updateGroupPanelName(i);
	}
}

bool GroupEditorPanel::updateGroupPanelName(size_t groupPanelIndex) {
	GroupPanel * groupPanel = getGroupPanel(groupPanelIndex);

	if(groupPanel == nullptr) {
		return false;
	}

	m_notebook->SetPageText(groupPanelIndex, groupPanel->getPanelName());

	return true;
}

void GroupEditorPanel::updateButtons() {
	GroupPanel * groupPanel = getCurrentGroupPanel();

	if(groupPanel == nullptr) {
		m_saveButton->Disable();
		m_saveAsButton->Disable();
		m_addFilesButton->Disable();
		m_removeFilesButton->Disable();
		m_replaceFileButton->Disable();
		m_renameFileButton->Disable();
		m_extractFilesButton->Disable();
		m_extractAllFilesButton->Disable();
		m_closeButton->Disable();
		m_closeAllButton->Disable();
		return;
	}

	Group * group = groupPanel->getGroup();

	bool isGroupModified = group->isModified();
	bool groupHasFilePath = group->hasFilePath();
	size_t groupFileCount = group->numberOfFiles();
	size_t selectedGroupFileCount = groupPanel->numberOfFilesSelected();

	WXUtilities::setButtonEnabled(m_saveButton, isGroupModified || !groupHasFilePath);
	WXUtilities::setButtonEnabled(m_saveAsButton, true);
	WXUtilities::setButtonEnabled(m_addFilesButton , true);
	WXUtilities::setButtonEnabled(m_removeFilesButton, selectedGroupFileCount != 0);
	WXUtilities::setButtonEnabled(m_replaceFileButton, selectedGroupFileCount == 1);
	WXUtilities::setButtonEnabled(m_renameFileButton, selectedGroupFileCount == 1);
	WXUtilities::setButtonEnabled(m_extractFilesButton, selectedGroupFileCount != 0);
	WXUtilities::setButtonEnabled(m_extractAllFilesButton, groupFileCount != 0);
	WXUtilities::setButtonEnabled(m_closeButton, true);
	WXUtilities::setButtonEnabled(m_closeAllButton, true);

	m_removeFilesButton->SetLabel(selectedGroupFileCount == 1 ? "Remove File" : "Remove Files");
	m_extractFilesButton->SetLabel(selectedGroupFileCount == 1 ? "Extract File" : "Extract Files");
}

bool GroupEditorPanel::addGroupPanel(GroupPanel * groupPanel) {
	if(groupPanel == nullptr || hasGroupPanel(groupPanel)) {
		return false;
	}

	m_groupPanelConnections.push_back(SignalConnectionGroup(
		groupPanel->groupModified.connect(std::bind(&GroupEditorPanel::onGroupModified, this, std::placeholders::_1)),
		groupPanel->groupFileSelectionChanged.connect(std::bind(&GroupEditorPanel::onGroupFileSelectionChanged, this, std::placeholders::_1))
	));

	m_notebook->AddPage(groupPanel, groupPanel->getPanelName());
	m_notebook->ChangeSelection(m_notebook->GetPageCount() - 1);

	updateButtons();

	return true;
}

bool GroupEditorPanel::newGroup() {
	int selectedGroupTypeIndex = wxGetSingleChoiceIndex(
		"Please choose a group type to create:",
		"Choose Group Type",
		WXUtilities::createItemWXArrayString({ "Build Engine Group", "Sunstorm Interactive File Collection" }),
		0,
		this
	);

	std::unique_ptr<Group> newGroup;

	switch(selectedGroupTypeIndex) {
		case 0: {
			newGroup = std::make_unique<GroupGRP>();
			break;
		}

		case 1: {
			newGroup = std::make_unique<GroupSSI>();
			break;
		}

		case wxNOT_FOUND:
		default: {
			return false;
		}
	}

	if(!addGroupPanel(new GroupPanel(std::move(newGroup), m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL))) {
		return false;
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["groupType"] = selectedGroupTypeIndex == 0 ? "GRP" : "SSI";

		SegmentAnalytics::getInstance()->track("New Group Created", properties);
	}

	return true;
}

bool GroupEditorPanel::openGroup(const std::string & filePath) {
	size_t existingGroupPanelIndex = indexOfPanelWithGroupFilePath(filePath);

	if(existingGroupPanelIndex != std::numeric_limits<size_t>::max()) {
		wxMessageBox(fmt::format("Group file '{}' already open!", Utilities::getFileName(filePath)), "Group Already Open", wxOK | wxICON_INFORMATION, this);

		m_notebook->ChangeSelection(existingGroupPanelIndex);

		return false;
	}

	bool isSSIGroup = false;
	std::unique_ptr<Group> group;
	std::string_view fileExtension(Utilities::getFileExtension(filePath));

	if(Utilities::areStringsEqualIgnoreCase(fileExtension, "SSI")) {
		group = GroupSSI::loadFrom(filePath);
		isSSIGroup = true;
	}
	else {
		group = GroupGRP::loadFrom(filePath);
	}

	if(!Group::isValid(group.get())) {
		wxMessageBox(fmt::format("Failed to open group file: '{}'.", filePath), "Group Loading Failed", wxOK | wxICON_ERROR, this);
		return false;
	}

	GroupPanel * groupPanel = new GroupPanel(std::move(group), m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	if(!addGroupPanel(groupPanel)) {
		return false;
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		const Group * group = groupPanel->getGroup();

		std::map<std::string, std::any> properties;
		properties["fileName"] = group->getFileName();
		properties["fileSize"] = group->getSizeInBytes();
		properties["numberOfFiles"] = group->numberOfFiles();
		properties["groupType"] = isSSIGroup ? "SSI" : "GRP";

		SegmentAnalytics::getInstance()->track("Group Opened", properties);
	}

	return true;
}

size_t GroupEditorPanel::openGroups() {
	wxFileDialog openFilesDialog(this, "Open Group(s) from File(s)", std::filesystem::current_path().string(), "", FILE_DIALOG_GROUPED_FILE_TYPES, wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST, wxDefaultPosition, wxDefaultSize, "Open Groups");
	int openFilesResult = openFilesDialog.ShowModal();

	if(openFilesResult == wxID_CANCEL) {
		return 0;
	}

	size_t openedGroupCount = 0;
	wxArrayString filePathArray;
	openFilesDialog.GetPaths(filePathArray);

	for(size_t i = 0; i < filePathArray.GetCount(); i++) {
		if(openGroup(filePathArray[i])) {
			openedGroupCount++;
		}
	}

	return openedGroupCount;
}

bool GroupEditorPanel::createGroupFromDirectory(const std::string & directoryPath) {
	int selectedGroupTypeIndex = wxGetSingleChoiceIndex(
		"Please choose a group type to create:",
		"Choose Group Type",
		WXUtilities::createItemWXArrayString({ "Build Engine Group", "Sunstorm Interactive File Collection" }),
		0,
		this
	);

	std::unique_ptr<Group> group;

	switch(selectedGroupTypeIndex) {
		case 0: {
			group = GroupGRP::createFrom(directoryPath);
			break;
		}

		case 1: {
			group = GroupSSI::createFrom(directoryPath);
			break;
		}

		case wxNOT_FOUND:
		default: {
			return false;
		}
	}

	if(!Group::isValid(group.get())) {
		wxMessageBox(fmt::format("Failed to create group from directory: '{}'.", directoryPath), "Group Creation Failed", wxOK | wxICON_ERROR, this);
		return false;
	}

	GroupPanel * groupPanel = new GroupPanel(std::move(group), m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	if(!addGroupPanel(groupPanel)) {
		return false;
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		const Group * group = groupPanel->getGroup();

		std::map<std::string, std::any> properties;
		properties["directoryName"] = Utilities::getFileName(directoryPath);
		properties["fileSize"] = group->getSizeInBytes();
		properties["numberOfFiles"] = group->numberOfFiles();
		properties["groupType"] = selectedGroupTypeIndex == 0 ? "GRP" : "SSI";

		SegmentAnalytics::getInstance()->track("Group Created from Directory", properties);
	}

	return true;
}

bool GroupEditorPanel::createGroupFromDirectory() {
	wxDirDialog selectDirectoryDialog(this, "Create Group from Directory", std::filesystem::current_path().string(), wxDD_DIR_MUST_EXIST, wxDefaultPosition, wxDefaultSize, "Create Group");
	int selectDirectoryResult = selectDirectoryDialog.ShowModal();

	if(selectDirectoryResult == wxID_CANCEL) {
		return false;
	}

	return createGroupFromDirectory(selectDirectoryDialog.GetPath());
}

bool GroupEditorPanel::saveGroup(Group * group) {
	if(!Group::isValid(group)) {
		return false;
	}

	std::string fileDialogFileTypes;
	std::string newGroupFileName;

	if(dynamic_cast<GroupSSI *>(group) != nullptr) {
		newGroupFileName = DEFAULT_NEW_SSI_FILE_NAME;
		fileDialogFileTypes = FILE_DIALOG_SSI_FILE_TYPE;
	}
	else {
		newGroupFileName = DEFAULT_NEW_GRP_FILE_NAME;
		fileDialogFileTypes = FILE_DIALOG_GRP_FILE_TYPE;
	}

	if(group->getFilePath().empty()) {
		wxFileDialog saveFileDialog(this, "Save Group to File", std::filesystem::current_path().string(), newGroupFileName, fileDialogFileTypes, wxFD_SAVE, wxDefaultPosition, wxDefaultSize, "Save Group");
		int saveFileResult = saveFileDialog.ShowModal();

		if(saveFileResult == wxID_CANCEL) {
			return false;
		}

		std::string newFilePath(saveFileDialog.GetPath());

		if(std::filesystem::exists(std::filesystem::path(newFilePath))) {
			int overwriteFileConfirmationResult = wxMessageBox(fmt::format("Group file already exists at path:\n\n{}\n\n Are you sure that want to overwrite this file?", newFilePath), "Overwrite Group File", wxYES_NO | wxCANCEL | wxICON_WARNING, this);

			if(overwriteFileConfirmationResult != wxYES) {
				return false;
			}
		}

		group->setFilePath(newFilePath);

		updateGroupPanelName(indexOfPanelWithGroup(group));
	}

	if(group->save()) {
		spdlog::info("Successfully saved group to file: '{}'.", group->getFilePath());
	}
	else {
		wxMessageBox(fmt::format("Failed to save group to file: '{}'.", group->getFilePath()), "Group Writing Failed", wxOK | wxICON_ERROR, this);
		return false;
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["fileName"] = group->getFileName();
		properties["fileSize"] = group->getSizeInBytes();
		properties["numberOfFiles"] = group->numberOfFiles();
		properties["groupType"] = dynamic_cast<GroupSSI *>(group) != nullptr ? "SSI" : "GRP";

		SegmentAnalytics::getInstance()->track("Group Saved", properties);
	}

	return true;
}

bool GroupEditorPanel::saveCurrentGroup() {
	return saveGroup(getCurrentGroup());
}

bool GroupEditorPanel::saveGroupAs(Group * group) {
	if(!Group::isValid(group)) {
		return false;
	}

	std::string basePath(Utilities::getBasePath(group->getFilePath()));

	if(basePath.empty()) {
		basePath = std::filesystem::current_path().string();
	}

	std::string previousFileName(group->getFileName());

	std::string newFileName;

	if(!group->getFilePath().empty()) {
		newFileName = group->getFileName();
	}

	bool isSSIGroup = false;
	std::string fileDialogFileTypes;

	if(dynamic_cast<GroupSSI *>(group) != nullptr) {
		isSSIGroup = true;
		fileDialogFileTypes = FILE_DIALOG_SSI_FILE_TYPE;

		if(newFileName.empty()) {
			newFileName = DEFAULT_NEW_SSI_FILE_NAME;
		}
	}
	else {
		fileDialogFileTypes = FILE_DIALOG_GRP_FILE_TYPE;

		if(newFileName.empty()) {
			newFileName = DEFAULT_NEW_GRP_FILE_NAME;
		}
	}

	wxFileDialog saveFileAsDialog(this, "Save Group to New File", Utilities::getAbsoluteFilePath(basePath, std::filesystem::current_path().string()), newFileName, fileDialogFileTypes, wxFD_SAVE, wxDefaultPosition, wxDefaultSize, "Save Group As");
	int saveFileAsResult = saveFileAsDialog.ShowModal();

	if(saveFileAsResult == wxID_CANCEL) {
		return false;
	}

	std::string newFilePath(saveFileAsDialog.GetPath());

	if(std::filesystem::exists(std::filesystem::path(newFilePath))) {
		int overwriteFileConfirmationResult = wxMessageBox(fmt::format("Group file already exists at path:\n\n{}\n\n Are you sure that want to overwrite this file?", newFilePath), "Overwrite Group File", wxYES_NO | wxCANCEL | wxICON_WARNING, this);

		if(overwriteFileConfirmationResult != wxYES) {
			return false;
		}
	}

	group->setFilePath(newFilePath);

	updateGroupPanelName(indexOfPanelWithGroup(group));

	if(group->save()) {
		spdlog::info("Successfully saved group to file: '{}'.", group->getFilePath());
	}
	else {
		wxMessageBox(fmt::format("Failed to save group to new file: '{}'.", group->getFilePath()), "Group Writing Failed", wxOK | wxICON_ERROR, this);
		return false;
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["newFileName"] = group->getFileName();
		properties["previousFileName"] = previousFileName;
		properties["fileSize"] = group->getSizeInBytes();
		properties["numberOfFiles"] = group->numberOfFiles();
		properties["groupType"] = isSSIGroup ? "SSI" : "GRP";

		SegmentAnalytics::getInstance()->track("Group Saved As", properties);
	}

	return true;
}

bool GroupEditorPanel::saveCurrentGroupAs() {
	return saveGroupAs(getCurrentGroup());
}

size_t GroupEditorPanel::addFilesToGroup(Group * group) {
	if(!Group::isValid(group)) {
		return 0;
	}

	wxFileDialog addFilesDialog(this, "Add Files to Group", std::filesystem::current_path().string(), "", FILE_DIALOG_ALL_FILES, wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST, wxDefaultPosition, wxDefaultSize, "Add Files");
	int addFilesResult = addFilesDialog.ShowModal();

	if(addFilesResult == wxID_CANCEL) {
		return 0;
	}

	bool replaceExistingFiles = false;
	std::string formattedFileName;
	std::vector<std::string> filePaths;
	std::vector<std::string> existingFileNames;
	std::stringstream existingFileNamesStream;
	wxArrayString filePathArray;
	addFilesDialog.GetPaths(filePathArray);

	for(size_t i = 0; i < filePathArray.GetCount(); i++) {
		filePaths.push_back(filePathArray[i]);

		formattedFileName = GroupFile::formatFileName(Utilities::getFileName(filePaths.back()));

		if(group->hasFileWithName(formattedFileName)) {
			bool duplicateFileName = false;

			for(const std::string & currentExistingFileName : existingFileNames) {
				if(Utilities::areStringsEqualIgnoreCase(currentExistingFileName, formattedFileName)) {
					duplicateFileName = true;
					break;
				}
			}

			if(!duplicateFileName) {
				existingFileNames.push_back(formattedFileName);

				if(existingFileNamesStream.tellp() != 0) {
					existingFileNamesStream << ", ";
				}

				existingFileNamesStream << '\'' << formattedFileName << '\'';
			}
		}
	}

	if(!existingFileNames.empty()) {
		bool singleFile = existingFileNames.size() == 1;

		int replaceFilesResult = wxMessageBox(
			fmt::format(
				"File{0} with name{0} {1} already exist{2}! Would you like to replace {3}?",
				singleFile ? "" : "s",
				existingFileNamesStream.str(),
				singleFile ? "s" : "",
				singleFile ? "this file" : "these files"
			),
			fmt::format(
				"Replace File{}",
				singleFile ? "" : "s"
			),
			wxICON_QUESTION | wxYES_NO | wxCANCEL,
			this
		);

		if(replaceFilesResult == wxYES) {
			replaceExistingFiles = true;
		}
		else if(replaceFilesResult == wxNO) {
			replaceExistingFiles = false;
		}
		else if(replaceFilesResult == wxCANCEL) {
			return 0;
		}
	}

	if(!group->addFiles(filePaths, replaceExistingFiles)) {
		return false;
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;

		if(group->hasFilePath()) {
			properties["fileName"] = group->getFileName();
		}

		properties["fileSize"] = group->getSizeInBytes();
		properties["numberOfFiles"] = group->numberOfFiles();
		properties["numberOfFilesAdded"] = filePaths.size();
		properties["groupType"] = dynamic_cast<const GroupSSI *>(group) != nullptr ? "SSI" : "GRP";

		SegmentAnalytics::getInstance()->track("Files Added to Group", properties);
	}

	return true;
}

size_t GroupEditorPanel::addFilesToCurrentGroup() {
	return addFilesToGroup(getCurrentGroup());
}

size_t GroupEditorPanel::removeSelectedFilesFromGroup(Group * group) {
	if(!Group::isValid(group)) {
		return 0;
	}

	GroupPanel * groupPanel = getPanelWithGroup(group);

	if(groupPanel == nullptr) {
		return 0;
	}

	std::vector<std::shared_ptr<GroupFile>> selectedFiles(groupPanel->getSelectedFiles());

	if(!group->removeFiles(selectedFiles)) {
		return false;
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;

		if(group->hasFilePath()) {
			properties["fileName"] = group->getFileName();
		}

		properties["fileSize"] = group->getSizeInBytes();
		properties["numberOfFiles"] = group->numberOfFiles();
		properties["numberOfFilesRemoved"] = selectedFiles.size();
		properties["groupType"] = dynamic_cast<GroupSSI *>(group) != nullptr ? "SSI" : "GRP";

		SegmentAnalytics::getInstance()->track("Files Removed from Group", properties);
	}

	return true;
}

size_t GroupEditorPanel::removeSelectedFilesFromCurrentGroup() {
	return removeSelectedFilesFromGroup(getCurrentGroup());
}

bool GroupEditorPanel::replaceSelectedFileInGroup(Group * group) {
	if(!Group::isValid(group)) {
		return false;
	}

	GroupPanel * groupPanel = getPanelWithGroup(group);

	if(groupPanel == nullptr) {
		return false;
	}

	std::vector<std::shared_ptr<GroupFile>> selectedFiles(groupPanel->getSelectedFiles());

	if(selectedFiles.size() != 1) {
		return false;
	}

	std::shared_ptr<GroupFile> selectedFile(selectedFiles[0]);

	wxFileDialog replaceFileDialog(this, "Select a Replacement File", std::filesystem::current_path().string(), "", "All Files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST, wxDefaultPosition, wxDefaultSize, "Replace File");
	int replaceFileResult = replaceFileDialog.ShowModal();

	if(replaceFileResult == wxID_CANCEL) {
		return 0;
	}

	bool renameFile = false;
	size_t indexOfExistingFileWithNewName = std::numeric_limits<size_t>::max();
	std::string newFilePath(replaceFileDialog.GetPath());
	std::string formattedNewFileName(GroupFile::formatFileName(Utilities::getFileName(newFilePath)));

	if(!Utilities::areStringsEqualIgnoreCase(selectedFile->getFileName(), formattedNewFileName)) {
		int renameExistingFileResult = wxMessageBox(fmt::format("Would you like to change the file name from '{}' to '{}', or keep the previous file name instead?", selectedFile->getFileName(), formattedNewFileName), "Rename File", wxICON_QUESTION | wxYES_NO | wxCANCEL, this);

		if(renameExistingFileResult == wxYES) {
			renameFile = true;
		}
		else if(renameExistingFileResult == wxNO) {
			renameFile = false;
		}
		else if(renameExistingFileResult == wxCANCEL) {
			return false;
		}

		if(renameFile) {
			indexOfExistingFileWithNewName = group->indexOfFileWithName(formattedNewFileName);

			if(indexOfExistingFileWithNewName != std::numeric_limits<size_t>::max()) {
				int replaceExistingFileResult = wxMessageBox(fmt::format("Group already has a file named '{}'! Would you like to replace it?", formattedNewFileName), "Replace File", wxICON_QUESTION | wxYES_NO | wxCANCEL, this);

				if(replaceExistingFileResult != wxYES) {
					return false;
				}
			}
		}
	}

	if(indexOfExistingFileWithNewName != std::numeric_limits<size_t>::max()) {
		if(!group->removeFile(indexOfExistingFileWithNewName)) {
			return false;
		}
	}

	size_t existingGroupFileIndex = group->indexOfFile(*selectedFile);
	std::string previousGroupFileName(selectedFile->getFileName());
	size_t previousGroupFileSize = selectedFile->getSize();

	if(!group->replaceFile(*selectedFile, newFilePath, !renameFile)) {
		return false;
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::shared_ptr<GroupFile> newGroupFile(group->getFile(existingGroupFileIndex));

		std::map<std::string, std::any> properties;

		if(group->hasFilePath()) {
			properties["fileName"] = group->getFileName();
		}

		properties["previousGroupFileName"] = previousGroupFileName;
		properties["previousGroupFileSize"] = previousGroupFileSize;
		properties["newGroupFileName"] = newGroupFile->getFileName();
		properties["newGroupFileSize"] = newGroupFile->getSize();
		properties["fileSize"] = group->getSizeInBytes();
		properties["numberOfFiles"] = group->numberOfFiles();
		properties["groupType"] = dynamic_cast<GroupSSI *>(group) != nullptr ? "SSI" : "GRP";

		SegmentAnalytics::getInstance()->track("Group File Replaced", properties);
	}

	return true;
}

bool GroupEditorPanel::replaceSelectedFileInCurrentGroup() {
	return replaceSelectedFileInGroup(getCurrentGroup());
}

bool GroupEditorPanel::renameSelectedFileInGroup(Group * group) {
	if(!Group::isValid(group)) {
		return false;
	}

	GroupPanel * groupPanel = getPanelWithGroup(group);

	if(groupPanel == nullptr) {
		return false;
	}

	std::vector<std::shared_ptr<GroupFile>> selectedFiles(groupPanel->getSelectedFiles());

	if(selectedFiles.size() != 1) {
		return false;
	}

	std::shared_ptr<GroupFile> selectedFile(selectedFiles[0]);

	wxTextEntryDialog newFileNameDialog(this, "Enter a new file name:", "Rename File", selectedFile->getFileName(), wxOK | wxCANCEL, wxDefaultPosition);
	newFileNameDialog.SetMaxLength(GroupFile::MAX_FILE_NAME_LENGTH);
	int renameFileResult = newFileNameDialog.ShowModal();

	if(renameFileResult == wxCANCEL) {
		return false;
	}

	std::string newFileName(newFileNameDialog.GetValue());

	if(newFileName.empty() || Utilities::areStringsEqualIgnoreCase(selectedFile->getFileName(), newFileName)) {
		return false;
	}

	size_t indexOfExistingFile = group->indexOfFileWithName(newFileName);

	if(indexOfExistingFile != std::numeric_limits<size_t>::max()) {
		int replaceFileResult = wxMessageBox(fmt::format("Group already has a file named '{}'! Would you like to replace it?", newFileName), "Replace File", wxICON_QUESTION | wxYES_NO | wxCANCEL, this);

		if(replaceFileResult != wxYES) {
			return false;
		}

		if(!group->removeFile(indexOfExistingFile)) {
			return false;
		}
	}

	size_t fileIndex = group->indexOfFile(*selectedFile);
	std::string previousGroupFileName(selectedFile->getFileName());

	if(!group->renameFile(*selectedFile, newFileName)) {
		return false;
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::shared_ptr<GroupFile> groupFile(group->getFile(fileIndex));

		std::map<std::string, std::any> properties;

		if(group->hasFilePath()) {
			properties["fileName"] = group->getFileName();
		}

		properties["previousGroupFileName"] = previousGroupFileName;
		properties["newGroupFileName"] = groupFile->getFileName();
		properties["fileSize"] = group->getSizeInBytes();
		properties["numberOfFiles"] = group->numberOfFiles();
		properties["groupType"] = dynamic_cast<GroupSSI *>(group) != nullptr ? "SSI" : "GRP";

		SegmentAnalytics::getInstance()->track("Group File Renamed", properties);
	}

	return true;
}

bool GroupEditorPanel::renameSelectedFileInCurrentGroup() {
	return renameSelectedFileInGroup(getCurrentGroup());
}

std::vector<std::shared_ptr<GroupFile>> GroupEditorPanel::extractFilesFromGroup(const Group * group, const std::vector<std::shared_ptr<GroupFile>> & files) {
	if(!Group::isValid(group)) {
		return {};
	}

	for(const std::shared_ptr<GroupFile> & file : files) {
		if(file->getParentGroup() != group) {
			spdlog::warn("Tried to extract file which did not belong to the provided group.");
			return {};
		}
	}

	std::string basePath(Utilities::getBasePath(group->getFilePath()));

	if(basePath.empty()) {
		basePath = std::filesystem::current_path().string();
	}

	wxDirDialog extractFilesDialog(this, "Select File Extraction Destination Directory", Utilities::getAbsoluteFilePath(basePath, std::filesystem::current_path().string()), wxDD_DIR_MUST_EXIST, wxDefaultPosition, wxDefaultSize, "Extract Files");
	int extractFilesResult = extractFilesDialog.ShowModal();

	if(extractFilesResult == wxID_CANCEL) {
		return {};
	}

	bool overwriteFiles = false;
	std::vector<std::string> existingFileNames;
	std::stringstream existingFileNamesStream;
	std::string destinationDirectoryPath(extractFilesDialog.GetPath());

	for(const std::shared_ptr<GroupFile> & file : files) {
		if(!std::filesystem::exists(std::filesystem::path(Utilities::joinPaths(destinationDirectoryPath, file->getFileName())))) {
			continue;
		}

		existingFileNames.push_back(file->getFileName());

		if(existingFileNamesStream.tellp() != 0) {
			existingFileNamesStream << ", ";
		}

		existingFileNamesStream << '\'' << file->getFileName() << '\'';
	}

	if(!existingFileNames.empty()) {
		bool singleFile = existingFileNames.size() == 1;

		int replaceFilesResult = wxMessageBox(
			fmt::format(
				"File{0} with name{0} {1} already exist{2} in destination directory: '{3}'! Would you like to overwrite {4}?",
				singleFile ? "" : "s",
				existingFileNamesStream.str(),
				singleFile ? "s" : "",
				destinationDirectoryPath,
				singleFile ? "this file" : "these files"
			),
			fmt::format(
				"Overwrite File{}",
				singleFile ? "" : "s"
			),
			wxICON_QUESTION | wxYES_NO | wxCANCEL,
			this
		);

		if(replaceFilesResult == wxYES) {
			overwriteFiles = true;
		}
		else if(replaceFilesResult == wxNO) {
			overwriteFiles = false;
		}
		else if(replaceFilesResult == wxCANCEL) {
			return {};
		}
	}

	std::vector<std::shared_ptr<GroupFile>> extractedFiles;

	for(const std::shared_ptr<GroupFile> & file : files) {
		if(file->writeTo(destinationDirectoryPath, overwriteFiles)) {
			extractedFiles.push_back(file);
		}
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;

		if(group->hasFilePath()) {
			properties["fileName"] = group->getFileName();
		}

		properties["fileSize"] = group->getSizeInBytes();
		properties["numberOfFiles"] = group->numberOfFiles();
		properties["numberOfFilesExtracted"] = extractedFiles.size();
		properties["groupType"] = dynamic_cast<const GroupSSI *>(group) != nullptr ? "SSI" : "GRP";

		SegmentAnalytics::getInstance()->track("Group Files Extracted", properties);
	}

	wxMessageBox(fmt::format("Extracted {} file{} to directory: '{}'.", extractedFiles.size(), extractedFiles.size() == 1 ? "" : "s", destinationDirectoryPath), "Extraction Summary", wxOK | wxICON_INFORMATION, this);

	return extractedFiles;
}

std::vector<std::shared_ptr<GroupFile>> GroupEditorPanel::extractSelectedFilesFromGroup(const Group * group) {
	if(!Group::isValid(group)) {
		return {};
	}

	GroupPanel * groupPanel = getPanelWithGroup(group);

	if(groupPanel == nullptr) {
		return {};
	}

	return extractFilesFromGroup(group, groupPanel->getSelectedFiles());
}

std::vector<std::shared_ptr<GroupFile>> GroupEditorPanel::extractSelectedFilesFromCurrentGroup() {
	return extractSelectedFilesFromGroup(getCurrentGroup());
}

std::vector<std::shared_ptr<GroupFile>> GroupEditorPanel::extractAllFilesFromGroup(const Group * group) {
	return extractFilesFromGroup(group, group->getFiles());
}

std::vector<std::shared_ptr<GroupFile>> GroupEditorPanel::extractAllFilesFromCurrentGroup() {
	return extractAllFilesFromGroup(getCurrentGroup());
}

bool GroupEditorPanel::closeGroupPanel(size_t groupPanelIndex) {
	Group * group = getGroup(groupPanelIndex);

	if(group == nullptr) {
		return false;
	}

	std::string groupFileName(group->getFileName());
	size_t groupFileSize = group->getSizeInBytes();
	size_t numberOfFiles = group->numberOfFiles();
	bool isSSIGroup = dynamic_cast<GroupSSI *>(group) != nullptr;

	if(group->isModified()) {
		int saveChangesResult = wxMessageBox("Group modifications have not been saved! Would you like to save your changes?", "Save Changes", wxICON_QUESTION | wxYES_NO | wxCANCEL, this);

		if(saveChangesResult == wxYES) {
			if(!saveGroup(group)) {
				return false;
			}
		}
		else if(saveChangesResult == wxCANCEL) {
			return false;
		}
	}

	if(m_notebook->GetPageCount() == 1) {
		m_newButton->SetFocus();
	}

	m_groupPanelConnections[groupPanelIndex].disconnect();
	m_groupPanelConnections.erase(m_groupPanelConnections.begin() + groupPanelIndex);

	GroupPanel * groupPanel = getGroupPanel(groupPanelIndex);
	m_notebook->RemovePage(groupPanelIndex);
	delete groupPanel;

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;

		if(!groupFileName.empty()) {
			properties["fileName"] = groupFileName;
		}

		properties["fileSize"] = groupFileSize;
		properties["numberOfFiles"] = numberOfFiles;
		properties["groupType"] = isSSIGroup ? "SSI" : "GRP";

		SegmentAnalytics::getInstance()->track("Group Closed", properties);
	}

	return true;
}

bool GroupEditorPanel::closeCurrentGroupPanel() {
	int currentGroupPanelIndex = m_notebook->GetSelection();

	if(currentGroupPanelIndex == wxNOT_FOUND) {
		return false;
	}

	GroupPanel * groupPanel = getGroupPanel(currentGroupPanelIndex);

	if(!closeGroupPanel(currentGroupPanelIndex)) {
		return false;
	}

	updateButtons();

	return true;
}

bool GroupEditorPanel::closeAllGroupPanels() {
	while(m_notebook->GetPageCount() != 0) {
		if(!closeCurrentGroupPanel()) {
			return false;
		}
	}

	return true;
}

void GroupEditorPanel::onNotebookPageChanged(wxBookCtrlEvent & event) {
	updateButtons();
}

void GroupEditorPanel::onNewButtonPressed(wxCommandEvent & event) {
	newGroup();
}

void GroupEditorPanel::onOpenButtonPressed(wxCommandEvent & event) {
	openGroups();
}

void GroupEditorPanel::onCreateFromButtonPressed(wxCommandEvent & event) {
	createGroupFromDirectory();
}

void GroupEditorPanel::onSaveButtonPressed(wxCommandEvent & event) {
	saveCurrentGroup();
}

void GroupEditorPanel::onSaveAsButtonPressed(wxCommandEvent & event) {
	saveCurrentGroupAs();
}

void GroupEditorPanel::onAddFilesButtonPressed(wxCommandEvent & event) {
	addFilesToCurrentGroup();
}

void GroupEditorPanel::onRemoveFilesButtonPressed(wxCommandEvent & event) {
	removeSelectedFilesFromCurrentGroup();
}

void GroupEditorPanel::onReplaceFileButtonPressed(wxCommandEvent & event) {
	replaceSelectedFileInCurrentGroup();
}

void GroupEditorPanel::onRenameFileButtonPressed(wxCommandEvent & event) {
	renameSelectedFileInCurrentGroup();
}

void GroupEditorPanel::onExtractFilesButtonPressed(wxCommandEvent & event) {
	extractSelectedFilesFromCurrentGroup();
}

void GroupEditorPanel::onExtractAllFilesButtonPressed(wxCommandEvent & event) {
	extractAllFilesFromCurrentGroup();
}

void GroupEditorPanel::onCloseButtonPressed(wxCommandEvent & event) {
	closeCurrentGroupPanel();
}

void GroupEditorPanel::onCloseAllButtonPressed(wxCommandEvent & event) {
	closeAllGroupPanels();
}

void GroupEditorPanel::onGroupModified(GroupPanel & groupPanel) {
	updateGroupPanel(indexOfGroupPanel(&groupPanel));
}

void GroupEditorPanel::onGroupFileSelectionChanged(GroupPanel & groupPanel) {
	updateButtons();
}
