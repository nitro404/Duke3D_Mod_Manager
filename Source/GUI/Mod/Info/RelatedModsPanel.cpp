#include "RelatedModsPanel.h"

#include "../../WXUtilities.h"
#include "Manager/SettingsManager.h"
#include "Mod/Mod.h"
#include "Mod/ModCollection.h"

#include <Analytics/Segment/SegmentAnalytics.h>

#include <spdlog/spdlog.h>

#include <any>
#include <map>
#include <string>

RelatedModsPanel::RelatedModsPanel(std::shared_ptr<ModCollection> mods, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Related Mods")
	, m_relatedModsPanelSizer(nullptr) {
	int border = 5;

	m_relatedModsPanelSizer = new wxFlexGridSizer(1, border, border);
	SetSizer(m_relatedModsPanelSizer);

	setMods(mods);
}

RelatedModsPanel::~RelatedModsPanel() { }

bool RelatedModsPanel::setMods(std::shared_ptr<ModCollection> mods) {
	if(!ModCollection::isValid(mods.get(), true)) {
		return false;
	}

	m_mods = mods;

	return true;
}

bool RelatedModsPanel::setMod(std::shared_ptr<Mod> mod) {
	if(m_mod == mod) {
		return true;
	}

	m_mod = mod;

	if(!Mod::isValid(m_mod.get(), true) || m_mods == nullptr) {
		return false;
	}

	DestroyChildren();
	m_relatedModDeepLinks.clear();

	for(size_t i = 0; i < m_mod->numberOfRelatedMods(); i++) {
		const std::string & relatedModID = m_mod->getRelatedMod(i);
		std::shared_ptr<Mod> relatedMod(m_mods->getModWithID(relatedModID));

		if(relatedMod == nullptr) {
			spdlog::error("Failed to find mod related to '{}' with ID: '{}'.", m_mod->getName(), relatedModID);
			continue;
		}

		wxGenericHyperlinkCtrl * relatedModDeepLink = WXUtilities::createDeepLink(this, wxID_ANY, relatedMod->getName(), relatedModID, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT, "Related Mod");
		relatedModDeepLink->Bind(wxEVT_HYPERLINK, &RelatedModsPanel::onRelatedModDeepLinkClicked, this);
		m_relatedModsPanelSizer->Add(relatedModDeepLink, 1, wxEXPAND | wxALL);
		m_relatedModDeepLinks.push_back(relatedModDeepLink);
	}

	Layout();

	return true;
}

void RelatedModsPanel::onRelatedModDeepLinkClicked(wxHyperlinkEvent & event) {
	event.Skip(false);

	std::string modID(event.GetURL().mb_str());

	modSelectionRequested(modID);

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["modID"] = modID;

		SegmentAnalytics::getInstance()->track("Related Mod Deep Link Clicked", properties);
	}
}
