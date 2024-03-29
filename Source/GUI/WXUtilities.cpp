#include "WXUtilities.h"

#include "Hyperlink.h"

namespace WXUtilities {

	wxPoint createWXPoint(const Point2D & point) {
		return wxPoint(point.x, point.y);
	}

	Point2D createPoint(const wxPoint & point) {
		return Point2D(point.x, point.y);
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
		return Colour(static_cast<uint8_t>(colour.Red()), static_cast<uint8_t>(colour.Green()), static_cast<uint8_t>(colour.Blue()), static_cast<uint8_t>(colour.Alpha()));
	}

	wxArrayString createItemWXArrayString(const std::vector<std::string> & items) {
		wxArrayString itemsArrayString;

		for(const std::string & item : items) {
			itemsArrayString.Add(wxString::FromUTF8(item));
		}

		return itemsArrayString;
	}

	wxGenericHyperlinkCtrl * createHyperlink(wxWindow * parent, wxWindowID id, const wxString & label, const wxString & url, const wxPoint & position, const wxSize & size, long style , const wxString & name) {
		// Note: Generic hyperlinks assert if both the label and URL are empty, so a non-empty label must be forced to avoid assertion failures.
		wxGenericHyperlinkCtrl * hyperlink = new Hyperlink(parent, id, label.empty() ? " " : label, url, position, size, style, name);
		hyperlink->SetNormalColour(createWXColour(Colour::BLUE));
		hyperlink->SetVisitedColour(createWXColour(Colour::BLUE));
		hyperlink->SetHoverColour(createWXColour(Colour::ROYAL_BLUE));

		return hyperlink;
	}

	wxGenericHyperlinkCtrl * createDeepLink(wxWindow * parent, wxWindowID id, const wxString & label, const wxString & url, const wxPoint & position, const wxSize & size, long style, const wxString & name) {
		// Note: Generic hyperlinks assert if both the label and URL are empty, so a non-empty label must be forced to avoid assertion failures.
		wxGenericHyperlinkCtrl * deepLink = new Hyperlink(parent, id, label.empty() ? " " : label, url, position, size, style, name);
		deepLink->SetNormalColour(createWXColour(Colour::DARK_GREEN));
		deepLink->SetVisitedColour(createWXColour(Colour::DARK_GREEN));
		deepLink->SetHoverColour(createWXColour(Colour::FOREST_GREEN));

		return deepLink;
	}

	void setButtonEnabled(wxButton * button, bool enabled) {
		if(button == nullptr) {
			return;
		}

		if(enabled) {
			button->Enable();
		}
		else {
			button->Disable();
		}
	}

	void setTextControlEnabled(wxTextCtrl * textControl, bool enabled) {
		if(textControl == nullptr) {
			return;
		}

		if(enabled) {
			textControl->Enable();
		}
		else {
			textControl->Disable();
		}
	}

	wxLogLevel spdLogLevelToWXLogLevel(spdlog::level::level_enum logLevel) {
		switch(logLevel) {
			case spdlog::level::level_enum::trace:
				return wxLOG_Trace;
			case spdlog::level::level_enum::debug:
				return wxLOG_Debug;
			case spdlog::level::level_enum::info:
				return wxLOG_Message;
			case spdlog::level::level_enum::warn:
				return wxLOG_Warning;
			case spdlog::level::level_enum::err:
				return wxLOG_Error;
			case spdlog::level::level_enum::critical:
				return wxLOG_FatalError;
			case spdlog::level::level_enum::off:
			case spdlog::level::level_enum::n_levels: {
				break;
			}
		}

		return wxLOG_Info;
	}

	spdlog::level::level_enum wxLogLevelToSPDLogLevel(wxLogLevel logLevel) {
		switch(logLevel) {
			case wxLOG_FatalError:
				return spdlog::level::level_enum::critical;
			case wxLOG_Error:
				return spdlog::level::level_enum::err;
			case wxLOG_Warning:
				return spdlog::level::level_enum::warn;
			case wxLOG_Message:
				return spdlog::level::level_enum::info;
			case wxLOG_Status:
				return spdlog::level::level_enum::info;
			case wxLOG_Info:
				return spdlog::level::level_enum::err;
			case wxLOG_Debug:
				return spdlog::level::level_enum::debug;
			case wxLOG_Trace:
				return spdlog::level::level_enum::trace;
			case wxLOG_Progress:
			case wxLOG_User:
			case wxLOG_Max: {
				break;
			}

		}

		return spdlog::level::level_enum::info;
	}

	std::string logLevelToString(wxLogLevel logLevel) {
		switch(logLevel) {
			case wxLOG_FatalError:
				return "Fatal Error";
			case wxLOG_Error:
				return "Error";
			case wxLOG_Warning:
				return "Warning";
			case wxLOG_Message:
				return "Message";
			case wxLOG_Status:
				return "Status";
			case wxLOG_Info:
				return "Information";
			case wxLOG_Debug:
				return "Debug";
			case wxLOG_Trace:
				return "Trace";
			case wxLOG_Progress:
				return "Progress";
			case wxLOG_User:
			case wxLOG_Max: {
				break;
			}

		}

		return "Unknown";
	}

}
