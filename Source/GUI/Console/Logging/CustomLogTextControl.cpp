#include "CustomLogTextControl.h"

CustomLogTextControl::CustomLogTextControl(wxTextCtrl * textControl)
	: wxLogTextCtrl(textControl)
	, m_textControl(textControl) { }

CustomLogTextControl::~CustomLogTextControl() { }

void CustomLogTextControl::DoLogTextAtLevel(wxLogLevel level, const wxString & logMessage) {
	DoLogText(logMessage);

	if(m_textControl != nullptr) {
		m_textControl->ShowPosition(m_textControl->GetLastPosition());
	}
}
