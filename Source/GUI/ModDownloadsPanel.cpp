#include "ModDownloadsPanel.h"

#include "ModDownloadPanel.h"
#include "Mod/Mod.h"
#include "Mod/ModDownload.h"

ModDownloadsPanel::ModDownloadsPanel(wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Mod Downloads")
	, m_downloadsPanelSizer(nullptr) {
	int border = 5;

	m_downloadsPanelSizer = new wxFlexGridSizer(1, border, border);
	SetSizer(m_downloadsPanelSizer);
}

ModDownloadsPanel::~ModDownloadsPanel() { }

void ModDownloadsPanel::setMod(std::shared_ptr<Mod> mod) {
	if(m_mod == mod) {
		return;
	}

	m_mod = mod;

	if(!Mod::isValid(m_mod.get(), true)) {
		return;
	}

	DestroyChildren();
	m_downloadPanels.clear();

	for(size_t i = 0; i < m_mod->numberOfDownloads(); i++) {
		std::shared_ptr<ModDownload> modDownload(m_mod->getDownload(i));

		if(modDownload->isModManagerFiles()) {
			continue;
		}

		ModDownloadPanel * modDownloadPanel = new ModDownloadPanel(modDownload, this);
		m_downloadsPanelSizer->Add(modDownloadPanel, 1, wxEXPAND | wxALL);
		m_downloadPanels.push_back(modDownloadPanel);
	}

	Layout();
}
