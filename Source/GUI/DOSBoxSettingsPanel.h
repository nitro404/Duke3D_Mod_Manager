#ifndef _DOSBOX_SETTINGS_PANEL_H_
#define _DOSBOX_SETTINGS_PANEL_H_

#include "DOSBox/DOSBoxVersionCollection.h"
#include "Manager/ModManager.h"
#include "SettingPanel.h"

#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <string>
#include <vector>

class DOSBoxSettingsPanel final
	: public wxStaticBox
	, public SettingPanel::Listener
	, public DOSBoxVersionCollection::Listener
	, public ModManager::Listener {
public:
	class Listener {
	public:
		virtual ~Listener();

		virtual void dosboxSettingModified(SettingPanel & settingPanel) = 0;
	};

	DOSBoxSettingsPanel(std::shared_ptr<ModManager> modManager, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxALIGN_LEFT, const std::string & title = "DOSBox Settings");
	virtual ~DOSBoxSettingsPanel();

	bool areSettingsModified() const;
	bool areSettingsValid() const;
	void updateSettings();
	bool saveSettings();
	void discardSettings();
	void resetSettings();

	size_t numberOfListeners() const;
	bool hasListener(const Listener & listener) const;
	size_t indexOfListener(const Listener & listener) const;
	Listener * getListener(size_t index) const;
	bool addListener(Listener & listener);
	bool removeListener(size_t index);
	bool removeListener(const Listener & listener);
	void clearListeners();

	// SettingPanel::Listener Virtuals
	virtual void settingModified(SettingPanel & settingPanel) override;

	// DOSBoxVersionCollection::Listener Virtuals
	virtual void dosboxVersionCollectionSizeChanged(DOSBoxVersionCollection & dosboxVersionCollection) override;
	virtual void dosboxVersionCollectionItemModified(DOSBoxVersionCollection & dosboxVersionCollection, DOSBoxVersion & dosboxVersion) override;

	// ModManager::Listener Virtuals
	virtual void modSelectionChanged(const std::shared_ptr<Mod> & mod, size_t modVersionIndex, size_t modVersionTypeIndex, size_t modGameVersionIndex) override;
	virtual void gameTypeChanged(GameType gameType) override;
	virtual void preferredDOSBoxVersionChanged(const std::shared_ptr<DOSBoxVersion> & dosboxVersion) override;
	virtual void preferredGameVersionChanged(const std::shared_ptr<GameVersion> & gameVersion) override;
	virtual void dosboxServerIPAddressChanged(const std::string & ipAddress) override;
	virtual void dosboxLocalServerPortChanged(uint16_t port) override;
	virtual void dosboxRemoteServerPortChanged(uint16_t port) override;

private:
	std::shared_ptr<ModManager> m_modManager;
	std::vector<SettingPanel *> m_settingsPanels;
	StringChoiceSettingPanel * m_preferredDOSBoxVersionSettingPanel;
	bool m_modified;
	std::vector<Listener *> m_listeners;
};

#endif // _DOSBOX_SETTINGS_PANEL_H_
