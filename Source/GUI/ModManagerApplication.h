#ifndef _MOD_MANAGER_APPLICATION_H_
#define _MOD_MANAGER_APPLICATION_H_

#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <memory>

class LogSinkWX;
class ModManager;
class ModManagerFrame;

class ModManagerApplication : public wxApp {
public:
	ModManagerApplication();
	virtual ~ModManagerApplication();

	void displayArgumentHelp();

	// wxApp Virtuals
	virtual bool OnInit() override;
	virtual int OnExit() override;
	virtual void CleanUp() override;

private:
	std::shared_ptr<ModManager> m_modManager;
	std::shared_ptr<LogSinkWX> m_logSinkWX;
};

#endif // _MOD_MANAGER_APPLICATION_H_
