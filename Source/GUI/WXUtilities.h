#ifndef _WX_UTILITIES_H_
#define _WX_UTILITIES_H_

#include <Colour.h>
#include <Dimension.h>
#include <Point.h>
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

	wxPoint createWXPoint(const Point & point);
	Point createPoint(const wxPoint & point);
	wxSize createWXSize(const Dimension & dimension);
	Dimension createDimension(const wxSize & size);
	wxColour createWXColour(const Colour & colour);
	Colour createColour(const wxColour & colour);
	wxArrayString createItemWXArrayString(const std::vector<std::string> & items);
	template <typename E>
	wxArrayString createEnumWXArrayString();

	template <typename E>
	wxArrayString createEnumWXArrayString() {
		wxArrayString enumArrayString;
		constexpr auto enumNames = magic_enum::enum_names<E>();

		for(const std::string_view enumName : enumNames) {
			enumArrayString.Add(Utilities::toCapitalCase(enumName));
		}

		return enumArrayString;
	}
}

#endif // _WX_UTILITIES_H_
