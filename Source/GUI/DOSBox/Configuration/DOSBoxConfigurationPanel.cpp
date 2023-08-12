#include "DOSBoxConfigurationPanel.h"

#include "../../WXUtilities.h"

#include <wx/gbsizer.h>
#include <wx/propgrid/propgrid.h>
#include <wx/wrapsizer.h>

#include <fmt/core.h>

DOSBoxConfigurationPanel::DOSBoxConfigurationPanel(std::shared_ptr<DOSBoxConfiguration> dosboxConfiguration, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "DOSBox Configuration")
	, m_dosboxConfiguration(dosboxConfiguration)
	, m_lastSavedDOSBoxConfiguration(std::make_shared<DOSBoxConfiguration>(*dosboxConfiguration))
	, m_enabled(true)
	, m_sectionListBox(nullptr)
	, m_configurationSectionPropertyGrid(nullptr)
	, m_addSectionButton(nullptr)
	, m_renameSectionButton(nullptr)
	, m_removeSectionButton(nullptr)
	, m_addEntryButton(nullptr)
	, m_editEntryButton(nullptr)
	, m_renameEntryButton(nullptr)
	, m_removeEntryButton(nullptr)
	, m_discardConfigurationButton(nullptr)
	, m_saveConfigurationButton(nullptr) {
	if(m_dosboxConfiguration != nullptr) {
		m_dosboxConfigurationConnections = SignalConnectionGroup(
			m_dosboxConfiguration->configurationModified.connect(std::bind(&DOSBoxConfigurationPanel::onConfigurationModified, this, std::placeholders::_1)),
			m_dosboxConfiguration->configurationSectionNameChanged.connect(std::bind(&DOSBoxConfigurationPanel::onConfigurationSectionNameChanged, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)),
			m_dosboxConfiguration->configurationSectionAdded.connect(std::bind(&DOSBoxConfigurationPanel::onConfigurationSectionAdded, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
			m_dosboxConfiguration->configurationSectionReplaced.connect(std::bind(&DOSBoxConfigurationPanel::onConfigurationSectionReplaced, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)),
			m_dosboxConfiguration->configurationSectionInserted.connect(std::bind(&DOSBoxConfigurationPanel::onConfigurationSectionInserted, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
			m_dosboxConfiguration->configurationSectionRemoved.connect(std::bind(&DOSBoxConfigurationPanel::onConfigurationSectionRemoved, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
			m_dosboxConfiguration->configurationSectionsCleared.connect(std::bind(&DOSBoxConfigurationPanel::onConfigurationSectionsCleared, this, std::placeholders::_1)),
			m_dosboxConfiguration->configurationSectionEntryNameChanged.connect(std::bind(&DOSBoxConfigurationPanel::onConfigurationSectionEntryNameChanged, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6)),
			m_dosboxConfiguration->configurationSectionEntryValueChanged.connect(std::bind(&DOSBoxConfigurationPanel::onConfigurationSectionEntryValueChanged, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6)),
			m_dosboxConfiguration->configurationSectionEntryAdded.connect(std::bind(&DOSBoxConfigurationPanel::onConfigurationSectionEntryAdded, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)),
			m_dosboxConfiguration->configurationSectionEntryReplaced.connect(std::bind(&DOSBoxConfigurationPanel::onConfigurationSectionEntryReplaced, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6)),
			m_dosboxConfiguration->configurationSectionEntryInserted.connect(std::bind(&DOSBoxConfigurationPanel::onConfigurationSectionEntryInserted, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)),
			m_dosboxConfiguration->configurationSectionEntryRemoved.connect(std::bind(&DOSBoxConfigurationPanel::onConfigurationSectionEntryRemoved, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)),
			m_dosboxConfiguration->configurationSectionEntriesCleared.connect(std::bind(&DOSBoxConfigurationPanel::onConfigurationSectionEntriesCleared, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
		);
	}

	wxPanel * sectionsPanel = new wxPanel(this);

	wxStaticText * sectionLabel = new wxStaticText(sectionsPanel, wxID_ANY, "Sections", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sectionLabel->SetFont(sectionLabel->GetFont().MakeBold());
	m_sectionListBox = new wxListBox(sectionsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_dosboxConfiguration != nullptr ? WXUtilities::createItemWXArrayString(m_dosboxConfiguration->getOrderedSectionNames()) : wxArrayString(), wxLB_SINGLE | wxLB_ALWAYS_SB);
	m_sectionListBox->SetMaxSize(wxSize(m_sectionListBox->GetMaxSize().x, 30));
	m_sectionListBox->Bind(wxEVT_LISTBOX, &DOSBoxConfigurationPanel::onSectionSelected, this);

	wxPanel * entriesPanel = new wxPanel(this);

	wxStaticText * sectionEntriesLabel = new wxStaticText(entriesPanel, wxID_ANY, "Section Settings", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sectionEntriesLabel->SetFont(sectionEntriesLabel->GetFont().MakeBold());
	m_configurationSectionPropertyGrid = new wxPropertyGrid(entriesPanel);
	m_configurationSectionPropertyGrid->SetMaxSize(wxSize(m_configurationSectionPropertyGrid->GetMaxSize().x, 30));
	m_configurationSectionPropertyGrid->Bind(wxEVT_PG_CHANGED, &DOSBoxConfigurationPanel::onConfigurationSectionPropertyGridChanged, this);

	wxPanel * actionsPanel = new wxPanel(this);

	m_addSectionButton = new wxButton(actionsPanel, wxID_ANY, "Add Section", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Add Section");
	m_addSectionButton->Bind(wxEVT_BUTTON, &DOSBoxConfigurationPanel::onAddSectionButtonPressed, this);

	m_renameSectionButton = new wxButton(actionsPanel, wxID_ANY, "Rename Section", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Rename Section");
	m_renameSectionButton->Bind(wxEVT_BUTTON, &DOSBoxConfigurationPanel::onRenameSectionButtonPressed, this);

	m_removeSectionButton = new wxButton(actionsPanel, wxID_ANY, "Remove Section", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Remove Section");
	m_removeSectionButton->Bind(wxEVT_BUTTON, &DOSBoxConfigurationPanel::onRemoveSectionButtonPressed, this);

	m_addEntryButton = new wxButton(actionsPanel, wxID_ANY, "Add Entry", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Add Entry");
	m_addEntryButton->Bind(wxEVT_BUTTON, &DOSBoxConfigurationPanel::onAddEntryButtonPressed, this);

	m_editEntryButton = new wxButton(actionsPanel, wxID_ANY, "Edit Entry Value", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Edit Entry Value");
	m_editEntryButton->Bind(wxEVT_BUTTON, &DOSBoxConfigurationPanel::onEditEntryButtonPressed, this);

	m_renameEntryButton = new wxButton(actionsPanel, wxID_ANY, "Rename Entry", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Rename Entry");
	m_renameEntryButton->Bind(wxEVT_BUTTON, &DOSBoxConfigurationPanel::onRenameEntryButtonPressed, this);

	m_removeEntryButton = new wxButton(actionsPanel, wxID_ANY, "Remove Entry", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Remove Entry");
	m_removeEntryButton->Bind(wxEVT_BUTTON, &DOSBoxConfigurationPanel::onRemoveEntryButtonPressed, this);

	m_discardConfigurationButton = new wxButton(actionsPanel, wxID_ANY, "Discard Configuration Changes", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Discard Configuration Changes");
	m_discardConfigurationButton->Bind(wxEVT_BUTTON, &DOSBoxConfigurationPanel::onDiscardConfigurationButtonPressed, this);

	m_saveConfigurationButton = new wxButton(actionsPanel, wxID_ANY, "Save Configuration", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Save Configuration");
	m_saveConfigurationButton->Bind(wxEVT_BUTTON, &DOSBoxConfigurationPanel::onSaveConfigurationButtonPressed, this);

	m_actionButtons = {
		m_addSectionButton,
		m_renameSectionButton,
		m_removeSectionButton,
		m_addEntryButton,
		m_editEntryButton,
		m_renameEntryButton,
		m_removeEntryButton,
		m_discardConfigurationButton,
		m_saveConfigurationButton
	};

	int border = 5;

	wxGridBagSizer * sectionsSizer = new wxGridBagSizer(2, 2);
	sectionsSizer->Add(sectionLabel, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL, 0);
	sectionsSizer->Add(m_sectionListBox, wxGBPosition(1, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, 0);
	sectionsSizer->AddGrowableRow(0, 0);
	sectionsSizer->AddGrowableRow(1, 1);
	sectionsSizer->AddGrowableCol(0, 1);
	sectionsPanel->SetSizerAndFit(sectionsSizer);

	wxGridBagSizer * entriesSizer = new wxGridBagSizer(2, 2);
	entriesSizer->Add(sectionEntriesLabel, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL, 0);
	entriesSizer->Add(m_configurationSectionPropertyGrid, wxGBPosition(1, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, 0);
	entriesSizer->AddGrowableRow(0, 0);
	entriesSizer->AddGrowableRow(1, 1);
	entriesSizer->AddGrowableCol(0, 1);
	entriesPanel->SetSizerAndFit(entriesSizer);

	wxWrapSizer * actionsSizer = new wxWrapSizer(wxHORIZONTAL);
	actionsSizer->Add(m_addSectionButton, 1, wxEXPAND | wxALL, 0);
	actionsSizer->Add(m_renameSectionButton, 1, wxEXPAND | wxALL, 0);
	actionsSizer->Add(m_removeSectionButton, 1, wxEXPAND | wxALL, 0);
	actionsSizer->Add(m_addEntryButton, 1, wxEXPAND | wxALL, 0);
	actionsSizer->Add(m_editEntryButton, 1, wxEXPAND | wxALL, 0);
	actionsSizer->Add(m_renameEntryButton, 1, wxEXPAND | wxALL, 0);
	actionsSizer->Add(m_removeEntryButton, 1, wxEXPAND | wxALL, 0);
	actionsSizer->Add(m_discardConfigurationButton, 1, wxEXPAND | wxALL, 0);
	actionsSizer->Add(m_saveConfigurationButton, 1, wxEXPAND | wxALL, 0);
	actionsPanel->SetSizerAndFit(actionsSizer);

	wxGridBagSizer * configurationSizer = new wxGridBagSizer(border * 2, border * 2);
	configurationSizer->Add(sectionsPanel, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, 0);
	configurationSizer->Add(entriesPanel, wxGBPosition(0, 1), wxGBSpan(1, 1), wxEXPAND | wxALL, 0);
	configurationSizer->Add(actionsPanel, wxGBPosition(1, 0), wxGBSpan(1, 2), wxEXPAND | wxHORIZONTAL, 0);
	configurationSizer->AddGrowableRow(0, 1);
	configurationSizer->AddGrowableRow(1, 0);
	configurationSizer->AddGrowableCol(0, 1);
	configurationSizer->AddGrowableCol(1, 1);
	SetSizerAndFit(configurationSizer);

	updateButtons();

	if(m_dosboxConfiguration != nullptr && m_dosboxConfiguration->numberOfSections() != 0) {
		m_sectionListBox->SetSelection(0);
		selectSection(0);
	}
}

DOSBoxConfigurationPanel::~DOSBoxConfigurationPanel() {
	m_dosboxConfigurationConnections.disconnect();
}

bool DOSBoxConfigurationPanel::isEnabled() const {
	return m_enabled;
}

void DOSBoxConfigurationPanel::enable() {
	setEnabled(true);
}

void DOSBoxConfigurationPanel::disable() {
	setEnabled(false);
}

void DOSBoxConfigurationPanel::setEnabled(bool enabled) {
	m_enabled = enabled;

	update();
}

bool DOSBoxConfigurationPanel::isModified() const {
	if(m_dosboxConfiguration == nullptr) {
		return false;
	}

	return m_dosboxConfiguration->isModified();
}

std::string DOSBoxConfigurationPanel::getPanelName() const {
	if(m_dosboxConfiguration == nullptr) {
		return {};
	}

	if(!m_dosboxConfiguration->hasFilePath()) {
		return "NEW DOSBOX CONFIG *";
	}

	return fmt::format("{}{}", m_dosboxConfiguration->getFileName(), isModified() ? " *" : "");
}

std::shared_ptr<DOSBoxConfiguration> DOSBoxConfigurationPanel::getDOSBoxConfiguration() const {
	return m_dosboxConfiguration;
}

bool DOSBoxConfigurationPanel::isValid() const {
	return DOSBoxConfiguration::isValid(m_dosboxConfiguration.get(), true);
}

bool DOSBoxConfigurationPanel::save() {
	if(!m_enabled || !isValid() || !m_dosboxConfiguration->hasFilePath()) {
		return false;
	}

	if(!m_dosboxConfiguration->save()) {
		wxMessageBox(fmt::format("Failed to save DOSBox configuration to file: '{}'.", m_dosboxConfiguration->getFilePath()), "DOSBox Config Writing Failed", wxOK | wxICON_ERROR, this);
		return false;
	}

	m_lastSavedDOSBoxConfiguration = std::make_shared<DOSBoxConfiguration>(*m_dosboxConfiguration);

	spdlog::info("Successfully saved DOSBox configuration to file: '{}'.", m_dosboxConfiguration->getFilePath());

	return true;
}

void DOSBoxConfigurationPanel::discard() {
	if(m_dosboxConfiguration == nullptr || !m_enabled) {
		return;
	}

	if(!m_dosboxConfiguration->setConfiguration(*m_lastSavedDOSBoxConfiguration)) {
		spdlog::error("Failed to reset DOSBox configuration to last saved version.");
		return;
	}
}

bool DOSBoxConfigurationPanel::selectSection(size_t sectionIndex) {
	if(m_dosboxConfiguration == nullptr) {
		return false;
	}

	std::shared_ptr<DOSBoxConfiguration::Section> section(m_dosboxConfiguration->getSection(sectionIndex));

	if(section == nullptr) {
		spdlog::error("Invalid selected DOSBox configuration section index: '{}'.", sectionIndex);
		return false;
	}

	return selectSection(section);
}

bool DOSBoxConfigurationPanel::selectSection(std::shared_ptr<DOSBoxConfiguration::Section> section) {
	if(m_dosboxConfiguration == nullptr) {
		return false;
	}

	if(section == nullptr || section->getParentConfiguration() != m_dosboxConfiguration.get()) {
		spdlog::error("Invalid selected DOSBox configuration section.");
		return false;
	}

	m_selectedSection = section;
	m_configurationSectionPropertyGrid->Clear();
	std::shared_ptr<DOSBoxConfiguration::Section::Entry> entry;

	for(const std::string & entryName : section->getOrderedEntryNames()) {
		entry = section->getEntryWithName(entryName);

		m_configurationSectionPropertyGrid->Append(new wxStringProperty(entry->getName(), entry->getName(), entry->getValue()));
	}

	m_configurationSectionPropertyGrid->FitColumns();

	updateButtons();

	return true;
}

void DOSBoxConfigurationPanel::clearSelectedSection() {
	m_configurationSectionPropertyGrid->Clear();
	m_sectionListBox->SetSelection(wxNOT_FOUND);
	m_selectedSection.reset();

	updateButtons();
}

void DOSBoxConfigurationPanel::update() {
	updateButtons();
}

void DOSBoxConfigurationPanel::updateButtons() {
	if(m_dosboxConfiguration == nullptr) {
		for(wxButton * actionButton : m_actionButtons) {
			actionButton->Disable();
		}

		return;
	}

	bool hasAnyEntries = m_selectedSection != nullptr && m_selectedSection->numberOfEntries() != 0;
	bool isModified = m_dosboxConfiguration->isModified();
	bool hasFilePath = m_dosboxConfiguration->hasFilePath();

	WXUtilities::setButtonEnabled(m_addSectionButton, m_enabled);
	WXUtilities::setButtonEnabled(m_renameSectionButton, m_enabled && m_selectedSection != nullptr);
	WXUtilities::setButtonEnabled(m_removeSectionButton, m_enabled && m_selectedSection != nullptr);
	WXUtilities::setButtonEnabled(m_addEntryButton, m_enabled && m_selectedSection != nullptr);
	WXUtilities::setButtonEnabled(m_editEntryButton, m_enabled && m_selectedSection != nullptr);
	WXUtilities::setButtonEnabled(m_renameEntryButton, m_enabled && hasAnyEntries);
	WXUtilities::setButtonEnabled(m_removeEntryButton, m_enabled && hasAnyEntries);
	WXUtilities::setButtonEnabled(m_discardConfigurationButton, m_enabled && isModified);
	WXUtilities::setButtonEnabled(m_saveConfigurationButton, m_enabled && isModified && hasFilePath);
}

void DOSBoxConfigurationPanel::onSectionSelected(wxCommandEvent & event) {
	if(!isValid()) {
		return;
	}

	int selectedSectionIndex = m_sectionListBox->GetSelection();

	if(selectedSectionIndex == wxNOT_FOUND) {
		return;
	}

	selectSection(selectedSectionIndex);
}

void DOSBoxConfigurationPanel::onConfigurationSectionPropertyGridChanged(wxPropertyGridEvent & event) {
	if(m_selectedSection == nullptr || !m_enabled) {
		spdlog::error("Tried to update DOSBox configuration section entry value with no section selected.");
		return;
	}

	const wxPGProperty * property = event.GetProperty();

	if(property == nullptr) {
		return;
	}

	wxAny value = property->GetValue();

	if(value.IsNull()) {
		return;
	}

	std::string entryName(property->GetName());
	std::string entryValue(value.As<wxString>());

	if(!m_selectedSection->setEntryValue(entryName, entryValue)) {
		spdlog::error("Failed to set DOSBox configuration '{}' section '{}' entry value to '{}'.", m_selectedSection->getName(), entryName, entryValue);
		return;
	}

	spdlog::debug("Updated DOSBox configuration '{}' section '{}' entry value to '{}'.", m_selectedSection->getName(), entryName, entryValue);
}

void DOSBoxConfigurationPanel::onAddSectionButtonPressed(wxCommandEvent & event) {
	if(m_dosboxConfiguration == nullptr || !m_enabled) {
		return;
	}

	wxTextEntryDialog newSectionNameDialog(this, "Enter a name for the new section:", "Add Section", wxEmptyString, wxOK | wxCANCEL, wxDefaultPosition);
	int addSectionResult = newSectionNameDialog.ShowModal();

	if(addSectionResult == wxID_CANCEL) {
		return;
	}

	std::string newSectionName(newSectionNameDialog.GetValue());

	if(!DOSBoxConfiguration::Section::isNameValid(newSectionName)) {
		wxMessageBox(fmt::format("Failed to create DOSBox configuration section with invalid name: '{}'!", newSectionName), "Invalid Section Name", wxOK | wxICON_ERROR, this);
		return;
	}

	if(m_dosboxConfiguration->hasSectionWithName(newSectionName)) {
		wxMessageBox(fmt::format("DOSBox configuration already has section with name: '{}'.", newSectionName), "Section Name Already Exists", wxOK | wxICON_ERROR, this);
		return;
	}

	if(!m_dosboxConfiguration->addSection(newSectionName)) {
		wxMessageBox(fmt::format("Failed to add new DOSBox configuration section with name: '{}'!", newSectionName), "Section Creation Failed", wxOK | wxICON_ERROR, this);
		return;
	}

	spdlog::debug("Created new DOSBox configuration section with name: '{}'.", newSectionName);
}

void DOSBoxConfigurationPanel::onRenameSectionButtonPressed(wxCommandEvent & event) {
	if(m_dosboxConfiguration == nullptr || m_selectedSection == nullptr || !m_enabled) {
		return;
	}

	wxTextEntryDialog newSectionNameDialog(this, "Enter a new name for the section:", "Rename Section", m_selectedSection->getName(), wxOK | wxCANCEL, wxDefaultPosition);
	int renameSectionResult = newSectionNameDialog.ShowModal();

	if(renameSectionResult == wxID_CANCEL) {
		return;
	}

	std::string newSectionName(newSectionNameDialog.GetValue());

	if(!DOSBoxConfiguration::Section::isNameValid(newSectionName)) {
		wxMessageBox(fmt::format("Failed to rename DOSBox configuration '{}' section to invalid name: '{}'!", m_selectedSection->getName(), newSectionName), "Invalid Section Name", wxOK | wxICON_ERROR, this);
		return;
	}

	if(m_dosboxConfiguration->hasSectionWithName(newSectionName)) {
		wxMessageBox(fmt::format("DOSBox configuration already has section with name: '{}'.", newSectionName), "Section Name Already Exists", wxOK | wxICON_ERROR, this);
		return;
	}

	std::string previousSectionName(m_selectedSection->getName());

	if(!m_selectedSection->setName(newSectionName)) {
		wxMessageBox(fmt::format("Failed to rename DOSBox configuration '{}' section to '{}'.", previousSectionName, newSectionName), "Section Rename Failed", wxOK | wxICON_ERROR, this);
		return;
	}

	spdlog::debug("Renamed DOSBox configuration '{}' section  to '{}'.", previousSectionName, newSectionName);
}

void DOSBoxConfigurationPanel::onRemoveSectionButtonPressed(wxCommandEvent & event) {
	if(m_dosboxConfiguration == nullptr || m_selectedSection == nullptr || !m_enabled) {
		return;
	}

	int removeSectionConfirmationResult = wxMessageBox(fmt::format("Are you sure you would like to remove the '{}' section from this DOSBox configuration?", m_selectedSection->getName()), "Remove Section", wxYES_NO | wxCANCEL | wxICON_WARNING, this);

	if(removeSectionConfirmationResult != wxYES) {
		return;
	}

	size_t sectionIndex = m_dosboxConfiguration->indexOfSectionWithName(m_selectedSection->getName());

	if(!m_selectedSection->remove()) {
		wxMessageBox(fmt::format("Failed to remove DOSBox configuration section '{}'.", m_selectedSection->getName()), "Section Removal Failed", wxOK | wxICON_ERROR, this);
		return;
	}
}

void DOSBoxConfigurationPanel::onAddEntryButtonPressed(wxCommandEvent & event) {
	if(m_dosboxConfiguration == nullptr || m_selectedSection == nullptr || !m_enabled) {
		return;
	}

	wxTextEntryDialog newEntryNameDialog(this, "Enter a name for the new entry:", "New Entry Name", wxEmptyString, wxOK | wxCANCEL, wxDefaultPosition);
	int newEntryNameResult = newEntryNameDialog.ShowModal();

	if(newEntryNameResult == wxID_CANCEL) {
		return;
	}

	std::string newEntryName(newEntryNameDialog.GetValue());

	if(!DOSBoxConfiguration::Section::Entry::isNameValid(newEntryName)) {
		wxMessageBox(fmt::format("Failed to create DOSBox configuration section entry with invalid name: '{}'!", newEntryName), "Invalid Entry Name", wxOK | wxICON_ERROR, this);
		return;
	}

	if(m_selectedSection->hasEntryWithName(newEntryName)) {
		wxMessageBox(fmt::format("DOSBox configuration section already has entry with name: '{}'.", newEntryName), "Entry Name Already Exists", wxOK | wxICON_ERROR, this);
		return;
	}

	wxTextEntryDialog newEntryValueDialog(this, "Enter a value for the new entry:", "New Entry Value", wxEmptyString, wxOK | wxCANCEL, wxDefaultPosition);
	int newEntryValueResult = newEntryValueDialog.ShowModal();

	if(newEntryValueResult == wxID_CANCEL) {
		return;
	}

	std::string newEntryValue(newEntryValueDialog.GetValue());

	if(!m_selectedSection->addEntry(newEntryName, newEntryValue)) {
		wxMessageBox(fmt::format("Failed to add new DOSBox configuration section entry with name: '{}'!", newEntryName), "Entry Creation Failed", wxOK | wxICON_ERROR, this);
		return;
	}

	spdlog::debug("Created new DOSBox configuration section entry with name: '{}' and value '{}'.", newEntryName, newEntryValue);
}

void DOSBoxConfigurationPanel::onEditEntryButtonPressed(wxCommandEvent & event) {
	if(m_dosboxConfiguration == nullptr || m_selectedSection == nullptr || m_selectedSection->numberOfEntries() == 0 || !m_enabled) {
		return;
	}

	int entryIndex = wxGetSingleChoiceIndex("Please choose an entry to edit:", "Edit Entry", WXUtilities::createItemWXArrayString(m_selectedSection->getOrderedEntryNames()), 0, this);

	if(entryIndex == wxNOT_FOUND) {
		return;
	}

	std::shared_ptr<DOSBoxConfiguration::Section::Entry> entry(m_selectedSection->getEntry(entryIndex));

	if(entry == nullptr) {
		wxMessageBox("Failed to obtain selected entry.", "Entry Selection Failed", wxOK | wxICON_ERROR, this);
		return;
	}

	wxTextEntryDialog newEntryValueDialog(this, "Enter a new value for the entry:", "New Entry Value", entry->getValue(), wxOK | wxCANCEL, wxDefaultPosition);
	int newEntryValueResult = newEntryValueDialog.ShowModal();

	if(newEntryValueResult == wxID_CANCEL) {
		return;
	}

	std::string previousEntryValue(entry->getValue());
	std::string newEntryValue(newEntryValueDialog.GetValue());

	entry->setValue(newEntryValue);

	spdlog::debug("Changed DOSBox configuration '{}' section '{}' entry value from '{}' to '{}'.", m_selectedSection->getName(), entry->getName(), previousEntryValue, newEntryValue);
}

void DOSBoxConfigurationPanel::onRenameEntryButtonPressed(wxCommandEvent & event) {
	if(m_dosboxConfiguration == nullptr || m_selectedSection == nullptr || !m_enabled) {
		return;
	}

	int entryIndex = wxGetSingleChoiceIndex("Please choose an entry to rename:", "Rename Entry", WXUtilities::createItemWXArrayString(m_selectedSection->getOrderedEntryNames()), 0, this);

	if(entryIndex == wxNOT_FOUND) {
		return;
	}

	std::shared_ptr<DOSBoxConfiguration::Section::Entry> entry(m_selectedSection->getEntry(entryIndex));

	if(entry == nullptr) {
		wxMessageBox("Failed to obtain selected entry.", "Entry Selection Failed", wxOK | wxICON_ERROR, this);
		return;
	}

	wxTextEntryDialog newEntryNameDialog(this, "Enter a new name for the section entry:", "Rename Entry", entry->getName(), wxOK | wxCANCEL, wxDefaultPosition);
	int renameEntryResult = newEntryNameDialog.ShowModal();

	if(renameEntryResult == wxID_CANCEL) {
		return;
	}

	std::string newEntryName(newEntryNameDialog.GetValue());

	if(!DOSBoxConfiguration::Section::Entry::isNameValid(newEntryName)) {
		wxMessageBox(fmt::format("Failed to rename DOSBox configuration '{}' section '{}' entry to invalid name: '{}'!", m_selectedSection->getName(), entry->getName(), newEntryName), "Invalid Entry Name", wxOK | wxICON_ERROR, this);
		return;
	}

	if(m_selectedSection->hasEntryWithName(newEntryName)) {
		wxMessageBox(fmt::format("DOSBox configuration already has '{}' section entry with name: '{}'.", m_selectedSection->getName(), newEntryName), "Entry Name Already Exists", wxOK | wxICON_ERROR, this);
		return;
	}

	std::string previousEntryName(entry->getName());

	if(!entry->setName(newEntryName)) {
		wxMessageBox(fmt::format("Failed to rename DOSBox configuration '{}' section '{}' entry to '{}'.", m_selectedSection->getName(), previousEntryName, newEntryName), "Entry Rename Failed", wxOK | wxICON_ERROR, this);
		return;
	}

	spdlog::debug("Renamed DOSBox configuration '{}' section '{}' entry to '{}'.", m_selectedSection->getName(), previousEntryName, newEntryName);
}

void DOSBoxConfigurationPanel::onRemoveEntryButtonPressed(wxCommandEvent & event) {
	if(m_dosboxConfiguration == nullptr || m_selectedSection == nullptr || !m_enabled) {
		return;
	}

	int entryIndex = wxGetSingleChoiceIndex("Please choose an entry to remove:", "Remove Entry", WXUtilities::createItemWXArrayString(m_selectedSection->getOrderedEntryNames()), 0, this);

	if(entryIndex == wxNOT_FOUND) {
		return;
	}

	std::shared_ptr<DOSBoxConfiguration::Section::Entry> entry(m_selectedSection->getEntry(entryIndex));

	if(entry == nullptr) {
		wxMessageBox("Failed to obtain selected entry.", "Entry Selection Failed", wxOK | wxICON_ERROR, this);
		return;
	}

	if(!entry->remove()) {
		wxMessageBox(fmt::format("Failed to remove DOSBox configuration '{}' section '{}' entry.", m_selectedSection->getName(), entry->getName()), "Entry Removal Failed", wxOK | wxICON_ERROR, this);
		return;
	}

	spdlog::debug("Removed DOSBox configuration '{}' section '{}' entry.", m_selectedSection->getName(), entry->getName());
}

void DOSBoxConfigurationPanel::onDiscardConfigurationButtonPressed(wxCommandEvent & event) {
	discard();
}

void DOSBoxConfigurationPanel::onSaveConfigurationButtonPressed(wxCommandEvent & event) {
	save();
}

void DOSBoxConfigurationPanel::onConfigurationModified(DOSBoxConfiguration & dosboxConfiguration) {
	if(m_dosboxConfiguration.get() != &dosboxConfiguration) {
		return;
	}

	updateButtons();
}

void DOSBoxConfigurationPanel::onConfigurationSectionNameChanged(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> section, size_t sectionIndex, std::string oldSectionName) {
	if(m_dosboxConfiguration.get() != &dosboxConfiguration) {
		return;
	}

	m_sectionListBox->Insert(section->getName(), sectionIndex);
	m_sectionListBox->Delete(sectionIndex + 1);
	m_sectionListBox->SetSelection(sectionIndex);
}

void DOSBoxConfigurationPanel::onConfigurationSectionAdded(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> newSection, size_t sectionIndex) {
	if(m_dosboxConfiguration.get() != &dosboxConfiguration) {
		return;
	}

	m_sectionListBox->Append(newSection->getName());
}

void DOSBoxConfigurationPanel::onConfigurationSectionReplaced(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> newSection, size_t sectionIndex, std::shared_ptr<DOSBoxConfiguration::Section> oldSection) {
	if(m_dosboxConfiguration.get() != &dosboxConfiguration) {
		return;
	}

	m_sectionListBox->Insert(newSection->getName(), sectionIndex);
	m_sectionListBox->Delete(sectionIndex + 1);
	m_sectionListBox->SetSelection(sectionIndex);

	if(m_selectedSection.get() == oldSection.get()) {
		selectSection(newSection);
	}
}

void DOSBoxConfigurationPanel::onConfigurationSectionInserted(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> newSection, size_t sectionIndex) {
	if(m_dosboxConfiguration.get() != &dosboxConfiguration) {
		return;
	}

	size_t selectedSectionIndex = m_sectionListBox->GetSelection();

	m_sectionListBox->Insert(newSection->getName(), sectionIndex);

	if(selectedSectionIndex != wxNOT_FOUND && selectedSectionIndex >= sectionIndex) {
		m_sectionListBox->SetSelection(selectedSectionIndex + 1);
	}
}

void DOSBoxConfigurationPanel::onConfigurationSectionRemoved(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> section, size_t sectionIndex) {
	if(m_dosboxConfiguration.get() != &dosboxConfiguration) {
		return;
	}

	m_sectionListBox->Delete(sectionIndex);

	if(m_selectedSection.get() == section.get()) {
		clearSelectedSection();
	}
}

void DOSBoxConfigurationPanel::onConfigurationSectionsCleared(DOSBoxConfiguration & dosboxConfiguration) {
	if(m_dosboxConfiguration.get() != &dosboxConfiguration) {
		return;
	}

	clearSelectedSection();

	m_sectionListBox->Clear();
}

void DOSBoxConfigurationPanel::onConfigurationSectionEntryNameChanged(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> section, size_t sectionIndex, std::shared_ptr<DOSBoxConfiguration::Section::Entry> entry, size_t entryIndex, std::string oldEntryName) {
	if(m_dosboxConfiguration.get() != &dosboxConfiguration || m_selectedSection.get() != section.get()) {
		return;
	}

	m_configurationSectionPropertyGrid->SetPropertyLabel(oldEntryName.c_str(), entry->getName().c_str());
	m_configurationSectionPropertyGrid->SetPropertyName(oldEntryName.c_str(), entry->getName().c_str());
	m_configurationSectionPropertyGrid->FitColumns();
}

void DOSBoxConfigurationPanel::onConfigurationSectionEntryValueChanged(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> section, size_t sectionIndex, std::shared_ptr<DOSBoxConfiguration::Section::Entry> entry, size_t entryIndex, std::string oldEntryValue) {
	if(m_dosboxConfiguration.get() != &dosboxConfiguration || m_selectedSection.get() != section.get()) {
		return;
	}

	m_configurationSectionPropertyGrid->SetPropertyValue(entry->getName().c_str(), entry->getValue().c_str());
}

void DOSBoxConfigurationPanel::onConfigurationSectionEntryAdded(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> section, size_t sectionIndex, std::shared_ptr<DOSBoxConfiguration::Section::Entry> newEntry, size_t entryIndex) {
	if(m_dosboxConfiguration.get() != &dosboxConfiguration || m_selectedSection.get() != section.get()) {
		return;
	}

	m_configurationSectionPropertyGrid->Append(new wxStringProperty(newEntry->getName(), newEntry->getName(), newEntry->getValue()));
	m_configurationSectionPropertyGrid->FitColumns();
}

void DOSBoxConfigurationPanel::onConfigurationSectionEntryReplaced(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> section, size_t sectionIndex, std::shared_ptr<DOSBoxConfiguration::Section::Entry> newEntry, size_t entryIndex, std::shared_ptr<DOSBoxConfiguration::Section::Entry> oldEntry) {
	if(m_dosboxConfiguration.get() != &dosboxConfiguration || m_selectedSection.get() != section.get()) {
		return;
	}

	m_configurationSectionPropertyGrid->SetPropertyLabel(oldEntry->getName().c_str(), newEntry->getName().c_str());
	m_configurationSectionPropertyGrid->SetPropertyName(oldEntry->getName().c_str(), newEntry->getName().c_str());
	m_configurationSectionPropertyGrid->SetPropertyValue(newEntry->getName().c_str(), newEntry->getValue().c_str());
	m_configurationSectionPropertyGrid->FitColumns();
}

void DOSBoxConfigurationPanel::onConfigurationSectionEntryInserted(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> section, size_t sectionIndex, std::shared_ptr<DOSBoxConfiguration::Section::Entry> newEntry, size_t entryIndex) {
	if(m_dosboxConfiguration.get() != &dosboxConfiguration || m_selectedSection.get() != section.get()) {
		return;
	}

	wxStringProperty * stringProperty = new wxStringProperty(newEntry->getName(), newEntry->getName(), newEntry->getValue());

	if(entryIndex == m_selectedSection->numberOfEntries() - 1) {
		m_configurationSectionPropertyGrid->Append(stringProperty);
	}
	else {
		m_configurationSectionPropertyGrid->Insert(m_selectedSection->getEntry(entryIndex + 1)->getName().c_str(), stringProperty);
	}

	m_configurationSectionPropertyGrid->FitColumns();
}

void DOSBoxConfigurationPanel::onConfigurationSectionEntryRemoved(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> section, size_t sectionIndex, std::shared_ptr<DOSBoxConfiguration::Section::Entry> entry, size_t entryIndex) {
	if(m_dosboxConfiguration.get() != &dosboxConfiguration || m_selectedSection.get() != section.get()) {
		return;
	}

	m_configurationSectionPropertyGrid->DeleteProperty(entry->getName().c_str());
	m_configurationSectionPropertyGrid->FitColumns();
}

void DOSBoxConfigurationPanel::onConfigurationSectionEntriesCleared(DOSBoxConfiguration & dosboxConfiguration, std::shared_ptr<DOSBoxConfiguration::Section> section, size_t sectionIndex) {
	if(m_dosboxConfiguration.get() != &dosboxConfiguration || m_selectedSection.get() != section.get()) {
		return;
	}

	m_configurationSectionPropertyGrid->Clear();
}
