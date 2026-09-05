#ifndef _MOD_MANAGER_APPLICATION_H_
#define _MOD_MANAGER_APPLICATION_H_

#include "Manager/ModManager.h"
#include "ModManagerFrame.h"

#include <Arguments/ArgumentParser.h>

#include <boost/signals2.hpp>
#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/progdlg.h>

#include <memory>

class LogSinkWX;
class ModManagerInitializationProgressUpdateEvent;
class ModManagerInitializationDoneEvent;

class ModManagerApplication : public wxApp {
public:
	ModManagerApplication();
	~ModManagerApplication() override;

	void reload();
	void displayArgumentHelp();

	void onFrameClosed(wxCloseEvent & event);

	// wxApp Virtuals
	bool OnInit() override;
	int OnExit() override;
	void CleanUp() override;

private:
	void initialize();
	void showWindow();
	void onInitializationProgressUpdate(ModManagerInitializationProgressUpdateEvent & event);
	void onInitializationDone(ModManagerInitializationDoneEvent & event);
	void onReloadRequested();

	std::shared_ptr<ArgumentParser> m_arguments;
	std::shared_ptr<ModManager> m_modManager;
	std::shared_ptr<LogSinkWX> m_logSinkWX;
	ModManagerFrame * m_modManagerFrame;
	ModManagerFrame * m_newModManagerFrame;
	boost::signals2::connection m_modManagerFrameReloadRequestedConnection;
	bool m_reloadRequired;
	std::future<bool> m_initializeFuture;
	std::unique_ptr<wxProgressDialog> m_initializingProgressDialog;
	boost::signals2::connection m_modManagerInitializationProgressConnection;

	ModManagerApplication(const ModManagerApplication &) = delete;
	const ModManagerApplication & operator = (const ModManagerApplication &) = delete;
};

#endif // _MOD_MANAGER_APPLICATION_H_
