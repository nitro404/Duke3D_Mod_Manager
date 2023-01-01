#include "CustomLogTextControl.h"

CustomLogTextControl::CustomLogTextControl(wxTextCtrl * textControl)
	: wxLogTextCtrl(textControl) { }

CustomLogTextControl::~CustomLogTextControl() { }

void CustomLogTextControl::DoLogTextAtLevel(wxLogLevel level, const wxString & logMessage) {
	DoLogText(logMessage);
}
