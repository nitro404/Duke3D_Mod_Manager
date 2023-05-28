#ifndef _WX_UTILITIES_H_
#define _WX_UTILITIES_H_

#include <Colour.h>
#include <Dimension.h>
#include <Point2D.h>
#include <Utilities/StringUtilities.h>

#include <magic_enum.hpp>
#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

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
	void setButtonEnabled(wxButton * button, bool enabled);
	void setTextControlEnabled(wxTextCtrl * textControl, bool enabled);

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
