#include "ReleaseNotesPanel.h"

#include "Project.h"

#include "WXUtilities.h"

#include <Analytics/Segment/SegmentAnalytics.h>
#include <Date.h>
#include <GitHub/GitHubService.h>
#include <Manager/SettingsManager.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/TimeUtilities.h>

#include <magic_enum.hpp>
#include <wx/gbsizer.h>
#include <wx/listbox.h>
#include <wx/wrapsizer.h>

wxDECLARE_EVENT(EVENT_RELEASES_LOADED, ReleasesLoadedEvent);

class ReleasesLoadedEvent final : public wxEvent {
public:
	ReleasesLoadedEvent()
		: wxEvent(0, EVENT_RELEASES_LOADED) { }

	virtual ~ReleasesLoadedEvent() { }

	virtual wxEvent * Clone() const override {
		return new ReleasesLoadedEvent(*this);
	}

	DECLARE_DYNAMIC_CLASS(ReleasesLoadedEvent);
};

IMPLEMENT_DYNAMIC_CLASS(ReleasesLoadedEvent, wxEvent);

wxDEFINE_EVENT(EVENT_RELEASES_LOADED, ReleasesLoadedEvent);

ReleaseNotesPanel::ReleaseNotesPanel(wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Release Notes")
	, m_releaseComboBox(nullptr)
	, m_releaseNameText(nullptr)
	, m_releaseDateText(nullptr)
	, m_scrollableDescriptionWindow(nullptr)
	, m_releaseDescriptionText(nullptr) {
	m_releaseComboBox = new wxComboBox(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, {}, 0, wxDefaultValidator, "Release Version");
	m_releaseComboBox->SetEditable(false);
	m_releaseComboBox->Bind(wxEVT_COMBOBOX, &ReleaseNotesPanel::onReleaseSelected, this);

	wxPanel * releaseTitlePanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER, "Release Title");

	m_releaseNameText = new wxStaticText(releaseTitlePanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_releaseNameText->SetFont(m_releaseNameText->GetFont().MakeBold());

	m_releaseDateText = new wxStaticText(releaseTitlePanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	m_scrollableDescriptionWindow = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "Release Description");
	m_scrollableDescriptionWindow->SetScrollRate(5, 5);

	m_releaseDescriptionText = new wxStaticText(m_scrollableDescriptionWindow, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

	int border = 5;

	wxWrapSizer * releaseTitleSizer = new wxWrapSizer(wxHORIZONTAL);
	releaseTitleSizer->Add(m_releaseNameText);
	releaseTitleSizer->Add(m_releaseDateText);
	releaseTitlePanel->SetSizerAndFit(releaseTitleSizer);

	wxBoxSizer * scrollableDescriptionSizer = new wxBoxSizer(wxVERTICAL);
	scrollableDescriptionSizer->Add(m_releaseDescriptionText);
	m_scrollableDescriptionWindow->SetSizer(scrollableDescriptionSizer);

	wxGridBagSizer * releaseNotesSizer = new wxGridBagSizer(border, border);
	releaseNotesSizer->Add(m_releaseComboBox, wxGBPosition(0, 0), wxGBSpan(1, 1), wxSTRETCH_NOT, border);
	releaseNotesSizer->Add(releaseTitlePanel, wxGBPosition(1, 0), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL, border);
	releaseNotesSizer->Add(m_scrollableDescriptionWindow, wxGBPosition(2, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	releaseNotesSizer->AddGrowableRow(2, 1);
	releaseNotesSizer->AddGrowableCol(0, 1);
	SetSizer(releaseNotesSizer);

	Bind(EVENT_RELEASES_LOADED, &ReleaseNotesPanel::onReleasesLoaded, this);
}

ReleaseNotesPanel::~ReleaseNotesPanel() { }

void ReleaseNotesPanel::load() {
	std::call_once(m_loadFlag, [this]() {
		m_releasesFuture = std::async(std::launch::async, [this]() {
			std::lock_guard<std::recursive_mutex> lock(m_releasesMutex);

			m_releases = GitHubService::getInstance()->getReleases(APPLICATION_REPOSITORY_URL);

			QueueEvent(new ReleasesLoadedEvent());
		});
	});
}

void ReleaseNotesPanel::setSelectedRelease(std::shared_ptr<GitHubRelease> release) {
	std::lock_guard<std::recursive_mutex> lock(m_releasesMutex);

	m_selectedRelease = release;

	if(m_selectedRelease == nullptr || !m_selectedRelease->isValid()) {
		m_releaseNameText->SetLabel("");
		m_releaseDateText->SetLabel("");
		m_releaseDescriptionText->SetLabel("");
		return;
	}

	m_releaseNameText->SetLabel(m_selectedRelease->getReleaseName());
	m_releaseDateText->SetLabel("  (" + Date(m_selectedRelease->getPublishedTimestamp()).toString() + ")");
	m_releaseDescriptionText->SetLabel(m_selectedRelease->getBody());

	m_scrollableDescriptionWindow->FitInside();

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["releaseID"] = m_selectedRelease->getID();
		properties["releaseName"] = m_selectedRelease->getReleaseName();
		properties["tagName"] = m_selectedRelease->getTagName();
		properties["draft"] = m_selectedRelease->isDraft();
		properties["preRelease"] = m_selectedRelease->isPreRelease();
		properties["createdAt"] = Utilities::timePointToString(m_selectedRelease->getCreatedTimestamp(), Utilities::TimeFormat::ISO8601);
		properties["publishedAt"] = Utilities::timePointToString(m_selectedRelease->getPublishedTimestamp(), Utilities::TimeFormat::ISO8601);
		properties["numberOfAssets"] = m_selectedRelease->numberOfAssets();
		properties["numberOfReleases"] = m_releases->numberOfReleases();

		SegmentAnalytics::getInstance()->track("View Release Notes", properties);
	}
}

void ReleaseNotesPanel::onReleasesLoaded(ReleasesLoadedEvent & event) {
	std::lock_guard<std::recursive_mutex> lock(m_releasesMutex);

	size_t currentReleaseIndex = m_releases->indexOfReleaseWithTagVersion(APPLICATION_VERSION);

	if(currentReleaseIndex == std::numeric_limits<size_t>::max()) {
		currentReleaseIndex = m_releases->indexOfLatestRelease();
	}

	m_releaseComboBox->Set(WXUtilities::createItemWXArrayString(m_releases->getReleaseTagNames()));

	if(currentReleaseIndex != std::numeric_limits<size_t>::max()) {
		m_releaseComboBox->SetSelection(currentReleaseIndex);

		setSelectedRelease(m_releases->getRelease(currentReleaseIndex));
	}
}

void ReleaseNotesPanel::onReleaseSelected(wxCommandEvent & event) {
	std::lock_guard<std::recursive_mutex> lock(m_releasesMutex);

	int selectedReleaseIndex = m_releaseComboBox->GetSelection();

	if(selectedReleaseIndex != wxNOT_FOUND) {
		setSelectedRelease(m_releases->getRelease(selectedReleaseIndex));
	}
}
