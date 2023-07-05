#ifndef _WX_UTILITIES_H_
#define _WX_UTILITIES_H_

#include <Colour.h>
#include <Dimension.h>
#include <Point2D.h>
#include <Utilities/StringUtilities.h>

#include <magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/hyperlink.h>
#include <wx/generic/hyperlink.h>

#include <string>
#include <vector>

namespace WXUtilities {

	wxPoint createWXPoint(const Point2D & point);
	Point2D createPoint(const wxPoint & point);
	wxSize createWXSize(const Dimension & dimension);
	Dimension createDimension(const wxSize & size);
	wxColour createWXColour(const Colour & colour);
	Colour createColour(const wxColour & colour);
	wxArrayString createItemWXArrayString(const std::vector<std::string> & items);
	template <typename E>
	wxArrayString createEnumWXArrayString(const std::vector<E> & disabledEnumValues = {});
	wxGenericHyperlinkCtrl * createHyperlink(wxWindow * parent, wxWindowID id, const wxString & label, const wxString & url, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxHL_DEFAULT_STYLE, const wxString & name = wxASCII_STR(wxHyperlinkCtrlNameStr));
	wxGenericHyperlinkCtrl * createDeepLink(wxWindow * parent, wxWindowID id, const wxString & label, const wxString & url, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxNO_BORDER | wxHL_ALIGN_CENTRE, const wxString & name = wxASCII_STR(wxHyperlinkCtrlNameStr));
	void setButtonEnabled(wxButton * button, bool enabled);
	void setTextControlEnabled(wxTextCtrl * textControl, bool enabled);
	wxLogLevel spdLogLevelToWXLogLevel(spdlog::level::level_enum logLevel);
	spdlog::level::level_enum wxLogLevelToSPDLogLevel(wxLogLevel logLevel);
	std::string logLevelToString(wxLogLevel logLevel);

	template <typename E>
	wxArrayString createEnumWXArrayString(const std::vector<E> & disabledEnumValues) {
		wxArrayString enumArrayString;
		constexpr auto & enumValues = magic_enum::enum_values<E>();

		for(const E enumValue : enumValues) {
			if(std::find(disabledEnumValues.cbegin(), disabledEnumValues.cend(), enumValue) != disabledEnumValues.cend()) {
				continue;
			}

			enumArrayString.Add(Utilities::toCapitalCase(magic_enum::enum_name(enumValue)));
		}

		return enumArrayString;
	}
}

#endif // _WX_UTILITIES_H_
