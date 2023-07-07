#include "SunstormInteractiveMetadataPanel.h"

#include "WXUtilities.h"

#include <Utilities/StringUtilities.h>

SunstormInteractiveMetadataPanel::SunstormInteractiveMetadataPanel(std::shared_ptr<GroupSSI> group, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style, const std::string & title)
	: wxPanel(parent, windowID, position, size, style, title.empty() ? "Sunstorm Interactive Metadata" : title)
	, m_group(group)
	, m_titleTextField(nullptr)
	, m_versionComboBox(nullptr)
	, m_descriptionTextFields({ nullptr })
	, m_runFileTextField(nullptr) {
	int border = 5;

	wxFlexGridSizer * sizer = new wxFlexGridSizer(2, border, border);
	sizer->AddGrowableCol(0, 0);
	sizer->AddGrowableCol(1, 1);
	SetSizer(sizer);

	wxStaticText * titleLabel = new wxStaticText(this, wxID_ANY, "Title:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	titleLabel->SetFont(titleLabel->GetFont().MakeBold());
	sizer->Add(titleLabel, 1, wxEXPAND | wxALL);

	m_titleTextField = new wxTextCtrl(this, wxID_ANY, m_group != nullptr ? m_group->getTitle() : Utilities::emptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Title");
	m_titleTextField->SetMaxLength(GroupSSI::MAX_TITLE_LENGTH);
	m_titleTextField->Bind(wxEVT_TEXT, &SunstormInteractiveMetadataPanel::onTitleTextChanged, this);
	sizer->Add(m_titleTextField, 1, wxEXPAND | wxALL);

	wxStaticText * versionLabel = new wxStaticText(this, wxID_ANY, "Version:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	versionLabel->SetFont(versionLabel->GetFont().MakeBold());
	sizer->Add(versionLabel, 1, wxEXPAND | wxALL);

	m_versionComboBox = new wxComboBox(this, wxID_ANY, std::to_string(m_group->getVersion()), wxDefaultPosition, wxDefaultSize, WXUtilities::createItemWXArrayString({ "1", "2" }), 0, wxDefaultValidator, "Versions");
	m_versionComboBox->SetEditable(false);
	m_versionComboBox->Bind(wxEVT_COMBOBOX, &SunstormInteractiveMetadataPanel::onVersionSelected, this);
	sizer->Add(m_versionComboBox, 1);

	wxStaticText * descriptionLabel = new wxStaticText(this, wxID_ANY, "Description:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	descriptionLabel->SetFont(descriptionLabel->GetFont().MakeBold());
	sizer->Add(descriptionLabel, 1, wxEXPAND | wxALL);

	for(size_t i = 0; i < m_descriptionTextFields.size(); i++) {
		if(i != 0) {
			sizer->Add(new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER), 1, wxEXPAND | wxHORIZONTAL)->GetWindow();
		}

		wxTextCtrl * descriptionTextField = new wxTextCtrl(this, wxID_ANY, m_group != nullptr ? m_group->getDescription(i) : Utilities::emptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Description");
		descriptionTextField->SetMaxLength(GroupSSI::MAX_DESCRIPTION_LENGTH);
		descriptionTextField->Bind(wxEVT_TEXT, &SunstormInteractiveMetadataPanel::onDescriptionTextChanged, this);
		m_descriptionTextFields[i] = descriptionTextField;
		sizer->Add(descriptionTextField, 1, wxEXPAND | wxALL);
	}

	wxStaticText * runFileLabel = new wxStaticText(this, wxID_ANY, "Run File:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	runFileLabel->SetFont(runFileLabel->GetFont().MakeBold());
	sizer->Add(runFileLabel, 1, wxEXPAND | wxALL);

	m_runFileTextField = new wxTextCtrl(this, wxID_ANY, m_group != nullptr ? m_group->getRunFile() : Utilities::emptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Run File");
	m_runFileTextField->SetMaxLength(GroupSSI::MAX_RUN_FILE_LENGTH);
	m_runFileTextField->Bind(wxEVT_TEXT, &SunstormInteractiveMetadataPanel::onRunFileTextChanged, this);
	sizer->Add(m_runFileTextField, 1, wxEXPAND | wxALL);

	if(!m_group->versionSupportsRunFile()) {
		m_runFileTextField->Disable();
	}
}

SunstormInteractiveMetadataPanel::~SunstormInteractiveMetadataPanel() { }

void SunstormInteractiveMetadataPanel::onTitleTextChanged(wxCommandEvent & event) {
	if(m_group == nullptr) {
		return;
	}

	m_group->setTitle(event.GetString());
}

void SunstormInteractiveMetadataPanel::onVersionSelected(wxCommandEvent & event) {
	int versionIndex = m_versionComboBox->GetSelection();

	if(versionIndex == wxNOT_FOUND) {
		return;
	}

	m_group->setVersion(versionIndex + 1);

	if(m_group->versionSupportsRunFile()) {
		m_runFileTextField->Enable();
	}
	else {
		m_runFileTextField->Disable();
	}
}

void SunstormInteractiveMetadataPanel::onDescriptionTextChanged(wxCommandEvent & event) {
	if(m_group == nullptr) {
		return;
	}

	for(size_t i = 0; i < m_descriptionTextFields.size(); i++) {
		wxTextCtrl * descriptionTextField = m_descriptionTextFields[i];

		if(event.GetId() == descriptionTextField->GetId()) {
			m_group->setDescription(i, event.GetString());
			break;
		}
	}
}

void SunstormInteractiveMetadataPanel::onRunFileTextChanged(wxCommandEvent & event) {
	if(m_group == nullptr) {
		return;
	}

	m_group->setRunFile(event.GetString());
}
