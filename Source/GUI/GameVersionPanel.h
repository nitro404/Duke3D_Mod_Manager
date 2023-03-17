#ifndef _GAME_VERSION_PANEL_H_
#define _GAME_VERSION_PANEL_H_

#include "Game/GameVersion.h"

#include <boost/signals2.hpp>
#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

class SettingPanel;

class GameVersionPanel final : public wxPanel {
public:
	class Listener {
	public:
		virtual ~Listener();

		virtual void gameVersionSettingChanged(GameVersionPanel & gameVersionPanel, SettingPanel & settingPanel) = 0;
		virtual void gameVersionChangesDiscarded(GameVersionPanel & gameVersionPanel) = 0;
		virtual void gameVersionReset(GameVersionPanel & gameVersionPanel) = 0;
		virtual void gameVersionSaved(GameVersionPanel & gameVersionPanel) = 0;
	};

	GameVersionPanel(std::shared_ptr<GameVersion> gameVersion, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~GameVersionPanel();

	bool isModified() const;
	std::string getPanelName() const;
	std::shared_ptr<GameVersion> getGameVersion() const;
	bool isValid() const;
	bool save();
	void discard();
	void reset();
	void discardGamePathChanges();

	void update();

	size_t numberOfListeners() const;
	bool hasListener(const Listener & listener) const;
	size_t indexOfListener(const Listener & listener) const;
	Listener * getListener(size_t index) const;
	bool addListener(Listener & listener);
	bool removeListener(size_t index);
	bool removeListener(const Listener & listener);
	void clearListeners();

private:
	void notifyGameVersionSettingChanged(SettingPanel & settingPanel);
	void notifyGameVersionChangesDiscarded();
	void notifyGameVersionReset();
	void notifyGameVersionSaved();
	void onGameVersionModified(GameVersion & gameVersion);
	void onSettingModified(SettingPanel & settingPanel);

	std::shared_ptr<GameVersion> m_gameVersion;
	boost::signals2::connection m_gameVersionModifiedConnection;
	std::vector<SettingPanel *> m_settingsPanels;
	std::vector<boost::signals2::connection> m_settingModifiedConnections;
	SettingPanel * m_gamePathSettingPanel;
	mutable bool m_modified;
	mutable std::vector<Listener *> m_listeners;
};

#endif // _GAME_VERSION_PANEL_H_
