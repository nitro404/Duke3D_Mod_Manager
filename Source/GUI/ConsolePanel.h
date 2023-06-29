#ifndef _CONSOLE_PANEL_H_
#define _CONSOLE_PANEL_H_

#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

class CustomLogTextControl;
class PreformattedLogFormatter;

class ConsolePanel final : public wxPanel {
public:
	ConsolePanel(wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~ConsolePanel();

private:
	CustomLogTextControl * m_logger;
	PreformattedLogFormatter * m_logFormatter;
	wxTextCtrl * m_logTextArea;

	ConsolePanel(const ConsolePanel &) = delete;
	const ConsolePanel & operator = (const ConsolePanel &) = delete;
};

#endif // _CONSOLE_PANEL_H_
