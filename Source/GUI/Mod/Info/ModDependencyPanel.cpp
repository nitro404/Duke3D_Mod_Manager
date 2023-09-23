#include "ModDependencyPanel.h"

#include "../../WXUtilities.h"
#include "Game/GameVersion.h"
#include "Manager/SettingsManager.h"
#include "Mod/Mod.h"
#include "Mod/ModVersionType.h"
#include "Mod/ModVersion.h"

#include <Analytics/Segment/SegmentAnalytics.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <wx/wrapsizer.h>

#include <any>
#include <map>
#include <sstream>
#include <string>
#include <vector>

ModDependencyPanel::ModDependencyPanel(std::shared_ptr<ModVersionType> modVersionType, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Mod Dependency")
	, m_modVersionType(modVersionType) {
	if(ModVersionType::isValid(m_modVersionType.get(), true)) {
		int border = 5;

		wxFlexGridSizer * dependencyInfoSizer = new wxFlexGridSizer(2, border, border);
		SetSizer(dependencyInfoSizer);

		wxGenericHyperlinkCtrl * dependencyDeepLink = WXUtilities::createDeepLink(this, wxID_ANY, m_modVersionType->getFullName(), m_modVersionType->getParentMod()->getID() + "/" + m_modVersionType->getParentModVersion()->getVersion() + "/" + m_modVersionType->getType(), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT, "Mod Dependency");
		dependencyDeepLink->Bind(wxEVT_HYPERLINK, &ModDependencyPanel::onModDependencyDeepLinkClicked, this);
		dependencyInfoSizer->Add(dependencyDeepLink, 1, wxEXPAND | wxALL);
	}
}

ModDependencyPanel::~ModDependencyPanel() { }

void ModDependencyPanel::onModDependencyDeepLinkClicked(wxHyperlinkEvent & event) {
	event.Skip(false);

	modVersionTypeSelectionRequested(m_modVersionType->getParentMod()->getID(), m_modVersionType->getParentModVersion()->getVersion(), m_modVersionType->getType());

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["modID"] = m_modVersionType->getParentMod()->getID();
		properties["modVersion"] = m_modVersionType->getParentModVersion()->getVersion();
		properties["modVersionType"] = m_modVersionType->getType();

		SegmentAnalytics::getInstance()->track("Mod Dependency Deep Link Clicked", properties);
	}
}
