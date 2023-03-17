#ifndef _MOD_MANAGER_FRAME_H_
#define _MOD_MANAGER_FRAME_H_

#include <Signal/SignalConnectionGroup.h>

#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/bookctrl.h>

#include <memory>

class ModManager;
class SettingsManagerPanel;

class ModManagerFrame final : public wxFrame {
public:
	class Listener {
	public:
		virtual ~Listener();

		virtual void reloadRequested() = 0;
	};

	ModManagerFrame();
	virtual ~ModManagerFrame();

	bool isInitialized() const;
	bool initialize(std::shared_ptr<ModManager> modManager);

	bool hasListener() const;
	Listener * getListener() const;
	void setListener(Listener & listener);
	void clearListener();

#if wxUSE_MENUS
	void onMenuBarItemPressed(wxCommandEvent & event);
#endif // wxUSE_MENUS

private:
	void requestReload();
	void onNotebookPageChanging(wxBookCtrlEvent & event);
	void onNotebookPageChanged(wxBookCtrlEvent & event);
	void onQuit(wxCommandEvent & event);
	void onAbout(wxCommandEvent & event);
	void onSettingsReset();
	void onSettingsSaved();

#if wxUSE_MENUS
	wxMenuItem * m_resetWindowPositionMenuItem;
	wxMenuItem * m_resetWindowSizeMenuItem;
#endif // wxUSE_MENUS

	bool m_initialized;
	wxNotebook * m_notebook;
	SettingsManagerPanel * m_settingsManagerPanel;
	SignalConnectionGroup m_settingsManagerPanelSignalConnectionGroup;
	Listener * m_listener;

	wxDECLARE_EVENT_TABLE();
};

#endif // _MOD_MANAGER_FRAME_H_
