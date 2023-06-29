#ifndef _RELEASE_NOTES_PANEL_H_
#define _RELEASE_NOTES_PANEL_H_

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <future>
#include <memory>
#include <mutex>
#include <string>

class GitHubRelease;
class GitHubReleaseCollection;
class ReleasesLoadedEvent;

class ReleaseNotesPanel final : public wxPanel {
public:
	ReleaseNotesPanel(wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~ReleaseNotesPanel();

	void load();

private:
	void setSelectedRelease(std::shared_ptr<GitHubRelease> release);
	void onReleasesLoaded(ReleasesLoadedEvent & event);
	void onReleaseSelected(wxCommandEvent & event);

	std::unique_ptr<GitHubReleaseCollection> m_releases;
	std::shared_ptr<GitHubRelease> m_selectedRelease;
	std::future<void> m_releasesFuture;
	mutable std::recursive_mutex m_releasesMutex;
	std::once_flag m_loadFlag;

	wxComboBox * m_releaseComboBox;
	wxStaticText * m_releaseNameText;
	wxStaticText * m_releaseDateText;
	wxScrolledWindow * m_scrollableDescriptionWindow;
	wxStaticText * m_releaseDescriptionText;

	ReleaseNotesPanel(const ReleaseNotesPanel &) = delete;
	const ReleaseNotesPanel & operator = (const ReleaseNotesPanel &) = delete;
};

#endif // _RELEASE_NOTES_PANEL_H_
