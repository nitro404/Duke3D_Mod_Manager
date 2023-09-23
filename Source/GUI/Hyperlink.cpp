#include "Hyperlink.h"

Hyperlink::Hyperlink()
	: wxGenericHyperlinkCtrl() { }

Hyperlink::Hyperlink(wxWindow * parent, wxWindowID id, const wxString & label, const wxString & url, const wxPoint & position, const wxSize & size, long style, const wxString & name)
	: wxGenericHyperlinkCtrl(parent, id, label, url, position, size, style, name) { }

void Hyperlink::SetLabel(const wxString & label) {
	wxGenericHyperlinkCtrl::SetLabel(label);

	// Force generic hyperlinks to re-paint when changing the label to avoid rendering issues
	Refresh();
}
