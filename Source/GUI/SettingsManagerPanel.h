#ifndef _SETTINGS_MANAGER_PANEL_H_
#define _SETTINGS_MANAGER_PANEL_H_

#include <boost/signals2.hpp>
#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <functional>
#include <memory>

class DOSBoxVersion;
class DOSBoxVersionCollection;
class GameVersion;
class GameVersionCollection;
class ModManager;
class SettingPanel;
class StringChoiceSettingPanel;

class SettingsManagerPanel final
	: public wxPanel {
public:
	SettingsManagerPanel(std::shared_ptr<ModManager> modManager, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~SettingsManagerPanel();

	bool isModified() const;
	bool isValid() const;
	void reset();
	void discard();
	bool save();

	void updateButtons();

	boost::signals2::signal<void ()> settingsChanged;
	boost::signals2::signal<void ()> settingsReset;
	boost::signals2::signal<void ()> settingsDiscarded;
	boost::signals2::signal<void ()> settingsSaved;

private:
	void onResetDefaultsButtonPressed(wxCommandEvent & event);
	void onDiscardChangesButtonPressed(wxCommandEvent & event);
	void onSaveSettingsButtonPressed(wxCommandEvent & event);
	void onDOSBoxVersionCollectionSizeChanged(DOSBoxVersionCollection & dosboxVersionCollection);
	void onDOSBoxVersionCollectionItemModified(DOSBoxVersionCollection & dosboxVersionCollection, DOSBoxVersion & dosboxVersion);
	void onGameVersionCollectionSizeChanged(GameVersionCollection & gameVersionCollection);
	void onGameVersionCollectionItemModified(GameVersionCollection & gameVersionCollection, GameVersion & gameVersion);
	void onSettingModified(SettingPanel & settingPanel);

	std::shared_ptr<ModManager> m_modManager;
	boost::signals2::connection m_dosboxVersionCollectionSizeChangedConnection;
	boost::signals2::connection m_dosboxVersionCollectionItemModifiedConnection;
	boost::signals2::connection m_gameVersionCollectionSizeChangedConnection;
	boost::signals2::connection m_gameVersionCollectionItemModifiedConnection;
	std::vector<SettingPanel *> m_settingsPanels;
	std::vector<boost::signals2::connection> m_settingModifiedConnections;
	StringChoiceSettingPanel * m_preferredDOSBoxVersionSettingPanel;
	StringChoiceSettingPanel * m_preferredGameVersionSettingPanel;
	bool m_modified;
	wxButton * m_discardChangesButton;
	wxButton * m_saveSettingsButton;
};

#endif // _SETTINGS_MANAGER_PANEL_H_
