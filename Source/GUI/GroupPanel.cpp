#include "GroupPanel.h"

#include "Group/Group.h"
#include "Group/GroupFile.h"
#include "WXUtilities.h"

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <wx/gbsizer.h>
#include <wx/wrapsizer.h>

GroupPanel::GroupPanel(std::unique_ptr<Group> group, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Group Information")
	, m_group(group != nullptr ? std::move(group) : std::make_unique<Group>())
	, m_numberOfFilesText(nullptr)
	, m_groupSizeText(nullptr)
	, m_fileExtensionsText(nullptr)
	, m_fileListBox(nullptr)
	, m_fileInfoBox(nullptr)
	, m_fileInfoPanel(nullptr)
	, m_fileNameLabel(nullptr)
	, m_fileNameText(nullptr)
	, m_numberOfFilesSelectedLabel(nullptr)
	, m_numberOfFilesSelectedText(nullptr)
	, m_fileSizeLabel(nullptr)
	, m_fileSizeText(nullptr)
	, m_fileInfoBoxSizer(nullptr)
	, m_groupPropertiesSizer(nullptr) {
	wxPanel * groupPropertiesPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	wxPanel * numberOfFilesPropertyPanel = new wxPanel(groupPropertiesPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	wxStaticText * numberOfFilesLabel = new wxStaticText(numberOfFilesPropertyPanel, wxID_ANY, "Number of Files:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	numberOfFilesLabel->SetFont(numberOfFilesLabel->GetFont().MakeBold());
	m_numberOfFilesText = new wxStaticText(numberOfFilesPropertyPanel, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	wxPanel * groupSizePropertyPanel = new wxPanel(groupPropertiesPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	wxStaticText * groupSizeLabel = new wxStaticText(groupSizePropertyPanel, wxID_ANY, "Group Size:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	groupSizeLabel->SetFont(groupSizeLabel->GetFont().MakeBold());
	m_groupSizeText = new wxStaticText(groupSizePropertyPanel, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	wxPanel * fileExtensionsPropertyPanel = new wxPanel(groupPropertiesPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	wxStaticText * fileExtensionsLabel = new wxStaticText(fileExtensionsPropertyPanel, wxID_ANY, "File Extensions:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	fileExtensionsLabel->SetFont(fileExtensionsLabel->GetFont().MakeBold());
	m_fileExtensionsText = new wxStaticText(fileExtensionsPropertyPanel, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_fileListBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, {}, wxLB_EXTENDED | wxLB_ALWAYS_SB);
	m_fileListBox->Bind(wxEVT_LISTBOX, &GroupPanel::onFileSelected, this);
	m_fileListBox->SetMinSize(wxSize(150, m_fileListBox->GetMinSize().y));

	m_fileInfoBox = new wxStaticBox(this, wxID_ANY, "File Information", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_fileInfoBox->SetOwnFont(m_fileInfoBox->GetFont().MakeBold());

	m_fileInfoPanel = new wxPanel(m_fileInfoBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "Group File Information");
	m_fileInfoPanel->Hide();

	m_fileNameLabel = new wxStaticText(m_fileInfoPanel, wxID_ANY, "File Name:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_fileNameLabel->SetFont(m_fileNameLabel->GetFont().MakeBold());
	m_fileNameText = new wxStaticText(m_fileInfoPanel, wxID_ANY, "N/A", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "File Name");

	m_numberOfFilesSelectedLabel = new wxStaticText(m_fileInfoPanel, wxID_ANY, "Number of Files Selected:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_numberOfFilesSelectedLabel->SetFont(m_numberOfFilesSelectedLabel->GetFont().MakeBold());
	m_numberOfFilesSelectedLabel->Hide();
	m_numberOfFilesSelectedText = new wxStaticText(m_fileInfoPanel, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Number of Files Selected");
	m_numberOfFilesSelectedText->Hide();

	m_fileSizeLabel = new wxStaticText(m_fileInfoPanel, wxID_ANY, "File Size:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_fileSizeLabel->SetFont(m_fileSizeLabel->GetFont().MakeBold());
	m_fileSizeText = new wxStaticText(m_fileInfoPanel, wxID_ANY, "0", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "File Size");

	int border = 5;

	wxFlexGridSizer * numberOfFilesPropertySizer = new wxFlexGridSizer(2, 3, 3);
	numberOfFilesPropertySizer->Add(numberOfFilesLabel, 0, wxALIGN_CENTER_VERTICAL, border);
	numberOfFilesPropertySizer->Add(m_numberOfFilesText, 0, wxALIGN_CENTER_VERTICAL, border);
	numberOfFilesPropertyPanel->SetSizer(numberOfFilesPropertySizer);

	wxFlexGridSizer * groupSizePropertySizer = new wxFlexGridSizer(2, 3, 3);
	groupSizePropertySizer->Add(groupSizeLabel, 0, wxALIGN_CENTER_VERTICAL, border);
	groupSizePropertySizer->Add(m_groupSizeText, 0, wxALIGN_CENTER_VERTICAL, border);
	groupSizePropertyPanel->SetSizer(groupSizePropertySizer);

	wxFlexGridSizer * fileExtensionsPropertySizer = new wxFlexGridSizer(2, 3, 3);
	fileExtensionsPropertySizer->Add(fileExtensionsLabel, 0, wxALIGN_CENTER_VERTICAL, border);
	fileExtensionsPropertySizer->Add(m_fileExtensionsText, 0, wxALIGN_CENTER_VERTICAL, border);
	fileExtensionsPropertyPanel->SetSizer(fileExtensionsPropertySizer);

	m_fileInfoBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	m_fileInfoBoxSizer->Add(m_fileInfoPanel, 1, wxEXPAND | wxALL, 20);
	m_fileInfoBox->SetSizer(m_fileInfoBoxSizer);

	wxFlexGridSizer * fileInfoPanelSizer = new wxFlexGridSizer(2, border, border);
	fileInfoPanelSizer->AddGrowableCol(0, 0);
	fileInfoPanelSizer->AddGrowableCol(1, 1);
	fileInfoPanelSizer->Add(m_fileNameLabel, 1, wxEXPAND | wxHORIZONTAL);
	fileInfoPanelSizer->Add(m_fileNameText, 1, wxEXPAND | wxHORIZONTAL);
	fileInfoPanelSizer->Add(m_numberOfFilesSelectedLabel, 1, wxEXPAND | wxHORIZONTAL);
	fileInfoPanelSizer->Add(m_numberOfFilesSelectedText, 1, wxEXPAND | wxHORIZONTAL);
	fileInfoPanelSizer->Add(m_fileSizeLabel, 1, wxEXPAND | wxHORIZONTAL);
	fileInfoPanelSizer->Add(m_fileSizeText, 1, wxEXPAND | wxHORIZONTAL);
	m_fileInfoPanel->SetSizer(fileInfoPanelSizer);

	m_groupPropertiesSizer = new wxFlexGridSizer(4, border * 2, border * 2);
	m_groupPropertiesSizer->Add(numberOfFilesPropertyPanel, 0, 0, border);
	m_groupPropertiesSizer->Add(groupSizePropertyPanel, 0, 0, border);
	m_groupPropertiesSizer->Add(fileExtensionsPropertyPanel, 0, 0, border);
	groupPropertiesPanel->SetSizer(m_groupPropertiesSizer);

	wxGridBagSizer * groupInfoSizer = new wxGridBagSizer(border, border);
	groupInfoSizer->Add(groupPropertiesPanel, wxGBPosition(0, 0), wxGBSpan(1, 2), wxEXPAND | wxHORIZONTAL, border);
	groupInfoSizer->Add(m_fileListBox, wxGBPosition(1, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	groupInfoSizer->Add(m_fileInfoBox, wxGBPosition(1, 1), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	groupInfoSizer->AddGrowableRow(1, 1);
	groupInfoSizer->AddGrowableCol(1, 1);
	SetSizer(groupInfoSizer);

	m_groupModifiedConnection = m_group->modified.connect(std::bind(&GroupPanel::onGroupModified, this, std::placeholders::_1));

	update();
}

GroupPanel::~GroupPanel() {
	m_groupModifiedConnection.disconnect();
}

std::string GroupPanel::getPanelName() const {
	std::string_view fileName(m_group->getFileName());

	return fileName.empty() ? "NEW GROUP *" : fmt::format("{}{}", fileName, m_group->isModified() ? " *" : "");
}

const Group * GroupPanel::getGroup() const {
	return m_group.get();
}

Group * GroupPanel::getGroup() {
	return m_group.get();
}

size_t GroupPanel::numberOfFilesSelected() const {
	wxArrayInt selections;

	m_fileListBox->GetSelections(selections);

	return selections.GetCount();
}

std::vector<std::shared_ptr<GroupFile>> GroupPanel::getSelectedFiles() const {
	wxArrayInt selections;

	m_fileListBox->GetSelections(selections);

	std::vector<std::shared_ptr<GroupFile>> selectedFiles;

	for(size_t i = 0; i < selections.GetCount(); i++) {
		selectedFiles.push_back(m_group->getFile(selections[i]));
	}

	return selectedFiles;
}

size_t GroupPanel::getTotalSizeOfSelectedFiles() const {
	size_t totalSize = 0;
	std::vector<std::shared_ptr<GroupFile>> selectedFiles(getSelectedFiles());

	for(const std::shared_ptr<GroupFile> & file : selectedFiles) {
		totalSize += file->getSize();
	}

	return totalSize;
}

std::string GroupPanel::getTotalSizeOfSelectedFilesAsString() const {
	size_t totalSize = getTotalSizeOfSelectedFiles();

	if(totalSize < 1000) {
		return fmt::format("{} B", totalSize);
	}
	else if(totalSize < 1000000) {
		return fmt::format("{:.2f} KB", totalSize / 1000.0);
	}
	else {
		return fmt::format("{:.2f} MB", totalSize / 1000000.0);
	}
}

size_t GroupPanel::extractSelectedFiles(const std::string & directoryPath) const {
	size_t extractedFileCount = 0;
	std::vector<std::shared_ptr<GroupFile>> selectedFiles(getSelectedFiles());

	for(const std::shared_ptr<GroupFile> & file : selectedFiles) {
		if(file->writeTo(directoryPath)) {
			extractedFileCount++;
		}
	}

	return extractedFileCount;
}

void GroupPanel::update() {
	m_numberOfFilesText->SetLabel(std::to_string(m_group->numberOfFiles()));
	m_groupSizeText->SetLabel(m_group->getGroupSizeAsString());

	std::string fileExtensions(m_group->getFileExtensionsAsString());

	m_fileExtensionsText->SetLabel(fileExtensions.empty() ? "None" : fileExtensions);

	wxArrayString fileNames;
	fileNames.Alloc(m_group->numberOfFiles());
	const std::vector<std::shared_ptr<GroupFile>> & groupFiles = m_group->getFiles();

	for(size_t i = 0; i < groupFiles.size(); i++) {
		fileNames.Add(fmt::format("{}. {}", i + 1, groupFiles[i]->getFileName()));
	}

	m_fileListBox->Set(fileNames);

	updateFileInfo();

	m_groupPropertiesSizer->Layout();
}

void GroupPanel::updateFileInfo() {
	wxArrayInt selections;

	m_fileListBox->GetSelections(selections);

	if(selections.GetCount() == 0) {
		m_fileInfoPanel->Hide();
		return;
	}
	else if(selections.GetCount() == 1) {
		int selectedFileIndex = selections[0];

		std::shared_ptr<GroupFile> selectedFile(m_group->getFile(selectedFileIndex));

		m_fileNameText->SetLabel(selectedFile->getFileName());
		m_fileSizeLabel->SetLabel("File Size:");
		m_fileSizeText->SetLabel(selectedFile->getSizeAsString());

		m_fileNameLabel->Show();
		m_fileNameText->Show();
		m_numberOfFilesSelectedLabel->Hide();
		m_numberOfFilesSelectedText->Hide();
	}
	else {
		m_fileSizeLabel->SetLabel("Total Size of Selected Files:");
		m_fileSizeText->SetLabel(getTotalSizeOfSelectedFilesAsString());
		m_numberOfFilesSelectedText->SetLabel(std::to_string(selections.GetCount()));

		m_fileNameLabel->Hide();
		m_fileNameText->Hide();
		m_numberOfFilesSelectedLabel->Show();
		m_numberOfFilesSelectedText->Show();
	}

	m_fileInfoPanel->Show();

	m_fileInfoBoxSizer->Layout();
}

void GroupPanel::onFileSelected(wxCommandEvent & event) {
	updateFileInfo();
	notifyGroupFileSelectionChanged();
}

GroupPanel::Listener::~Listener() { }

size_t GroupPanel::numberOfListeners() const {
	return m_listeners.size();
}

bool GroupPanel::hasListener(const Listener & listener) const {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			return true;
		}
	}

	return false;
}

size_t GroupPanel::indexOfListener(const Listener & listener) const {
	for(size_t i = 0; i < m_listeners.size(); i++) {
		if(m_listeners[i] == &listener) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

GroupPanel::Listener * GroupPanel::getListener(size_t index) const {
	if(index >= m_listeners.size()) {
		return nullptr;
	}

	return m_listeners[index];
}

bool GroupPanel::addListener(Listener & listener) {
	if(!hasListener(listener)) {
		m_listeners.push_back(&listener);

		return true;
	}

	return false;
}

bool GroupPanel::removeListener(size_t index) {
	if(index >= m_listeners.size()) {
		return false;
	}

	m_listeners.erase(m_listeners.cbegin() + index);

	return true;
}

bool GroupPanel::removeListener(const Listener & listener) {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			m_listeners.erase(i);

			return true;
		}
	}

	return false;
}

void GroupPanel::clearListeners() {
	m_listeners.clear();
}

void GroupPanel::notifyGroupFileSelectionChanged() {
	for(Listener * listener : m_listeners) {
		listener->groupFileSelectionChanged(this);
	}
}

void GroupPanel::onGroupModified(const Group & group) {
	update();

	for(Listener * listener : m_listeners) {
		listener->groupModified(this, group.isModified());
	}
}
