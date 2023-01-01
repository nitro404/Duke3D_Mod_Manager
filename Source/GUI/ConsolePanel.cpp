#include "ConsolePanel.h"

#include "Logging/CustomLogTextControl.h"
#include "Logging/PreformattedLogFormatter.h"

ConsolePanel::ConsolePanel(wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Console")
	, m_logger(nullptr)
	, m_logFormatter(nullptr)
	, m_logTextArea(nullptr) {
	m_logTextArea = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	m_logTextArea->SetEditable(false);

	m_logFormatter = new PreformattedLogFormatter();

	m_logger = new CustomLogTextControl(m_logTextArea);
	m_logger->SetLogLevel(wxLOG_Debug);
	delete m_logger->SetFormatter(m_logFormatter);
	m_logger->SetActiveTarget(m_logger);

	wxBoxSizer * consoleBoxSizer = new wxBoxSizer(wxVERTICAL);
	consoleBoxSizer->Add(m_logTextArea, 1, wxEXPAND | wxALL, 0);
	SetSizer(consoleBoxSizer);
}

ConsolePanel::~ConsolePanel() { }
