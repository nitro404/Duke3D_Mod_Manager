#include "RelatedModsPanel.h"

#include "Mod/Mod.h"
#include "Mod/ModCollection.h"
#include "WXUtilities.h"

#include <spdlog/spdlog.h>

#include <string>

RelatedModsPanel::RelatedModsPanel(std::shared_ptr<ModCollection> mods, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Mod Downloads")
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
	m_relatedModHyperlinks.clear();

	for(size_t i = 0; i < m_mod->numberOfRelatedMods(); i++) {
		const std::string & relatedModID = m_mod->getRelatedMod(i);
		std::shared_ptr<Mod> relatedMod(m_mods->getModWithID(relatedModID));

		if(relatedMod == nullptr) {
			spdlog::error("Failed to find mod related to '{}' with ID: '{}'.", m_mod->getName(), relatedModID);
			continue;
		}

		wxGenericHyperlinkCtrl * relatedModHyperlink = WXUtilities::createDeepLink(this, wxID_ANY, relatedMod->getName(), relatedModID, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT, "Related Mod");
		relatedModHyperlink->Bind(wxEVT_HYPERLINK, &RelatedModsPanel::onRelatedModHyperlinkClicked, this);
		m_relatedModsPanelSizer->Add(relatedModHyperlink, 1, wxEXPAND | wxALL);
		m_relatedModHyperlinks.push_back(relatedModHyperlink);
	}

	Layout();

	return true;
}

void RelatedModsPanel::onRelatedModHyperlinkClicked(wxHyperlinkEvent & event) {
	event.Skip(false);

	modSelectionRequested(std::string(event.GetURL().mb_str()));
}
