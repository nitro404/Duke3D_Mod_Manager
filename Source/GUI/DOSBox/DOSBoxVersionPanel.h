#ifndef _DOSBOX_VERSION_PANEL_H_
#define _DOSBOX_VERSION_PANEL_H_

#include "../SettingPanel.h"

#include <boost/signals2.hpp>
#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

class DOSBoxVersion;

class DOSBoxVersionPanel final
	: public wxPanel {
public:
	DOSBoxVersionPanel(std::shared_ptr<DOSBoxVersion> dosboxVersion, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~DOSBoxVersionPanel();

	bool isModified() const;
	std::string getPanelName() const;
	std::shared_ptr<DOSBoxVersion> getDOSBoxVersion() const;
	bool isValid() const;
	bool save();
	void discard();
	void reset();
	void discardDOSBoxPathChanges();

	void update();

	boost::signals2::signal<void (DOSBoxVersionPanel & /* dosboxVersionPanel */, SettingPanel & /* settingPanel */)> dosboxVersionSettingChanged;
	boost::signals2::signal<void (DOSBoxVersionPanel & /* dosboxVersionPanel */)> dosboxVersionChangesDiscarded;
	boost::signals2::signal<void (DOSBoxVersionPanel & /* dosboxVersionPanel */)> dosboxVersionReset;
	boost::signals2::signal<void (DOSBoxVersionPanel & /* dosboxVersionPanel */)> dosboxVersionSaved;

private:
	void onDOSBoxVersionModified(DOSBoxVersion & dosboxVersion);
	void onSettingModified(SettingPanel & settingPanel);

	std::shared_ptr<DOSBoxVersion> m_dosboxVersion;
	boost::signals2::connection m_dosboxVersionModifiedConnection;
	std::vector<SettingPanel *> m_settingsPanels;
	SettingPanel * m_dosboxIDSettingPanel;
	SettingPanel * m_dosboxPathSettingPanel;
	std::vector<boost::signals2::connection> m_settingModifiedConnections;
	mutable bool m_modified;

	DOSBoxVersionPanel(const DOSBoxVersionPanel &) = delete;
	const DOSBoxVersionPanel & operator = (const DOSBoxVersionPanel &) = delete;
};

#endif // _DOSBOX_VERSION_PANEL_H_
