#include "ModDownloadPanel.h"

#include "Game/GameVersion.h"
#include "Manager/SettingsManager.h"
#include "Mod/Mod.h"
#include "Mod/ModDownload.h"
#include "Mod/ModVersion.h"

#include "WXUtilities.h"

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

ModDownloadPanel::ModDownloadPanel(std::shared_ptr<ModDownload> modDownload, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Mod Download")
	, m_download(modDownload) {
	if(ModDownload::isValid(modDownload.get(), true)) {
		const Mod * mod = modDownload->getParentMod();
		std::shared_ptr<ModVersion> modVersion(modDownload->getModVersion());
		SettingsManager * settings = SettingsManager::getInstance();
		std::string downloadBaseURL(Utilities::joinPaths(settings->apiBaseURL, settings->remoteDownloadsDirectoryName, settings->remoteModDownloadsDirectoryName, modDownload->getSubfolder(), mod->getID()));

		int border = 5;

		wxFlexGridSizer * downloadInfoSizer = new wxFlexGridSizer(2, border, border);
		SetSizer(downloadInfoSizer);

		wxGenericHyperlinkCtrl * downloadHyperlink = WXUtilities::createHyperlink(this, wxID_ANY, modDownload->getFileName(), Utilities::joinPaths(downloadBaseURL, modDownload->getFileName()), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxHL_ALIGN_LEFT | wxHL_CONTEXTMENU, "Mod Download");
		downloadHyperlink->Bind(wxEVT_HYPERLINK, &ModDownloadPanel::onModDownloadHyperlinkClicked, this);
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

		descriptionStringStream << " - ";

		descriptionStringStream << Utilities::fileSizeToString(modDownload->getFileSize());

		wxStaticText * downloadDescription = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
		// Note: For some reason ampersands are not rendered if you assign the text contents upon creation rather than through the set label text function
		downloadDescription->SetLabelText(wxString::FromUTF8(descriptionStringStream.str()));
		downloadInfoSizer->Add(downloadDescription, 1, wxEXPAND | wxALL);
	}
}

ModDownloadPanel::~ModDownloadPanel() { }

void ModDownloadPanel::onModDownloadHyperlinkClicked(wxHyperlinkEvent & event) {
	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		const Mod * mod = m_download->getParentMod();

		std::map<std::string, std::any> properties;
		properties["modID"] = mod->getID();
		properties["modName"] = mod->getName();
		properties["modVersion"] = m_download->getVersion();
		properties["modVersionType"] = m_download->getVersionType();
		properties["gameVersionID"] = m_download->getGameVersionID();
		properties["type"] = m_download->getType();
		properties["fileName"] = m_download->getFileName();
		properties["fileSize"] = m_download->getFileSize();
		properties["sha1"] = m_download->getSHA1();
		properties["url"] = std::string(event.GetURL().mb_str());

		if(m_download->hasSpecial()) {
			properties["special"] = m_download->getSpecial();
		}

		if(m_download->hasMultipleParts()) {
			properties["partNumber"] = m_download->getPartNumber();
			properties["numberOfParts"] = m_download->getPartCount();
		}

		SegmentAnalytics::getInstance()->track("Mod Download Hyperlink Clicked", properties);
	}
}
