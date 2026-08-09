#include <wx/log.h>

class PreformattedLogFormatter final : public wxLogFormatter {
public:
	PreformattedLogFormatter();
	~PreformattedLogFormatter() override;

private:
	// wxLogFormatter Virtuals
	wxString Format(wxLogLevel level, const wxString & logMessage, const wxLogRecordInfo & info) const override;
};
