#include "ModDownloadPanel.h"

#include "Game/GameVersion.h"
#include "Manager/SettingsManager.h"
#include "Mod/Mod.h"
#include "Mod/ModDownload.h"
#include "Mod/ModVersion.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <wx/wrapsizer.h>

#include <sstream>
#include <vector>

ModDownloadPanel::ModDownloadPanel(std::shared_ptr<ModDownload> modDownload, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Mod Download") {
	if(ModDownload::isValid(modDownload.get(), true)) {
		const Mod * mod = modDownload->getParentMod();
		std::shared_ptr<ModVersion> modVersion(modDownload->getModVersion());
		SettingsManager * settings = SettingsManager::getInstance();
		std::string downloadBaseURL(Utilities::joinPaths(settings->apiBaseURL, settings->remoteDownloadsDirectoryName, settings->remoteModDownloadsDirectoryName, modDownload->getSubfolder(), mod->getID()));

		int border = 5;

		wxFlexGridSizer * downloadInfoSizer = new wxFlexGridSizer(2, border, border);
		SetSizer(downloadInfoSizer);

		wxHyperlinkCtrl * downloadHyperlink = new wxHyperlinkCtrl(this, wxID_ANY, modDownload->getFileName(), Utilities::joinPaths(downloadBaseURL, modDownload->getFileName()), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Mod Download");
		downloadInfoSizer->Add(downloadHyperlink, 1, wxEXPAND | wxALL);

		std::stringstream descriptionStringStream;
		descriptionStringStream << "- ";

		if(modVersion != nullptr) {
			descriptionStringStream << modVersion->getFullName();
		}
		else {
			descriptionStringStream << mod->getName();
		}

		if(modDownload->hasSpecial()) {
			descriptionStringStream << " ";
			descriptionStringStream << modDownload->getSpecial();
		}

		if(modDownload->hasMultipleParts()) {
			descriptionStringStream << fmt::format(" ({} of {})", modDownload->getPartNumber(), modDownload->getPartCount());
		}

		if(!modDownload->isOriginalFiles()) {
			descriptionStringStream << fmt::format(" ({})", modDownload->getType());
		}

		if(modDownload->hasGameVersionID()) {
			std::vector<const GameVersion *>::const_iterator gameVersionIterator(std::find_if(GameVersion::DEFAULT_GAME_VERSIONS.cbegin(), GameVersion::DEFAULT_GAME_VERSIONS.cend(), [modDownload](const GameVersion * gameVersion) {
				return Utilities::areStringsEqualIgnoreCase(modDownload->getGameVersionID(), gameVersion->getID());
			}));

			descriptionStringStream << fmt::format(" ({})", gameVersionIterator != GameVersion::DEFAULT_GAME_VERSIONS.cend() ? (*gameVersionIterator)->getLongName() : modDownload->getGameVersionID());
		}

		if(modDownload->isConverted()) {
			descriptionStringStream << " (Converted)";
		}

		if(modDownload->isCorrupted()) {
			descriptionStringStream << " (Corrupted)";
		}

		if(modDownload->isRepaired()) {
			descriptionStringStream << " (Repaired)";
		}

		wxStaticText * downloadDescription = new wxStaticText(this, wxID_ANY, wxString::FromUTF8(descriptionStringStream.str()), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
		downloadInfoSizer->Add(downloadDescription, 1, wxEXPAND | wxALL);
	}
}

ModDownloadPanel::~ModDownloadPanel() { }
