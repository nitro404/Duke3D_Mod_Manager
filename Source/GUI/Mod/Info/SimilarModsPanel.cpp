#include "SimilarModsPanel.h"

#include "../../WXUtilities.h"
#include "Manager/SettingsManager.h"
#include "Mod/Mod.h"
#include "Mod/ModCollection.h"

#include <Analytics/Segment/SegmentAnalytics.h>

#include <spdlog/spdlog.h>

#include <any>
#include <map>
#include <string>

SimilarModsPanel::SimilarModsPanel(std::shared_ptr<ModCollection> mods, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Similar Mods")
	, m_similarModsPanelSizer(nullptr) {
	int border = 5;

	m_similarModsPanelSizer = new wxFlexGridSizer(1, border, border);
	SetSizer(m_similarModsPanelSizer);

	setMods(mods);
}

SimilarModsPanel::~SimilarModsPanel() { }

bool SimilarModsPanel::setMods(std::shared_ptr<ModCollection> mods) {
	if(!ModCollection::isValid(mods.get(), true)) {
		return false;
	}

	m_mods = mods;

	return true;
}

bool SimilarModsPanel::setMod(std::shared_ptr<Mod> mod) {
	if(m_mod == mod) {
		return true;
	}

	m_mod = mod;

	if(!Mod::isValid(m_mod.get(), true) || m_mods == nullptr) {
		return false;
	}

	DestroyChildren();
	m_similarModDeepLinks.clear();

	for(size_t i = 0; i < m_mod->numberOfSimilarMods(); i++) {
		const std::string & similarModID = m_mod->getSimilarMod(i);
		std::shared_ptr<Mod> similarMod(m_mods->getModWithID(similarModID));

		if(similarMod == nullptr) {
			spdlog::error("Failed to find mod similar to '{}' with ID: '{}'.", m_mod->getName(), similarModID);
			continue;
		}

		wxGenericHyperlinkCtrl * similarModDeepLink = WXUtilities::createDeepLink(this, wxID_ANY, similarMod->getName(), similarModID, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT, "Similar Mod");
		similarModDeepLink->Bind(wxEVT_HYPERLINK, &SimilarModsPanel::onSimilarModDeepLinkClicked, this);
		m_similarModsPanelSizer->Add(similarModDeepLink, 1, wxEXPAND | wxALL);
		m_similarModDeepLinks.push_back(similarModDeepLink);
	}

	Layout();

	return true;
}

void SimilarModsPanel::onSimilarModDeepLinkClicked(wxHyperlinkEvent & event) {
	event.Skip(false);

	std::string modID(event.GetURL().mb_str());

	modSelectionRequested(modID);

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["modID"] = modID;

		SegmentAnalytics::getInstance()->track("Similar Mod Deep Link Clicked", properties);
	}
}
