#include "WXUtilities.h"

namespace WXUtilities {

	wxPoint createWXPoint(const Point & point) {
		return wxPoint(point.x, point.y);
	}

	Point createPoint(const wxPoint & point) {
		return Point(point.x, point.y);
	}

	wxSize createWXSize(const Dimension & dimension) {
		return wxSize(dimension.w, dimension.h);
	}

	Dimension createDimension(const wxSize & size) {
		return Dimension(size.x, size.y);
	}

	wxColor createWXColour(const Colour & colour) {
		return wxColor(colour.r, colour.g, colour.b, colour.a);
	}

	Colour createColour(const wxColour & colour) {
		return Colour(static_cast<uint8_t>(colour.GetRed()), static_cast<uint8_t>(colour.GetGreen()), static_cast<uint8_t>(colour.GetBlue()), static_cast<uint8_t>(colour.GetAlpha()));
	}

	wxArrayString createItemWXArrayString(const std::vector<std::string> & items) {
		wxArrayString itemsArrayString;

		for(const std::string & item : items) {
			itemsArrayString.Add(item);
		}

		return itemsArrayString;
	}

}
