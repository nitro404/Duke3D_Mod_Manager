#ifndef _MOD_MANAGER_FRAME_H_
#define _MOD_MANAGER_FRAME_H_

#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <memory>

class ModManager;

class ModManagerFrame final : public wxFrame {
public:
	ModManagerFrame(std::shared_ptr<ModManager> modManager);
	virtual ~ModManagerFrame();

#if wxUSE_MENUS
	void onMenuBarItemPressed(wxCommandEvent & event);
#endif // wxUSE_MENUS

	void onQuit(wxCommandEvent & event);
	void onAbout(wxCommandEvent & event);

private:
#if wxUSE_MENUS
	wxMenuItem * m_resetWindowPositionMenuItem;
	wxMenuItem * m_resetWindowSizeMenuItem;
#endif // wxUSE_MENUS

	wxDECLARE_EVENT_TABLE();
};

#endif // _MOD_MANAGER_FRAME_H_
