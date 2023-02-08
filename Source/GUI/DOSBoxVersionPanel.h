#ifndef _DOSBOX_VERSION_PANEL_H_
#define _DOSBOX_VERSION_PANEL_H_

#include "DOSBox/DOSBoxVersion.h"
#include "SettingPanel.h"

#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

class DOSBoxVersionPanel final
	: public wxPanel
	, public DOSBoxVersion::Listener
	, public SettingPanel::Listener {
public:
	class Listener {
	public:
		virtual ~Listener();

		virtual void dosboxVersionSettingChanged(DOSBoxVersionPanel & dosboxVersionPanel, SettingPanel & settingPanel) = 0;
		virtual void dosboxVersionChangesDiscarded(DOSBoxVersionPanel & dosboxVersionPanel) = 0;
		virtual void dosboxVersionReset(DOSBoxVersionPanel & dosboxVersionPanel) = 0;
		virtual void dosboxVersionSaved(DOSBoxVersionPanel & dosboxVersionPanel) = 0;
	};

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

	size_t numberOfListeners() const;
	bool hasListener(const Listener & listener) const;
	size_t indexOfListener(const Listener & listener) const;
	Listener * getListener(size_t index) const;
	bool addListener(Listener & listener);
	bool removeListener(size_t index);
	bool removeListener(const Listener & listener);
	void clearListeners();

	// DOSBoxVersion::Listener Virtuals
	virtual void dosboxVersionModified(DOSBoxVersion & dosboxVersion) override;

	// SettingPanel::Listener Virtuals
	virtual void settingModified(SettingPanel & settingPanel) override;

private:
	void notifyDOSBoxVersionSettingChanged(SettingPanel & settingPanel);
	void notifyDOSBoxVersionChangesDiscarded();
	void notifyDOSBoxVersionReset();
	void notifyDOSBoxVersionSaved();

	std::shared_ptr<DOSBoxVersion> m_dosboxVersion;
	std::vector<SettingPanel *> m_settingsPanels;
	SettingPanel * m_dosboxPathSettingPanel;
	mutable bool m_modified;
	mutable std::vector<Listener *> m_listeners;
};

#endif // _DOSBOX_VERSION_PANEL_H_