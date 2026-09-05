#include "Hyperlink.h"

Hyperlink::Hyperlink()
	: wxGenericHyperlinkCtrl() {
	wxASSERT(wxIsMainThread());
}

Hyperlink::Hyperlink(wxWindow * parent, wxWindowID id, const wxString & label, const wxString & url, const wxPoint & position, const wxSize & size, long style, const wxString & name)
	: wxGenericHyperlinkCtrl(parent, id, label, url, position, size, style, name) {
	wxASSERT(wxIsMainThread());
}

void Hyperlink::SetLabel(const wxString & label) {
	wxASSERT(wxIsMainThread());

	wxGenericHyperlinkCtrl::SetLabel(label);

	// Force generic hyperlinks to re-paint when changing the label to avoid rendering issues
	Refresh();
}
