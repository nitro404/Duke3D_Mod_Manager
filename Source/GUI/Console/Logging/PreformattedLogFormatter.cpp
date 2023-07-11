#include "PreformattedLogFormatter.h"

PreformattedLogFormatter::PreformattedLogFormatter() { }

PreformattedLogFormatter::~PreformattedLogFormatter() { }

wxString PreformattedLogFormatter::Format(wxLogLevel level, const wxString & logMessage, const wxLogRecordInfo & info) const {
	return wxString::Format("%s", logMessage);
}
