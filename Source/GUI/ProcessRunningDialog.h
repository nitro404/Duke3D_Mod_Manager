#ifndef _PROCESS_RUNNING_DIALOG_H_
#define _PROCESS_RUNNING_DIALOG_H_

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <boost/signals2.hpp>

#include <future>
#include <memory>
#include <string>

class Process;
class ProcessTerminatedEvent;

class ProcessRunningDialog final : public wxDialog {
public:
	ProcessRunningDialog(wxWindow * parent, const std::string & title, const std::string & message, const std::string & closeButtonCaption);
	virtual ~ProcessRunningDialog();

	void setProcess(std::shared_ptr<Process> process);
	void close();

private:
	void onProcessTerminated(uint64_t nativeExitCode, bool forceTerminated);
	void onProcessTerminated(ProcessTerminatedEvent & processTerminatedEvent);
	void onCloseButtonPressed(wxCommandEvent & event);
	void onClose(wxCloseEvent & closeEvent);

	std::shared_ptr<Process> m_process;
	boost::signals2::connection m_processTermintedConnection;

	ProcessRunningDialog(const ProcessRunningDialog &) = delete;
	const ProcessRunningDialog & operator = (const ProcessRunningDialog &) = delete;

	wxDECLARE_EVENT_TABLE();
};

#endif // _PROCESS_RUNNING_DIALOG_H_
