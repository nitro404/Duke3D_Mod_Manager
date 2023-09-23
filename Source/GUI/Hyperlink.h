#ifndef _HYPERLINK_H_
#define _HYPERLINK_H_

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/hyperlink.h>
#include <wx/generic/hyperlink.h>

class Hyperlink final : public wxGenericHyperlinkCtrl {
public:
	Hyperlink();
	Hyperlink(wxWindow * parent, wxWindowID id, const wxString & label, const wxString & url, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxHL_DEFAULT_STYLE, const wxString & name = wxASCII_STR(wxHyperlinkCtrlNameStr));

	virtual void SetLabel(const wxString & label) override;

};

#endif // _HYPERLINK_H_
