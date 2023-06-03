#include "MetadataPanel.h"

#include <spdlog/spdlog.h>

MetadataPanel::MetadataPanel(wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style, const std::string & title)
	: wxScrolledWindow(parent, windowID, position, size, style, title.empty() ? "Metadata" : title)
	, m_metadataPanelSizer(nullptr) {
	SetScrollRate(5, 5);

	int border = 5;

	m_metadataPanelSizer = new wxFlexGridSizer(2, border, border);
	m_metadataPanelSizer->AddGrowableCol(0, 0);
	m_metadataPanelSizer->AddGrowableCol(1, 1);
	SetSizer(m_metadataPanelSizer);
}

MetadataPanel::~MetadataPanel() { }

void MetadataPanel::setMetadata(const std::vector<std::pair<std::string, std::string>> & metadata) {
	clearMetadata();

	for(const std::pair<std::string, std::string> metadataEntry : metadata) {
		wxStaticText * metadataEntryLabel = new wxStaticText(this, wxID_ANY, metadataEntry.first, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
		metadataEntryLabel->SetFont(metadataEntryLabel->GetFont().MakeBold());
		// Note: For some reason ampersands are not rendered if you assign the text contents upon creation rather than through the set label text function
		metadataEntryLabel->SetLabelText(wxString::FromUTF8(metadataEntry.first));
		wxStaticText * metadataEntryText = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, metadataEntry.first);
		// Note: For some reason ampersands are not rendered if you assign the text contents upon creation rather than through the set label text function
		metadataEntryText->SetLabelText(wxString::FromUTF8(metadataEntry.second));

		m_metadataText.push_back(std::make_pair(metadataEntryLabel, metadataEntryText));

		m_metadataPanelSizer->Add(metadataEntryLabel, 1, wxEXPAND | wxALL);
		m_metadataPanelSizer->Add(metadataEntryText, 1, wxEXPAND | wxALL);
	}

	Layout();
}

void MetadataPanel::clearMetadata() {
	DestroyChildren();
	m_metadataText.clear();
}
