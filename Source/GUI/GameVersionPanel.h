#ifndef _GAME_VERSION_PANEL_H_
#define _GAME_VERSION_PANEL_H_

#include <boost/signals2.hpp>
#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

class GameVersion;
class SettingPanel;

class GameVersionPanel final : public wxPanel {
public:
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

	boost::signals2::signal<void (GameVersionPanel & /* gameVersionPanel */, SettingPanel & /* settingPanel */)> gameVersionSettingChanged;
	boost::signals2::signal<void (GameVersionPanel & /* gameVersionPanel */)> gameVersionNotesChanged;
	boost::signals2::signal<void (GameVersionPanel & /* gameVersionPanel */)> gameVersionChangesDiscarded;
	boost::signals2::signal<void (GameVersionPanel & /* gameVersionPanel */)> gameVersionReset;
	boost::signals2::signal<void (GameVersionPanel & /* gameVersionPanel */)> gameVersionSaved;

private:
	void onGameVersionModified(GameVersion & gameVersion);
	void onSettingModified(SettingPanel & settingPanel);
	void onNotesModified(wxCommandEvent & event);

	std::shared_ptr<GameVersion> m_gameVersion;
	boost::signals2::connection m_gameVersionModifiedConnection;
	std::vector<SettingPanel *> m_settingsPanels;
	SettingPanel * m_gameVersionIDSettingPanel;
	SettingPanel * m_gamePathSettingPanel;
	wxTextCtrl * m_notesTextField;
	std::vector<boost::signals2::connection> m_settingModifiedConnections;
	mutable bool m_modified;

	GameVersionPanel(const GameVersionPanel &) = delete;
	const GameVersionPanel & operator = (const GameVersionPanel &) = delete;
};

#endif // _GAME_VERSION_PANEL_H_
