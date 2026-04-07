#include <wx/log.h>

class PreformattedLogFormatter final : public wxLogFormatter {
public:
	PreformattedLogFormatter();
	~PreformattedLogFormatter() override;

private:
	virtual wxString Format(wxLogLevel level, const wxString & logMessage, const wxLogRecordInfo & info) const override;
};
