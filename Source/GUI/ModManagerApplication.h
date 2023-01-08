#ifndef _MOD_MANAGER_APPLICATION_H_
#define _MOD_MANAGER_APPLICATION_H_

#include "Manager/ModManager.h"
#include "ModManagerFrame.h"

#include <Arguments/ArgumentParser.h>

#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <memory>

class LogSinkWX;

class ModManagerApplication : public wxApp,
							  public ModManagerFrame::Listener {
public:
	ModManagerApplication();
	virtual ~ModManagerApplication();

	void reload();
	void displayArgumentHelp();

	void onFrameClosed(wxCloseEvent & event);

	// wxApp Virtuals
	virtual bool OnInit() override;
	virtual int OnExit() override;
	virtual void CleanUp() override;

	// ModManagerFrame::Listener Virtuals
	virtual void reloadRequested() override;

private:
	void initialize();

	std::shared_ptr<ArgumentParser> m_arguments;
	std::shared_ptr<ModManager> m_modManager;
	std::shared_ptr<LogSinkWX> m_logSinkWX;
	ModManagerFrame * m_modManagerFrame;
	ModManagerFrame * m_newModManagerFrame;
	bool m_reloadRequired;
};

#endif // _MOD_MANAGER_APPLICATION_H_
