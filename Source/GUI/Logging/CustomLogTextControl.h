#ifndef _CUSTOM_LOG_TEXT_CONTROL_H_
#define _CUSTOM_LOG_TEXT_CONTROL_H_

#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/log.h>
#include <wx/string.h>

class CustomLogTextControl final : public wxLogTextCtrl {
public:
	CustomLogTextControl(wxTextCtrl * textControl);
	virtual ~CustomLogTextControl();

	virtual void DoLogTextAtLevel(wxLogLevel level, const wxString & message) override;

	CustomLogTextControl(const CustomLogTextControl &) = delete;
	const CustomLogTextControl & operator = (const CustomLogTextControl &) = delete;
};

#endif // _CUSTOM_LOG_TEXT_CONTROL_H_
