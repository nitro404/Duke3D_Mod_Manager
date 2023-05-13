#include "ModDownloadPanel.h"

#include "Manager/SettingsManager.h"
#include "Mod/Mod.h"
#include "Mod/ModDownload.h"

#include <Utilities/FileUtilities.h>

#include <wx/wrapsizer.h>

ModDownloadPanel::ModDownloadPanel(std::shared_ptr<ModDownload> modDownload, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Mod Download") {
	if(ModDownload::isValid(modDownload.get(), true)) {
		SettingsManager * settings = SettingsManager::getInstance();
		std::string downloadBaseURL(Utilities::joinPaths(settings->apiBaseURL, settings->remoteDownloadsDirectoryName, settings->remoteModDownloadsDirectoryName, modDownload->getSubfolder(), modDownload->getParentMod()->getID()));

		wxWrapSizer * downloadInfoSizer = new wxWrapSizer(wxHORIZONTAL);
		SetSizer(downloadInfoSizer);

		wxHyperlinkCtrl * downloadHyperlink = new wxHyperlinkCtrl(this, wxID_ANY, modDownload->getFileName(), Utilities::joinPaths(downloadBaseURL, modDownload->getFileName()), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Mod Download");
		downloadInfoSizer->Add(downloadHyperlink, 1, wxEXPAND | wxALL);
	}
}

ModDownloadPanel::~ModDownloadPanel() { }
