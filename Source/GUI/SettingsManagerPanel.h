#ifndef _SETTINGS_MANAGER_PANEL_H_
#define _SETTINGS_MANAGER_PANEL_H_

#include "DOSBox/DOSBoxVersionCollection.h"
#include "Game/GameVersionCollectionListener.h"
#include "SettingPanel.h"

#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <functional>
#include <memory>

class GameVersionCollection;
class ModManager;

class SettingsManagerPanel final : public wxPanel,
								   public SettingPanel::Listener,
								   public DOSBoxVersionCollection::Listener,
								   public GameVersionCollectionListener {
public:
	class Listener {
	public:
		virtual ~Listener();

		virtual void settingsChanged() = 0;
		virtual void settingsReset() = 0;
		virtual void settingsDiscarded() = 0;
		virtual void settingsSaved() = 0;
	};

	SettingsManagerPanel(std::shared_ptr<ModManager> modManager, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~SettingsManagerPanel();

	bool isModified() const;
	bool isValid() const;
	void reset();
	void discard();
	bool save();

	void updateButtons();

	void onResetDefaultsButtonPressed(wxCommandEvent & event);
	void onDiscardChangesButtonPressed(wxCommandEvent & event);
	void onSaveSettingsButtonPressed(wxCommandEvent & event);

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

	// GameVersionCollectionListener Virtuals
	virtual void gameVersionCollectionSizeChanged(GameVersionCollection & gameVersionCollection) override;
	virtual void gameVersionCollectionItemModified(GameVersionCollection & gameVersionCollection, GameVersion & gameVersion) override;

private:
	void notifySettingsChanged();
	void notifySettingsReset();
	void notifySettingsDiscarded();
	void notifySettingsSaved();

	std::shared_ptr<ModManager> m_modManager;
	std::vector<SettingPanel *> m_settingsPanels;
	StringChoiceSettingPanel * m_preferredDOSBoxVersionSettingPanel;
	StringChoiceSettingPanel * m_preferredGameVersionSettingPanel;
	bool m_modified;
	std::vector<Listener *> m_listeners;
	wxButton * m_discardChangesButton;
	wxButton * m_saveSettingsButton;
};

#endif // _SETTINGS_MANAGER_PANEL_H_
