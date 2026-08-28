#ifndef _MOD_MANAGER_FRAME_H_
#define _MOD_MANAGER_FRAME_H_

#include <Signal/SignalConnectionGroup.h>

#include <boost/signals2.hpp>
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
	ModManagerFrame();
	~ModManagerFrame() override;

	bool isInitialized() const;
	bool initialize(std::shared_ptr<ModManager> modManager);

#if wxUSE_MENUS
	void onFileMenuItemPressed(wxCommandEvent & event);
	void onViewMenuItemPressed(wxCommandEvent & event);
	void onHelpMenuItemPressed(wxCommandEvent & event);
#endif // wxUSE_MENUS

	boost::signals2::signal<void ()> reloadRequested;

private:
	void requestReload();
	void onNotebookPageChanging(wxBookCtrlEvent & event);
	void onNotebookPageChanged(wxBookCtrlEvent & event);
	void onSettingsReset();
	void onSettingsSaved();

#if wxUSE_MENUS
	wxMenuItem * m_exitMenuItem;
	wxMenuItem * m_resetWindowPositionMenuItem;
	wxMenuItem * m_resetWindowSizeMenuItem;
	wxMenuItem * m_aboutMenuItem;
#endif // wxUSE_MENUS

	bool m_initialized;
	wxNotebook * m_notebook;
	SettingsManagerPanel * m_settingsManagerPanel;
	SignalConnectionGroup m_settingsManagerPanelSignalConnectionGroup;

	ModManagerFrame(const ModManagerFrame &) = delete;
	const ModManagerFrame & operator = (const ModManagerFrame &) = delete;
};

#endif // _MOD_MANAGER_FRAME_H_
