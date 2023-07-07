#include "GroupPanel.h"

#include "Game/File/GameFile.h"
#include "Game/File/GameFileFactoryRegistry.h"
#include "Game/File/Group/Group.h"
#include "Game/File/Group/GroupFile.h"
#include "Game/File/Group/SSI/GroupSSI.h"
#include "MetadataPanel.h"
#include "WXUtilities.h"

#include <Utilities/FileUtilities.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <wx/gbsizer.h>
#include <wx/wrapsizer.h>

GroupPanel::GroupPanel(std::unique_ptr<Group> group, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Group Information")
	, m_group(std::shared_ptr<Group>(group.release()))
	, m_numberOfFilesText(nullptr)
	, m_groupSizeText(nullptr)
	, m_fileExtensionsText(nullptr)
	, m_fileListBox(nullptr)
	, m_fileInfoBox(nullptr)
	, m_fileInfoPanel(nullptr)
	, m_ssiMetadataBox(nullptr)
	, m_ssiMetadataPanel(nullptr)
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

	m_fileInfoPanel = new MetadataPanel(m_fileInfoBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER, "File Info");
	m_fileInfoPanel->Hide();

	std::shared_ptr<GroupSSI> groupSSI(std::dynamic_pointer_cast<GroupSSI>(m_group));

	if(groupSSI != nullptr) {
		m_ssiMetadataBox = new wxStaticBox(this, wxID_ANY, "Sunstorm Interactive File Metadata", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
		m_ssiMetadataBox->SetOwnFont(m_ssiMetadataBox->GetFont().MakeBold());

		m_ssiMetadataPanel = new SunstormInteractiveMetadataPanel(groupSSI, m_ssiMetadataBox);
	}

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

	if(m_ssiMetadataPanel != nullptr) {
		wxBoxSizer * ssiMetadataBoxSizer = new wxBoxSizer(wxHORIZONTAL);
		ssiMetadataBoxSizer->Add(m_ssiMetadataPanel, 1, wxEXPAND | wxALL, 20);
		m_ssiMetadataBox->SetSizer(ssiMetadataBoxSizer);

		groupInfoSizer->Add(m_ssiMetadataBox, wxGBPosition(2, 0), wxGBSpan(1, 2), wxEXPAND | wxHORIZONTAL, border);
		groupInfoSizer->AddGrowableRow(2, 0);
	}

	SetSizer(groupInfoSizer);

	m_groupModifiedConnection = m_group->modified.connect(std::bind(&GroupPanel::onGroupModified, this, std::placeholders::_1));

	update();
}

GroupPanel::~GroupPanel() {
	m_groupModifiedConnection.disconnect();
}

std::string GroupPanel::getPanelName() const {
	std::string_view fileName(m_group->getFileName());

	return fileName.empty() ? fmt::format("NEW {} GROUP *", std::dynamic_pointer_cast<GroupSSI>(m_group) != nullptr ? "SSI" : "GRP") : fmt::format("{}{}", fileName, m_group->isModified() ? " *" : "");
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
	m_groupSizeText->SetLabel(Utilities::fileSizeToString(m_group->getSizeInBytes()));

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

		std::unique_ptr<GameFile> gameFile(GameFileFactoryRegistry::getInstance()->readGameFileFrom(selectedFile->getData(), selectedFile->getFileName()));

		if(gameFile != nullptr) {
			gameFile->setFilePath(selectedFile->getFileName());
			m_fileInfoPanel->setMetadata(gameFile->getMetadata());
		}
		else {
			std::vector<std::pair<std::string, std::string>> metadata;
			metadata.push_back({ "File Name", selectedFile->getFileName() });
			metadata.push_back({ "File Size", Utilities::fileSizeToString(selectedFile->getSize()) });

			m_fileInfoPanel->setMetadata(metadata);
		}
	}
	else {
		std::vector<std::pair<std::string, std::string>> metadata;
		metadata.push_back({ "Number of Files Selected", std::to_string(selections.GetCount()) });
		metadata.push_back({ "Total Size of Selected Files", Utilities::fileSizeToString(getTotalSizeOfSelectedFiles()) });

		m_fileInfoPanel->setMetadata(metadata);
	}

	m_fileInfoPanel->Show();

	m_fileInfoBoxSizer->Layout();
}

void GroupPanel::onFileSelected(wxCommandEvent & event) {
	updateFileInfo();
	groupFileSelectionChanged(*this);
}

void GroupPanel::onGroupModified(const GameFile & group) {
	update();

	groupModified(*this);
}
