#ifndef _DOSBOX_SETTINGS_PANEL_H_
#define _DOSBOX_SETTINGS_PANEL_H_

#include "DOSBox/DOSBoxVersionCollection.h"
#include "Manager/ModManager.h"

#include <boost/signals2.hpp>
#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <string>
#include <vector>

class SettingPanel;
class StringChoiceSettingPanel;

class DOSBoxSettingsPanel final
	: public wxStaticBox {
public:
	DOSBoxSettingsPanel(std::shared_ptr<ModManager> modManager, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxALIGN_LEFT, const std::string & title = "DOSBox Settings");
	virtual ~DOSBoxSettingsPanel();

	bool areSettingsModified() const;
	bool areSettingsValid() const;
	void updateSettings();
	bool saveSettings();
	void discardSettings();
	void resetSettings();

	boost::signals2::signal<void (SettingPanel & /* settingPanel */)> dosboxSettingModified;

private:
	void onPreferredDOSBoxVersionChanged(std::shared_ptr<DOSBoxVersion> dosboxVersion);
	void onDOSBoxServerIPAddressChanged(std::string ipAddress);
	void onDOSBoxLocalServerPortChanged(uint16_t port);
	void onDOSBoxRemoteServerPortChanged(uint16_t port);
	void onDOSBoxVersionCollectionSizeChanged(DOSBoxVersionCollection & dosboxVersionCollection);
	void onDOSBoxVersionCollectionItemModified(DOSBoxVersionCollection & dosboxVersionCollection, DOSBoxVersion & dosboxVersion);
	void onSettingModified(SettingPanel & settingPanel);

	std::shared_ptr<ModManager> m_modManager;
	boost::signals2::connection m_preferredDOSBoxVersionChangedConnection;
	boost::signals2::connection m_dosboxServerIPAddressChangedConnection;
	boost::signals2::connection m_dosboxLocalServerPortChangedConnection;
	boost::signals2::connection m_dosboxRemoteServerPortChangedConnection;
	boost::signals2::connection m_dosboxVersionCollectionSizeChangedConnection;
	boost::signals2::connection m_dosboxVersionCollectionItemModifiedConnection;
	std::vector<SettingPanel *> m_settingsPanels;
	std::vector<boost::signals2::connection> m_settingModifiedConnections;
	StringChoiceSettingPanel * m_preferredDOSBoxVersionSettingPanel;
	bool m_modified;
};

#endif // _DOSBOX_SETTINGS_PANEL_H_
