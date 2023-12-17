#include "ProcessRunningDialog.h"

#include "Manager/SettingsManager.h"
#include "Project.h"

#include <Analytics/Segment/SegmentAnalytics.h>
#include <Platform/Process.h>

#include <fmt/core.h>
#include <wx/gbsizer.h>

wxDECLARE_EVENT(EVENT_PROCESS_TERMINATED, ProcessTerminatedEvent);

class ProcessTerminatedEvent final : public wxEvent {
public:
	ProcessTerminatedEvent()
		: wxEvent(0, EVENT_PROCESS_TERMINATED)
		, m_nativeExitCode(0)
		, m_forceTerminated(false) { }

	virtual ~ProcessTerminatedEvent() { }

	virtual wxEvent * Clone() const override {
		return new ProcessTerminatedEvent(*this);
	}

	void setProcessExitStatus(uint64_t nativeExitCode, bool forceTerminated) {
		m_nativeExitCode = nativeExitCode;
		m_forceTerminated = forceTerminated;
	}

	uint64_t getNativeExitCode() const {
		return m_nativeExitCode;
	}

	bool wasForceTerminated() const {
		return m_forceTerminated;
	}

	DECLARE_DYNAMIC_CLASS(ProcessTerminatedEvent);

private:
	uint64_t m_nativeExitCode;
	bool m_forceTerminated;
};

IMPLEMENT_DYNAMIC_CLASS(ProcessTerminatedEvent, wxEvent);

wxDEFINE_EVENT(EVENT_PROCESS_TERMINATED, ProcessTerminatedEvent);

ProcessRunningDialog::ProcessRunningDialog(wxWindow * parent, const std::string & title, const std::string & message, const std::string & closeButtonCaption)
	: wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX, "Process Running Dialog")
	, m_message(message)
	, m_messageLabel(nullptr) {
	SetIcon(wxICON(D3DMODMGR_ICON));

	int border = 5;

	wxPanel * contentsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);

	m_messageLabel = new wxStaticText(contentsPanel, wxID_ANY, m_message, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_messageLabel->Wrap(GetClientSize().x - (border * 2));

	wxButton * closeButton = new wxButton(contentsPanel, wxID_ANY, closeButtonCaption, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Close");
	closeButton->Bind(wxEVT_BUTTON, &ProcessRunningDialog::onCloseButtonPressed, this);

	wxGridBagSizer * contentsSizer = new wxGridBagSizer(border, border);
	contentsSizer->Add(m_messageLabel, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND | wxHORIZONTAL | wxVERTICAL | wxALIGN_LEFT | wxALIGN_TOP, border);
	contentsSizer->Add(closeButton, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, border);
	contentsSizer->AddGrowableRow(0, 1);
	contentsSizer->AddGrowableCol(0, 1);
	contentsPanel->SetSizer(contentsSizer);

	wxGridSizer * processRunningDialogSizer = new wxGridSizer(1, 1, border);
	processRunningDialogSizer->Add(contentsPanel, 1, wxEXPAND | wxALL, border);
	SetSizer(processRunningDialogSizer);
}

ProcessRunningDialog::~ProcessRunningDialog() {
	m_processTermintedConnection.disconnect();
}

void ProcessRunningDialog::setStatus(const std::string & status) {
	m_messageLabel->SetLabelText(fmt::format("{}\n\n{}", m_message, status));
}

void ProcessRunningDialog::clearStatus() {
	m_messageLabel->SetLabelText(m_message);
}

void ProcessRunningDialog::setProcess(std::shared_ptr<Process> process) {
	if(m_process != nullptr) {
		m_processTermintedConnection.disconnect();
	}

	if(process == nullptr) {
		return;
	}

	m_process = process;
	m_processTermintedConnection = m_process->terminated.connect(std::bind(static_cast<void(ProcessRunningDialog::*)(uint64_t, bool)>(&ProcessRunningDialog::onProcessTerminated), this, std::placeholders::_1, std::placeholders::_2));

	Bind(EVENT_PROCESS_TERMINATED, &ProcessRunningDialog::onProcessTerminated, this);
}

void ProcessRunningDialog::close() {
	if(m_process->isRunning()) {
		m_process->terminate();

		if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
			SegmentAnalytics::getInstance()->track("Process Terminated");
		}
	}
}

void ProcessRunningDialog::onProcessTerminated(uint64_t nativeExitCode, bool forceTerminated) {
	ProcessTerminatedEvent * processTerminatedEvent = new ProcessTerminatedEvent();
	processTerminatedEvent->setProcessExitStatus(nativeExitCode, forceTerminated);
	QueueEvent(processTerminatedEvent);
}

void ProcessRunningDialog::onProcessTerminated(ProcessTerminatedEvent & processTerminatedEvent) {
	EndModal(processTerminatedEvent.getNativeExitCode());
}

void ProcessRunningDialog::onCloseButtonPressed(wxCommandEvent & event) {
	close();
}

void ProcessRunningDialog::onClose(wxCloseEvent & closeEvent) {
	close();
}

wxBEGIN_EVENT_TABLE(ProcessRunningDialog, wxDialog)
	EVT_CLOSE(ProcessRunningDialog::onClose)
wxEND_EVENT_TABLE()
