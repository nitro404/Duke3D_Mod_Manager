#include "ModDependenciesPanel.h"

#include "ModDependencyPanel.h"
#include "Mod/Mod.h"
#include "Mod/ModCollection.h"
#include "Mod/ModDependency.h"
#include "Mod/ModVersionType.h"

ModDependenciesPanel::ModDependenciesPanel(std::shared_ptr<ModCollection> mods, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Mod Dependencies")
	, m_mods(mods)
	, m_dependenciesPanelSizer(nullptr) {
	int border = 5;

	m_dependenciesPanelSizer = new wxFlexGridSizer(1, border, border);
	SetSizer(m_dependenciesPanelSizer);
}

ModDependenciesPanel::~ModDependenciesPanel() {
	m_modDependencyPanelConnections.disconnect();
}

void ModDependenciesPanel::setModVersionType(std::shared_ptr<ModVersionType> modVersionType) {
	if(m_modVersionType == modVersionType) {
		return;
	}

	m_modDependencyPanelConnections.disconnect();
	m_modVersionType = modVersionType;

	DestroyChildren();
	m_dependencyPanels.clear();

	if(!ModVersionType::isValid(m_modVersionType.get(), true)) {
		return;
	}

	for(size_t i = 0; i < m_modVersionType->numberOfDependencies(); i++) {
		std::shared_ptr<ModDependency> dependency(m_modVersionType->getDependency(i));

		ModDependencyPanel * modDependencyPanel = new ModDependencyPanel(m_mods->getModVersionTypeFromDependency(*dependency), this);
		m_modDependencyPanelConnections.addConnection(modDependencyPanel->modVersionTypeSelectionRequested.connect(std::bind(&ModDependenciesPanel::onModVersionTypeSelectionRequested, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
		m_dependenciesPanelSizer->Add(modDependencyPanel, 1, wxEXPAND | wxALL);
		m_dependencyPanels.push_back(modDependencyPanel);
	}

	Layout();
}

void ModDependenciesPanel::onModVersionTypeSelectionRequested(const std::string & modID, const std::string & modVersion, const std::string & modVersionType) {
	modVersionTypeSelectionRequested(modID, modVersion, modVersionType);
}
