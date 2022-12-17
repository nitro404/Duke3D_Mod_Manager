#include "GameManager.h"

#include "GameDownload.h"
#include "GameDownloadCollection.h"
#include "GameDownloadFile.h"
#include "GameDownloadVersion.h"
#include "GameLocator.h"
#include "Game/GameVersion.h"
#include "Game/NoCDCracker.h"
#include "Group/Group.h"
#include "Manager/SettingsManager.h"

#include <Archive/ArchiveFactoryRegistry.h>
#include <GitHub/GitHubService.h>
#include <Network/HTTPService.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/TidyHTMLUtilities.h>
#include <Utilities/TinyXML2Utilities.h>

#include <magic_enum.hpp>
#include <rapidjson/document.h>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <array>
#include <filesystem>
#include <optional>
#include <sstream>
#include <vector>

using namespace std::chrono_literals;

static const std::string REGULAR_VERSION_GAME_FILES_FILE_NAME("Duke_Nukem_3D_-_Regular_Version_Game_Files.zip");
static const std::string REGULAR_VERSION_GROUP_FILE_NAME("Duke_Nukem_3D_-_Regular_Version_Group.zip");
static const std::string ATOMIC_EDITION_GAME_FILES_FILE_NAME("Duke_Nukem_3D_-_Atomic_Edition_Game_Files.zip");
static const std::string ATOMIC_EDITION_GROUP_FILE_NAME("Duke_Nukem_3D_-_Atomic_Edition_Group.zip");

static const std::string REGULAR_VERSION_FALLBACK_DOWNLOAD_URL("https://archive.org/download/DUKE3D_DOS/DUKE3D.zip"); // https://archive.org/details/DUKE3D_DOS
static const std::string ATOMIC_EDITION_FALLBACK_DOWNLOAD_URL("https://archive.org/download/duke-3d-atomic3datomictweak/Duke3d_ATOMIC.rar"); // https://archive.org/details/duke-3d-atomic3datomictweak

static const std::string REGULAR_VERSION_FALLBACK_DOWNLOAD_SHA1("9bc975193ccc4738a31fe2dc6958f0b34135b9ae");
static const std::string ATOMIC_EDITION_FALLBACK_DOWNLOAD_SHA1("ac2df3fb75f84584cadc82db00ef46ab21ef6a95");

static const std::string JFDUKE32_DOWNLOAD_PAGE_URL("http://www.jonof.id.au/jfduke3d");
static const std::string EDUKE32_DOWNLOAD_PAGE_URL("https://dukeworld.com/eduke32/synthesis/latest");
static const std::string RED_NUKEM_DOWNLOAD_PAGE_URL("https://lerppu.net/wannabethesis");

struct GameFileInformation {
	std::string fileName;
	std::vector<std::string> hashes;
	bool required = true;
};

static const std::array<GameFileInformation, 12> REGULAR_VERSION_GAME_FILE_INFO_LIST = {
	GameFileInformation{ "COMMIT.EXE",   { "fbb3d9ae08cd451b2a4cfc75b92de6dedc82d4b2" } },
	GameFileInformation{ "DEMO1.DMO",    { "c55a1a4635738da4e91b49475604608a04c03fcd" } },
	GameFileInformation{ "DEMO2.DMO",    { "c33e21c845584b8703264e032dd7808871b04286" } },
	GameFileInformation{ "DN3DHELP.EXE", { "eceefd4f9fa868ced33fcdd68673f1fb9cd8bfc5" } },
	GameFileInformation{ "DUKE.RTS",     { "738c7f5fd0c8b57ee2e87ae7a97bf8e21a821d07" } },
	GameFileInformation{ "DUKE3D.EXE",   { "a64cc5b61cba728427cfcc537aa2f74438ea4c65" } },
	GameFileInformation{ "DUKE3D.GRP",   { Group::DUKE_NUKEM_3D_REGULAR_VERSION_GROUP_SHA1_FILE_HASH } },
	GameFileInformation{ "MODEM.PCK",    { "c0353741d28ded860d708f0915a27028fb47b9f3" } },
	GameFileInformation{ "SETMAIN.EXE",  { "e43d96ea4429ab6dd5aab0509bd8705223ba5b21" } },
	GameFileInformation{ "SETUP.EXE",    { "7110b4b66e5823527750dcda2a8df5939a978c9b" } },
	GameFileInformation{ "TENGAME.INI",  { "2b6e157bede128d48788ebb80f29f1e635dd20dd" }, false },
	GameFileInformation{ "ULTRAMID.INI", { "54a404652aecfddf73aea0c11326f9f95fdd1e25" } }
};

static const std::array<GameFileInformation, 11> ATOMIC_EDITION_GAME_FILE_INFO_LIST = {
	GameFileInformation{ "COMMIT.EXE",   { "8832f0fc61f7286b14cac0cd1ff0a56e179c5d6f" } },
	GameFileInformation{ "DN3DHELP.EXE", { "eeb5666d6e1b705e3684f8ed84ab5ac50b30a690" } },
	GameFileInformation{ "DUKE.RTS",     { "738c7f5fd0c8b57ee2e87ae7a97bf8e21a821d07" } },
	GameFileInformation{ "DUKE3D.EXE",   { "f0dc7f1ca810aa517fcad544a3bf5af623a3e44e", "a849e1e00ac58c0271498dd302d5c5f2819ab2e9" } },
	GameFileInformation{ "DUKE3D.GRP",   { Group::DUKE_NUKEM_3D_ATOMIC_EDITION_GROUP_SHA1_FILE_HASH, Group::DUKE_NUKEM_3D_WORLD_TOUR_GROUP_SHA1_FILE_HASH } },
	GameFileInformation{ "LICENSE.DOC",  { "ce1a1bb1afbd714bb96ec0c0d8e0b23a94f14c0b" } },
	GameFileInformation{ "MODEM.PCK",    { "7d88d2ae3a0fc21fcaaeb9cc5c1e72399c0fd0cb" } },
	GameFileInformation{ "SETMAIN.EXE",  { "40bd08600df2cd6328e69889b5325b72a123614e" } },
	GameFileInformation{ "SETUP.EXE",    { "861db7aa6dfc868b6a0b333b4cb091e276a18832" } },
	GameFileInformation{ "TENGAME.INI",  { "01a0f0e66fb05f5b2de72a8dd3ad19cdfa4ae323" }, false },
	GameFileInformation{ "ULTRAMID.INI", { "54a404652aecfddf73aea0c11326f9f95fdd1e25" } }
};

GameManager::GameManager()
	: m_initialized(false) { }

GameManager::~GameManager() { }

bool GameManager::isInitialized() const {
	return m_initialized;
}

bool GameManager::initialize(std::shared_ptr<GameVersionCollection> gameVersions) {
	if(m_initialized || !GameVersionCollection::isValid(gameVersions.get())) {
		return false;
	}

	if(!HTTPService::getInstance()->isInitialized()) {
		return false;
	}

	m_gameVersions = gameVersions;

	updateGameDownloadList();

	m_initialized = true;

	return true;
}

bool GameManager::updateGameDownloadList(bool force) const {
	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		return false;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	std::string gameListRemoteFilePath(Utilities::joinPaths(settings->remoteDownloadsDirectoryName, settings->remoteGameDownloadsDirectoryName, settings->remoteGamesListFileName));
	std::string gameListURL(Utilities::joinPaths(httpService->getBaseURL(), gameListRemoteFilePath));

	spdlog::info("Downloading Duke Nukem 3D game download list from: '{}'...", gameListURL);

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, gameListURL));

	if(!force && !m_gameListFileETag.empty()) {
		request->setIfNoneMatchETag(m_gameListFileETag);
	}

	std::shared_ptr<HTTPResponse> response(httpService->sendRequestAndWait(request));

	if(response->isFailure()) {
		spdlog::error("Failed to download Duke Nukem 3D game download list with error: {}", response->getErrorMessage());
		return false;
	}

	if(response->getStatusCode() == magic_enum::enum_integer(HTTPStatusCode::NotModified)) {
		spdlog::info("Duke Nukem 3D game download list is already up to date!");
		return true;
	}
	else if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to download Duke Nukem 3D game download list ({}{})!", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return false;
	}

	spdlog::info("Duke Nukem 3D game download list downloaded successfully after {} ms.", response->getRequestDuration().value().count());

	std::unique_ptr<rapidjson::Document> gameDownloadCollectionDocument(response->getBodyAsJSON());

	if(gameDownloadCollectionDocument == nullptr) {
		spdlog::error("Failed to parse game download collection JSON data.");
		return false;
	}

	std::unique_ptr<GameDownloadCollection> gameDownloads(GameDownloadCollection::parseFrom(*gameDownloadCollectionDocument));

	if(!GameDownloadCollection::isValid(gameDownloads.get())) {
		spdlog::error("Failed to parse game download collection from JSON data.");
		return false;
	}

	m_gameDownloads = std::move(gameDownloads);
	m_gameListFileETag = response->getETag();

	return true;
}

bool GameManager::isGameDownloadable(const std::string & gameName) {
	return gameName == GameVersion::ORIGINAL_REGULAR_VERSION.getName() ||
		   gameName == GameVersion::ORIGINAL_ATOMIC_EDITION.getName() ||
		   gameName == GameVersion::JFDUKE3D.getName() ||
		   gameName == GameVersion::EDUKE32.getName() ||
		   gameName == GameVersion::RAZE.getName() ||
		   gameName == GameVersion::RED_NUKEM.getName() ||
		   gameName == GameVersion::BELGIAN_CHOCOLATE_DUKE3D.getName();;
}

std::string GameManager::getGameDownloadURL(const std::string & gameName) {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	if(gameName == GameVersion::ORIGINAL_REGULAR_VERSION.getName()) {
		return Utilities::joinPaths(getRemoteGameDownloadsBaseURL(), REGULAR_VERSION_GAME_FILES_FILE_NAME);
	}
	else if(gameName == GameVersion::ORIGINAL_ATOMIC_EDITION.getName()) {
		return Utilities::joinPaths(getRemoteGameDownloadsBaseURL(), ATOMIC_EDITION_GAME_FILES_FILE_NAME);
	}

	DeviceInformationBridge * deviceInformationBridge = DeviceInformationBridge::getInstance();

	std::optional<DeviceInformationBridge::OperatingSystemType> optionalOperatingSystemType(deviceInformationBridge->getOperatingSystemType());

	if(!optionalOperatingSystemType.has_value()) {
		spdlog::error("Failed to determine operating system type.");
		return {};
	}

	std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> optionalOperatingSystemArchitectureType(deviceInformationBridge->getOperatingSystemArchitectureType());

	if(!optionalOperatingSystemArchitectureType.has_value()) {
		spdlog::error("Failed to determine operating system architecture type.");
		return {};
	}

	if(gameName == GameVersion::JFDUKE3D.getName()) {
		return getJFDuke3DDownloadURL(optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value());
	}
	else if(gameName == GameVersion::EDUKE32.getName()) {
		return getEDuke32DownloadURL(optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value());
	}
	else if(gameName == GameVersion::RAZE.getName()) {
		return getRazeDownloadURL(optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value());
	}
	else if(gameName == GameVersion::RED_NUKEM.getName()) {
		return getRedNukemDownloadURL();
	}
	else if(gameName == GameVersion::BELGIAN_CHOCOLATE_DUKE3D.getName()) {
		return getBelgianChocolateDuke3DDownloadURL();
	}

	return {};
}

std::string GameManager::getFallbackGameDownloadURL(const std::string & gameName) const {
	if(gameName == GameVersion::ORIGINAL_REGULAR_VERSION.getName()) {
		return REGULAR_VERSION_FALLBACK_DOWNLOAD_URL;
	}
	else if(gameName == GameVersion::ORIGINAL_ATOMIC_EDITION.getName()) {
		return ATOMIC_EDITION_FALLBACK_DOWNLOAD_URL;
	}

	if(!updateGameDownloadList()) {
		return {};
	}

	DeviceInformationBridge * deviceInformationBridge = DeviceInformationBridge::getInstance();

	std::optional<DeviceInformationBridge::OperatingSystemType> optionalOperatingSystemType(deviceInformationBridge->getOperatingSystemType());

	if(!optionalOperatingSystemType.has_value()) {
		spdlog::error("Failed to determine operating system type.");
		return {};
	}

	std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> optionalOperatingSystemArchitectureType(deviceInformationBridge->getOperatingSystemArchitectureType());

	if(!optionalOperatingSystemArchitectureType.has_value()) {
		spdlog::error("Failed to determine operating system architecture type.");
		return {};
	}

	std::shared_ptr<GameDownloadFile> gameDownloadFile(m_gameDownloads->getLatestGameDownloadFile(gameName, GameDownloadFile::Type::Game, optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value()));

	if(gameDownloadFile == nullptr) {
		switch(optionalOperatingSystemArchitectureType.value()) {
			case DeviceInformationBridge::OperatingSystemArchitectureType::x86: {
				spdlog::error("Could not find '{}' game file download information for '{}' ({}).", gameName, magic_enum::enum_name(optionalOperatingSystemType.value()), magic_enum::enum_name(optionalOperatingSystemArchitectureType.value()));
				return {};
			}
			case DeviceInformationBridge::OperatingSystemArchitectureType::x64: {
				gameDownloadFile = m_gameDownloads->getLatestGameDownloadFile(gameName, GameDownloadFile::Type::Game, optionalOperatingSystemType.value(), DeviceInformationBridge::OperatingSystemArchitectureType::x86);

				if(gameDownloadFile == nullptr) {
					spdlog::error("Could not find '{}' game file download information for '{}' ({} or {}).", gameName, magic_enum::enum_name(optionalOperatingSystemType.value()), magic_enum::enum_name(DeviceInformationBridge::OperatingSystemArchitectureType::x64), magic_enum::enum_name(DeviceInformationBridge::OperatingSystemArchitectureType::x86));
					return {};
				}

				break;
			}
		}
	}

	return Utilities::joinPaths(getRemoteGameDownloadsBaseURL(), gameDownloadFile->getFileName());
}

std::string GameManager::getRemoteGameDownloadsBaseURL() const {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->apiBaseURL.empty()) {
		spdlog::error("Missing API base URL setting.");
		return {};
	}

	if(settings->remoteDownloadsDirectoryName.empty()) {
		spdlog::error("Missing remote downloads directory name setting.");
		return {};
	}

	if(settings->remoteGameDownloadsDirectoryName.empty()) {
		spdlog::error("Missing remote game downloads directory name setting.");
		return {};
	}

	return Utilities::joinPaths(settings->apiBaseURL, settings->remoteDownloadsDirectoryName, settings->remoteGameDownloadsDirectoryName);
}

std::string GameManager::getGroupDownloadURL(const std::string & gameName) const {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	std::string_view groupFileName;

	if(gameName == GameVersion::ORIGINAL_REGULAR_VERSION.getName()) {
		groupFileName = REGULAR_VERSION_GROUP_FILE_NAME;
	}
	else if(gameName == GameVersion::ORIGINAL_ATOMIC_EDITION.getName()) {
		groupFileName = ATOMIC_EDITION_GROUP_FILE_NAME;
	}
	else {
		return {};
	}

	return Utilities::joinPaths(getRemoteGameDownloadsBaseURL(), groupFileName);
}

std::string GameManager::getFallbackGroupDownloadURL(const std::string & gameName) const {
	if(gameName == GameVersion::ORIGINAL_REGULAR_VERSION.getName()) {
		return REGULAR_VERSION_FALLBACK_DOWNLOAD_URL;
	}
	else if(gameName == GameVersion::ORIGINAL_ATOMIC_EDITION.getName()) {
		return ATOMIC_EDITION_FALLBACK_DOWNLOAD_URL;
	}

	return {};
}

std::string GameManager::getFallbackGroupDownloadSHA1(const std::string & gameName) const {
	if(gameName == GameVersion::ORIGINAL_REGULAR_VERSION.getName()) {
		return REGULAR_VERSION_FALLBACK_DOWNLOAD_SHA1;
	}
	else if(gameName == GameVersion::ORIGINAL_ATOMIC_EDITION.getName()) {
		return ATOMIC_EDITION_FALLBACK_DOWNLOAD_SHA1;
	}

	return {};
}

std::string GameManager::getJFDuke3DDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const {
	static const std::string WINDOWS_IDENTIFIER("win");
	static const std::string WINDOWS_X86_ARCHITECTURE_IDENTIFIER("x86");

	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	std::shared_ptr<GameVersion> jfDuke3DGameVersion(m_gameVersions->getGameVersion(GameVersion::JFDUKE3D.getName()));
	const GameVersion * jfDuke3DGameVersionRaw = jfDuke3DGameVersion != nullptr ? jfDuke3DGameVersion.get() : &GameVersion::JFDUKE3D;

	if(!jfDuke3DGameVersionRaw->hasSupportedOperatingSystemType(operatingSystemType)) {
		return {};
	}

	HTTPService * httpService = HTTPService::getInstance();

	size_t schemeSuffixIndex = JFDUKE32_DOWNLOAD_PAGE_URL.find("://");

	if(schemeSuffixIndex == std::string::npos) {
		spdlog::error("Malformed JFDuke3D download page URL, missing scheme.");
		return {};
	}

	size_t firstPathSeparatorIndex = JFDUKE32_DOWNLOAD_PAGE_URL.find_first_of("/", schemeSuffixIndex + 3);

	std::string_view downloadPageBaseURL(JFDUKE32_DOWNLOAD_PAGE_URL.data(), JFDUKE32_DOWNLOAD_PAGE_URL.length());

	if(firstPathSeparatorIndex != std::string::npos) {
		downloadPageBaseURL = std::string_view(JFDUKE32_DOWNLOAD_PAGE_URL.data(), firstPathSeparatorIndex);
	}

	std::shared_ptr<HTTPRequest> downloadPageRequest(httpService->createRequest(HTTPRequest::Method::Get, JFDUKE32_DOWNLOAD_PAGE_URL));
	downloadPageRequest->setConnectionTimeout(5s);
	downloadPageRequest->setNetworkTimeout(10s);

	std::future<std::shared_ptr<HTTPResponse>> futureResponse(httpService->sendRequest(downloadPageRequest));

	if(!futureResponse.valid()) {
		spdlog::error("Failed to create JFDuke3D download page HTTP request!");
		return {};
	}

	futureResponse.wait();

	std::shared_ptr<HTTPResponse> response(futureResponse.get());

	if(response->isFailure()) {
		spdlog::error("Failed to retrieve JFDuke3D download page with error: {}", response->getErrorMessage());
		return {};
	}

	if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to get JFDuke3D download page ({}{})!", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return {};
	}

	std::string pageHTML(Utilities::tidyHTML(response->getBodyAsString()));

	if(pageHTML.empty()) {
		spdlog::error("Failed to tidy JFDuke3D download page HTML.");
		return {};
	}

	response.reset();

	tinyxml2::XMLDocument document;

	if(document.Parse(pageHTML.c_str(), pageHTML.length()) != tinyxml2::XML_SUCCESS) {
		spdlog::error("Failed to parse JDFuke3D download page XHTML with error: '{}'.", document.ErrorStr());
		return {};
	}

	std::vector<const tinyxml2::XMLElement *> downloadElements(Utilities::findXMLElementsWithAttributeValue(document.RootElement(), "class", "download"));

	if(downloadElements.empty()) {
		spdlog::error("JFDuke3D download page parsing failed, missing download element.");
		return {};
	}

	if(downloadElements.size() != 1) {
		spdlog::warn("More than one download element found on JFDuke3D download page.");
	}

	const tinyxml2::XMLElement * downloadElement = downloadElements[0];

	std::vector<const tinyxml2::XMLElement *> downloadLinkElements(Utilities::findXMLElementsWithName(downloadElement, "a"));

	const char * classAttributeRawValue = nullptr;
	const char * linkAttributeRawValue = nullptr;
	std::string_view classAttributeValue;
	std::string_view linkAttributeValue;
	std::string downloadPath;
	std::string windowsX86DownloadPath;
	std::string windowsX64DownloadPath;

	for(std::vector<const tinyxml2::XMLElement *>::const_iterator i = downloadLinkElements.cbegin(); i != downloadLinkElements.cend(); ++i) {
		classAttributeRawValue = (*i)->Attribute("class");

		if(classAttributeRawValue == nullptr) {
			continue;
		}

		linkAttributeRawValue = (*i)->Attribute("href");

		if(linkAttributeRawValue == nullptr) {
			continue;
		}

		classAttributeValue = classAttributeRawValue;
		linkAttributeValue = linkAttributeRawValue;

		if(operatingSystemType == DeviceInformationBridge::OperatingSystemType::MacOS) {
			if(classAttributeValue.find("mac") != std::string::npos) {
				downloadPath = linkAttributeValue;
			}
		}
		else if(operatingSystemType == DeviceInformationBridge::OperatingSystemType::Windows) {
			if(classAttributeValue.find("win") != std::string::npos) {
				if(classAttributeValue.find("x86") == std::string::npos) {
					windowsX86DownloadPath = linkAttributeValue;
				}
				else {
					windowsX64DownloadPath = linkAttributeValue;
				}
			}
		}
	}

	if(operatingSystemType == DeviceInformationBridge::OperatingSystemType::Windows) {
		if(operatingSystemArchitectureType == DeviceInformationBridge::OperatingSystemArchitectureType::x64) {
			if(!windowsX64DownloadPath.empty()) {
				downloadPath = windowsX64DownloadPath;
			}
			else {
				downloadPath = windowsX86DownloadPath;
			}
		}
		else if(operatingSystemArchitectureType == DeviceInformationBridge::OperatingSystemArchitectureType::x86) {
			downloadPath = windowsX86DownloadPath;
		}
	}

	if(downloadPath.empty()) {
		spdlog::error("Failed to determine JFDuke3D download URL from download page XHTML.");
		return {};
	}

	if(downloadPath[0] != '/') {
		return Utilities::joinPaths(JFDUKE32_DOWNLOAD_PAGE_URL, downloadPath);
	}
	else {
		return Utilities::joinPaths(downloadPageBaseURL, downloadPath);
	}
}

std::string GameManager::getEDuke32DownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const {
	static const std::string WINDOWS_X86_ARCHITECTURE_IDENTIFIER("win32");
	static const std::string WINDOWS_X64_ARCHITECTURE_IDENTIFIER("win64");

	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	std::shared_ptr<GameVersion> eDuke32GameVersion(m_gameVersions->getGameVersion(GameVersion::EDUKE32.getName()));
	const GameVersion * eDuke32GameVersionRaw = eDuke32GameVersion != nullptr ? eDuke32GameVersion.get() : &GameVersion::EDUKE32;

	if(!eDuke32GameVersionRaw->hasSupportedOperatingSystemType(operatingSystemType)) {
		return {};
	}

	HTTPService * httpService = HTTPService::getInstance();

	std::shared_ptr<HTTPRequest> downloadPageRequest(httpService->createRequest(HTTPRequest::Method::Get, EDUKE32_DOWNLOAD_PAGE_URL));
	downloadPageRequest->setConnectionTimeout(5s);
	downloadPageRequest->setNetworkTimeout(10s);

	std::future<std::shared_ptr<HTTPResponse>> futureResponse(httpService->sendRequest(downloadPageRequest));

	if(!futureResponse.valid()) {
		spdlog::error("Failed to create eDuke32 download page HTTP request!");
		return {};
	}

	futureResponse.wait();

	std::shared_ptr<HTTPResponse> response(futureResponse.get());

	if(response->isFailure()) {
		spdlog::error("Failed to retrieve eDuke32 download page with error: {}", response->getErrorMessage());
		return {};
	}

	if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to get eDuke32 download page ({}{})!", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return {};
	}

	std::string pageHTML(Utilities::tidyHTML(response->getBodyAsString()));

	if(pageHTML.empty()) {
		spdlog::error("Failed to tidy eDuke32 download page HTML.");
		return {};
	}

	response.reset();

	tinyxml2::XMLDocument document;

	if(document.Parse(pageHTML.c_str(), pageHTML.length()) != tinyxml2::XML_SUCCESS) {
		spdlog::error("Failed to parse eDuke32 download page XHTML with error: '{}'.", document.ErrorStr());
		return {};
	}

	const tinyxml2::XMLElement * listingElement = Utilities::findXMLElementWithID(document.RootElement(), "listing");

	if(listingElement == nullptr) {
		spdlog::error("eDuke32 download page parsing failed, could not find listing element.");
		return {};
	}

	std::string_view currentDownloadFileName;
	std::string windowsX86DownloadFileName;
	std::string windowsX64DownloadFileName;
	const tinyxml2::XMLElement * listingEntryLinkElement = nullptr;
	const tinyxml2::XMLElement * listingEntryElement = listingElement->FirstChildElement();

	while(true) {
		if(listingEntryElement == nullptr) {
			break;
		}

		listingEntryLinkElement = Utilities::findXMLElementWithName(listingEntryElement, "a");

		if(listingEntryLinkElement != nullptr) {
			currentDownloadFileName = listingEntryLinkElement->Attribute("href");

			if(currentDownloadFileName.find("eduke32") == 0 && currentDownloadFileName.find("debug") == std::string::npos) {
				if(currentDownloadFileName.find(WINDOWS_X86_ARCHITECTURE_IDENTIFIER) != std::string::npos) {
					windowsX86DownloadFileName = currentDownloadFileName;
				}
				else if(currentDownloadFileName.find(WINDOWS_X64_ARCHITECTURE_IDENTIFIER) != std::string::npos) {
					windowsX64DownloadFileName = currentDownloadFileName;
				}
			}
		}

		listingEntryElement = listingEntryElement->NextSiblingElement();
	}

	std::string finalDownloadFileName;

	if(operatingSystemArchitectureType == DeviceInformationBridge::OperatingSystemArchitectureType::x64) {
		if(!windowsX64DownloadFileName.empty()) {
			finalDownloadFileName = windowsX64DownloadFileName;
		}
		else {
			finalDownloadFileName = windowsX86DownloadFileName;
		}
	}
	else if(operatingSystemArchitectureType == DeviceInformationBridge::OperatingSystemArchitectureType::x86) {
		finalDownloadFileName = windowsX86DownloadFileName;
	}

	if(finalDownloadFileName.empty()) {
		spdlog::error("Failed to determine eDuke32 download URL from download page XHTML.");

		return {};
	}

	return Utilities::joinPaths(EDUKE32_DOWNLOAD_PAGE_URL, finalDownloadFileName);
}

std::string GameManager::getRazeDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const {
	static const std::string WINDOWS_X64_ARCHITECTURE_IDENTIFIER("arm64");
	static const std::string LINUX_IDENTIFIER("linux");
	static const std::string MACOS_IDENTIFIER("mac");
	static const std::string PDB_IDENTIFIER("pdb");

	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	std::shared_ptr<GameVersion> razeGameVersion(m_gameVersions->getGameVersion(GameVersion::RAZE.getName()));
	const GameVersion * razeGameVersionRaw = razeGameVersion != nullptr ? razeGameVersion.get() : &GameVersion::RAZE;

	if(!razeGameVersionRaw->hasSupportedOperatingSystemType(operatingSystemType)) {
		return {};
	}

	GitHubService * gitHubService = GitHubService::getInstance();

	std::unique_ptr<GitHubRelease> latestRelease(gitHubService->getLatestRelease(GameVersion::RAZE.getSourceCodeURL()));

	if(latestRelease == nullptr) {
		return {};
	}

	std::shared_ptr<GitHubReleaseAsset> currentReleaseAsset;
	std::shared_ptr<GitHubReleaseAsset> latestReleaseAsset;

	if(operatingSystemType == DeviceInformationBridge::OperatingSystemType::Windows && operatingSystemArchitectureType == DeviceInformationBridge::OperatingSystemArchitectureType::x64) {
		for(size_t i = 0; i < latestRelease->numberOfAssets(); i++) {
			currentReleaseAsset = latestRelease->getAsset(i);

			if(Utilities::toLowerCase(currentReleaseAsset->getFileName()).find(WINDOWS_X64_ARCHITECTURE_IDENTIFIER) != std::string::npos) {
				latestReleaseAsset = currentReleaseAsset;
				break;
			}
		}
	}

	if(latestReleaseAsset == nullptr) {
		std::string lowerCaseAssetFileName;

		for(size_t i = 0; i < latestRelease->numberOfAssets(); i++) {
			currentReleaseAsset = latestRelease->getAsset(i);
			lowerCaseAssetFileName = Utilities::toLowerCase(currentReleaseAsset->getFileName());

			if(lowerCaseAssetFileName.find(PDB_IDENTIFIER) != std::string::npos ||
			   lowerCaseAssetFileName.find(WINDOWS_X64_ARCHITECTURE_IDENTIFIER) != std::string::npos) {
				continue;
			}

			if(latestReleaseAsset != nullptr) {
				spdlog::warn("Found multiple '{}' asset downloads, GitHub release may be misconfigured.", GameVersion::RAZE.getName());
				continue;
			}

			if(lowerCaseAssetFileName.find(LINUX_IDENTIFIER) != std::string::npos) {
				if(operatingSystemType != DeviceInformationBridge::OperatingSystemType::Linux) {
					continue;
				}

				latestReleaseAsset = currentReleaseAsset;
			}
			else if(lowerCaseAssetFileName.find(MACOS_IDENTIFIER) != std::string::npos) {
				if(operatingSystemType != DeviceInformationBridge::OperatingSystemType::MacOS) {
					continue;
				}

				latestReleaseAsset = currentReleaseAsset;
			}

			if(operatingSystemType == DeviceInformationBridge::OperatingSystemType::Windows) {
				latestReleaseAsset = currentReleaseAsset;
			}
		}
	}

	if(latestReleaseAsset == nullptr) {
		spdlog::error("Could not find '{}' GitHub release asset with matching download file name for '{}'.", GameVersion::RAZE.getName(), magic_enum::enum_name(operatingSystemType));
		return {};
	}

	return latestReleaseAsset->getDownloadURL();
}

std::string GameManager::getBelgianChocolateDuke3DDownloadURL() const {
	static const std::string X86_ARCHITECTURE_IDENTIFIER("win32");
	static const std::string X64_ARCHITECTURE_IDENTIFIER("win64");

	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	DeviceInformationBridge * deviceInformationBridge = DeviceInformationBridge::getInstance();

	std::optional<DeviceInformationBridge::OperatingSystemType> optionalOperatingSystemType(deviceInformationBridge->getOperatingSystemType());

	if(!optionalOperatingSystemType.has_value()) {
		spdlog::error("Failed to determine operating system type.");
		return {};
	}

	std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> optionalOperatingSystemArchitectureType(deviceInformationBridge->getOperatingSystemArchitectureType());

	if(!optionalOperatingSystemArchitectureType.has_value()) {
		spdlog::error("Failed to determine operating system architecture type.");
		return {};
	}

	GitHubService * gitHubService = GitHubService::getInstance();

	std::unique_ptr<GitHubRelease> latestRelease(gitHubService->getLatestRelease(GameVersion::BELGIAN_CHOCOLATE_DUKE3D.getSourceCodeURL()));

	if(latestRelease == nullptr) {
		return {};
	}

	std::string_view architectureIdentifier;

	switch(optionalOperatingSystemArchitectureType.value()) {
		case DeviceInformationBridge::OperatingSystemArchitectureType::x86: {
			architectureIdentifier = X86_ARCHITECTURE_IDENTIFIER;
			break;
		}
		case DeviceInformationBridge::OperatingSystemArchitectureType::x64: {
			architectureIdentifier = X64_ARCHITECTURE_IDENTIFIER;
			break;
		}
	}

	std::shared_ptr<GitHubReleaseAsset> currentReleaseAsset;
	std::shared_ptr<GitHubReleaseAsset> latestReleaseAsset;

	for(size_t i = 0; i < latestRelease->numberOfAssets(); i++) {
		currentReleaseAsset = latestRelease->getAsset(i);

		if(Utilities::toLowerCase(currentReleaseAsset->getFileName()).find(architectureIdentifier) != std::string::npos) {
			latestReleaseAsset = currentReleaseAsset;
			break;
		}
	}

	if(latestReleaseAsset == nullptr) {
		switch(optionalOperatingSystemArchitectureType.value()) {
			case DeviceInformationBridge::OperatingSystemArchitectureType::x86: {
				spdlog::error("Could not find '{}' GitHub release asset with matching download file name for '{}' ({}).",  GameVersion::BELGIAN_CHOCOLATE_DUKE3D.getName(), magic_enum::enum_name(optionalOperatingSystemType.value()), magic_enum::enum_name(optionalOperatingSystemArchitectureType.value()));
				return {};
			}
			case DeviceInformationBridge::OperatingSystemArchitectureType::x64: {
				for(size_t i = 0; i < latestRelease->numberOfAssets(); i++) {
					currentReleaseAsset = latestRelease->getAsset(i);

					if(Utilities::toLowerCase(currentReleaseAsset->getFileName()).find(X86_ARCHITECTURE_IDENTIFIER) != std::string::npos) {
						latestReleaseAsset = currentReleaseAsset;
						break;
					}
				}

				if(latestReleaseAsset == nullptr) {
					spdlog::error("Could not find '{}' GitHub release asset with matching download file name for '{}' ({} or {}).", GameVersion::BELGIAN_CHOCOLATE_DUKE3D.getName(), magic_enum::enum_name(optionalOperatingSystemType.value()), magic_enum::enum_name(DeviceInformationBridge::OperatingSystemArchitectureType::x64), magic_enum::enum_name(DeviceInformationBridge::OperatingSystemArchitectureType::x86));
					return {};
				}

				break;
			}
		}
	}

	return latestReleaseAsset->getDownloadURL();
}

std::string GameManager::getRedNukemDownloadURL() const {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return {};
	}

	HTTPService * httpService = HTTPService::getInstance();

	std::shared_ptr<HTTPRequest> downloadPageRequest(httpService->createRequest(HTTPRequest::Method::Get, RED_NUKEM_DOWNLOAD_PAGE_URL));
	downloadPageRequest->setConnectionTimeout(5s);
	downloadPageRequest->setNetworkTimeout(10s);

	std::future<std::shared_ptr<HTTPResponse>> futureResponse(httpService->sendRequest(downloadPageRequest));

	if(!futureResponse.valid()) {
		spdlog::error("Failed to create RedNukem download page HTTP request!");
		return {};
	}

	futureResponse.wait();

	std::shared_ptr<HTTPResponse> response(futureResponse.get());

	if(response->isFailure()) {
		spdlog::error("Failed to retrieve RedNukem download page with error: {}", response->getErrorMessage());
		return {};
	}

	if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to get RedNukem download page ({}{})!", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return {};
	}

	std::string pageHTML(Utilities::tidyHTML(response->getBodyAsString()));

	if(pageHTML.empty()) {
		spdlog::error("Failed to tidy RedNukem download page HTML.");
		return {};
	}

	response.reset();

	tinyxml2::XMLDocument document;

	if(document.Parse(pageHTML.c_str(), pageHTML.length()) != tinyxml2::XML_SUCCESS) {
		spdlog::error("Failed to parse RedNukem download page XHTML with error: '{}'.", document.ErrorStr());
		return {};
	}

	const tinyxml2::XMLElement * bodyElement = Utilities::findXMLElementWithName(document.RootElement(), "body");

	if(bodyElement == nullptr) {
		spdlog::error("RedNukem download page XHTML is missing 'body' element.");
		return {};
	}

	std::string version;
	size_t separatorIndex = std::string::npos;
	const tinyxml2::XMLElement * downloadContainerElement = nullptr;
	std::vector<const tinyxml2::XMLElement *> downloadLinks;
	const char * downloadLinkRaw = nullptr;
	std::string_view downloadLink;
	const tinyxml2::XMLElement * currentBodyChildElement = bodyElement->FirstChildElement();

	while(true) {
		if(currentBodyChildElement == nullptr) {
			break;
		}

		if(Utilities::areStringsEqual(currentBodyChildElement->Name(), "button")) {
			std::string_view buttonText(currentBodyChildElement->GetText());

			separatorIndex = buttonText.find_first_of("-");

			if(separatorIndex != std::string::npos) {
				version = Utilities::trimString(std::string_view(buttonText.data(), separatorIndex));
			}

			if(!version.empty()) {
				downloadContainerElement = currentBodyChildElement->NextSiblingElement();

				if(downloadContainerElement != nullptr) {
					downloadLinks = Utilities::findXMLElementsWithName(downloadContainerElement, "a");

					if(!downloadLinks.empty()) {
						for(std::vector<const tinyxml2::XMLElement *>::const_iterator i = downloadLinks.cbegin(); i != downloadLinks.cend(); ++i) {
							downloadLinkRaw = (*i)->Attribute("href");

							if(downloadLinkRaw != nullptr) {
								downloadLink = downloadLinkRaw;

								if(downloadLink.find("rednukem") != std::string::npos &&
								   downloadLink.find("win64") != std::string::npos &&
								   downloadLink.find("debug") == std::string::npos) {
									return std::string(downloadLink);
								}
							}
							else {
								spdlog::warn("RedNukem download link is missing 'href' attribute.");
							}
						}
					}
					else {
						spdlog::warn("RedNukem download container element has no download links.");
					}
				}
				else {
					spdlog::warn("RedNukem download button has no corresponding download container element.");
				}
			}
			else {
				spdlog::warn("Failed to parse version from RedNukem download button.");
			}
		}

		currentBodyChildElement = currentBodyChildElement->NextSiblingElement();
	}

	spdlog::error("Failed to determine RedNukem download URL from download page XHTML.");

	return {};
}

bool GameManager::installGame(const GameVersion & gameVersion, const std::string & destinationDirectoryPath, bool useFallback, bool overwrite) {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return false;
	}

	HTTPService * httpService = HTTPService::getInstance();

	if(!std::filesystem::exists(std::filesystem::path(destinationDirectoryPath))) {
		std::error_code errorCode;
		std::filesystem::create_directories(destinationDirectoryPath, errorCode);

		if(errorCode) {
			spdlog::error("Failed to create game installation destination directory structure for path '{}': {}", destinationDirectoryPath, errorCode.message());
			return false;
		}
	}

	if(!std::filesystem::is_directory(std::filesystem::path(destinationDirectoryPath))) {
		spdlog::error("Game installation destination directory path is not a valid directory!");
		return false;
	}

	if(!isGameDownloadable(gameVersion.getName())) {
		spdlog::error("'{}' is not downloadable.", gameVersion.getName());
		return false;
	}

	std::string gameDownloadURL;

	if(useFallback) {
		spdlog::info("Using fallback Duke Nukem 3D game files download URL.");

		gameDownloadURL = getFallbackGameDownloadURL(gameVersion.getName());
	}
	else {
		gameDownloadURL = getGameDownloadURL(gameVersion.getName());
	}

	if(gameDownloadURL.empty()) {
		spdlog::error("Failed to determine download URL for '{}'.", gameVersion.getName());
		return false;
	}

	spdlog::info("Downloading '{}' game files package from: '{}'...", gameVersion.getName(), gameDownloadURL);

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, gameDownloadURL));

	std::shared_ptr<HTTPResponse> response(httpService->sendRequestAndWait(request));

	if(response->isFailure()) {
		spdlog::error("Failed to download '{}' game files package with error: {}", gameVersion.getName(), response->getErrorMessage());

		if(!useFallback) {
			return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
		}

		return false;
	}

	if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to download '{}' game files package ({}{})!", gameVersion.getName(), response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);

		if(!useFallback) {
			return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
		}

		return false;
	}

	std::string expectedArchiveMD5Hash(response->getHeaderValue("Content-MD5"));

	if(!expectedArchiveMD5Hash.empty()) {
		std::string actualArchiveMD5Hash(response->getBodyMD5(ByteBuffer::HashFormat::Base64));

		if(actualArchiveMD5Hash == expectedArchiveMD5Hash) {
			spdlog::debug("Validated MD5 hash of '{}' game files package.", gameVersion.getName());
		}
		else {
			spdlog::error("Failed to validate MD5 hash of '{}' game files package! Expected base 64 MD5 hash: '{}', actual: '{}'.", expectedArchiveMD5Hash, actualArchiveMD5Hash);

			if(!useFallback) {
				return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
			}

			return false;
		}
	}

	spdlog::info("'{}' game files downloaded successfully after {} ms, extracting to '{}'...", gameVersion.getName(), response->getRequestDuration().value().count(), destinationDirectoryPath);

	std::unique_ptr<Archive> gameFilesArchive(ArchiveFactoryRegistry::getInstance()->createArchiveFrom(response->transferBody(), std::string(Utilities::getFileExtension(gameDownloadURL))));

	if(gameFilesArchive == nullptr) {
		spdlog::error("Failed to create archive handle from '{}' game files archive package!", gameVersion.getName());

		if(!useFallback) {
			return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
		}

		return false;
	}

	bool isRegularVersion = gameVersion.getName() == GameVersion::ORIGINAL_REGULAR_VERSION.getName();
	bool isAtomicEdition = gameVersion.getName() == GameVersion::ORIGINAL_ATOMIC_EDITION.getName();
	bool isJFDuke3D = gameVersion.getName() == GameVersion::JFDUKE3D.getName();
	bool isOriginalGameFallback = useFallback && (isRegularVersion || isAtomicEdition);

	std::function<bool(std::shared_ptr<ArchiveEntry>, const GameFileInformation &)> extractGameFileFunction([gameVersion, gameDownloadURL, destinationDirectoryPath, overwrite](std::shared_ptr<ArchiveEntry> gameFileEntry, const GameFileInformation & gameFileInfo) {
		if(gameFileEntry == nullptr) {
			// skip missing game files that aren't required
			if(!gameFileInfo.required) {
				return true;
			}

			spdlog::error("'{}' fallback game files package file '{}' is missing required '{}' file entry!", gameVersion.getName(), Utilities::getFileName(gameDownloadURL), gameFileInfo.fileName);
			return false;
		}

		std::unique_ptr<ByteBuffer> gameFileData(gameFileEntry->getData());

		if(gameFileData == nullptr) {
			spdlog::error("Failed to obtain '{}' fallback '{}' game file data from game files package file '{}'.", gameVersion.getName(), gameFileInfo.fileName, Utilities::getFileName(gameDownloadURL));
			return false;
		}

		bool gameFileSHA1Verified = false;
		std::string calculatedGameFileSHA1(gameFileData->getSHA1());

		for(const std::string & expectedGameFileSHA1 : gameFileInfo.hashes) {
			if(calculatedGameFileSHA1 == expectedGameFileSHA1) {
				gameFileSHA1Verified = true;
				break;
			}
		}

		if(!gameFileSHA1Verified) {
			if(gameFileInfo.hashes.size() == 1) {
				spdlog::error("'{}' '{}' game file SHA1 hash verification failed. Calculated '{}', but expected: '{}'.", gameVersion.getName(), gameFileInfo.fileName, calculatedGameFileSHA1, gameFileInfo.hashes.front());
			}
			else {
				std::stringstream gameFileHashesStream;

				for(const std::string & gameFileSHA1 : gameFileInfo.hashes) {
					if(gameFileHashesStream.tellp() != 0) {
						gameFileHashesStream << ", ";
					}

					gameFileHashesStream << "'" << gameFileSHA1 << "'";
				}

				spdlog::error("'{}' '{}' game file SHA1 hash verification failed. Calculated '{}', but expected one of: {}.", gameVersion.getName(), gameFileInfo.fileName, calculatedGameFileSHA1, gameFileHashesStream.str());
			}

			return false;
		}

		std::string gameFileDestinationPath(Utilities::joinPaths(destinationDirectoryPath, gameFileInfo.fileName));

		if(!gameFileData->writeTo(gameFileDestinationPath, overwrite)) {
			spdlog::error("Failed to write '{}' '{}' game file data from game files package file '{}' to '{}'.", gameVersion.getName(), gameFileInfo.fileName, Utilities::getFileName(gameDownloadURL), gameFileDestinationPath);
			return false;
		}

		return true;
	});

	if(isOriginalGameFallback) {
		// only extract required files from fallback downloads for original game files since there are other files we don't want or need in these archives
		if(isRegularVersion) {
			for(const GameFileInformation & gameFileInfo : REGULAR_VERSION_GAME_FILE_INFO_LIST) {
				if(!extractGameFileFunction(std::shared_ptr<ArchiveEntry>(gameFilesArchive->getFirstEntryWithName(gameFileInfo.fileName, true).lock()), gameFileInfo)) {
					if(!useFallback) {
						return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
					}

					return false;
				}
			}
		}
		else if(isAtomicEdition) {
			for(const GameFileInformation & gameFileInfo : ATOMIC_EDITION_GAME_FILE_INFO_LIST) {
				if(!extractGameFileFunction(std::shared_ptr<ArchiveEntry>(gameFilesArchive->getFirstEntryWithName(gameFileInfo.fileName, true).lock()), gameFileInfo)) {
					if(!useFallback) {
						return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
					}

					return false;
				}
			}
		}
	}
	else {
		if(gameFilesArchive->extractAllEntries(destinationDirectoryPath, true) == 0) {
			spdlog::error("Failed to extract '{}' game files archive package to '{}'.", gameVersion.getName(), destinationDirectoryPath);

			if(!useFallback) {
				return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
			}

			return false;
		}
	}

	if(isJFDuke3D) {
		std::filesystem::directory_entry jfDuke3DSubdirectory;

		for(const std::filesystem::directory_entry & entry : std::filesystem::directory_iterator(std::filesystem::path(destinationDirectoryPath))) {
			if(entry.is_directory()) {
				jfDuke3DSubdirectory = entry;
				break;
			}
		}

		if(jfDuke3DSubdirectory.exists()) {
			std::string jfDuke3DSubdirectoryPath(jfDuke3DSubdirectory.path().string());
			std::error_code errorCode;

			for(const std::filesystem::directory_entry & entry : std::filesystem::directory_iterator(std::filesystem::path(jfDuke3DSubdirectory))) {
				std::string currentEntryPath(entry.path().string());
				std::string newEntryPath(Utilities::joinPaths(destinationDirectoryPath, currentEntryPath.substr(currentEntryPath.find(jfDuke3DSubdirectoryPath) + jfDuke3DSubdirectoryPath.length())));

				spdlog::debug("Moving '{}' file '{}' to parent directory...", gameVersion.getName(), Utilities::getFileName(currentEntryPath));

				std::filesystem::rename(std::filesystem::path(currentEntryPath), std::filesystem::path(newEntryPath), errorCode);

				if(errorCode) {
					spdlog::error("Failed to move '{}' to '{}' with error: {}", currentEntryPath, newEntryPath, errorCode.message());
				}
			}

			spdlog::debug("Removing empty '{}' subdirectory: '{}'...", gameVersion.getName(), Utilities::getFileName(jfDuke3DSubdirectoryPath));

			std::filesystem::remove(std::filesystem::path(jfDuke3DSubdirectoryPath), errorCode);

			if(errorCode) {
				spdlog::error("Failed to remove '{}' subdirectory '{}' with error: {}", gameVersion.getName(), jfDuke3DSubdirectoryPath, errorCode.message());
			}
		}
	}
	else if(isRegularVersion || isAtomicEdition) {
		std::function<bool(const GameFileInformation &)> gameFileSHA1VerificationFunction([gameVersion, destinationDirectoryPath](const GameFileInformation & gameFileInfo) {
			bool gameFileSHA1Verified = false;
			std::string calculatedGameFileSHA1(Utilities::getFileSHA1Hash(Utilities::joinPaths(destinationDirectoryPath, gameFileInfo.fileName)));

			if(calculatedGameFileSHA1.empty()) {
				// skip missing files that aren't required
				if(!gameFileInfo.required) {
					return true;
				}

				spdlog::error("Failed to calculate '{}' '{}' game file SHA1.", gameVersion.getName(), gameFileInfo.fileName);
				return false;
			}

			for(const std::string & expectedGameFileSHA1 : gameFileInfo.hashes) {
				if(calculatedGameFileSHA1 == expectedGameFileSHA1) {
					gameFileSHA1Verified = true;
					break;
				}
			}

			if(!gameFileSHA1Verified) {
				if(gameFileInfo.hashes.size() == 1) {
					spdlog::error("'{}' '{}' game file SHA1 hash verification failed. Calculated '{}', but expected: '{}'.", gameVersion.getName(), gameFileInfo.fileName, calculatedGameFileSHA1, gameFileInfo.hashes.front());
				}
				else {
					std::stringstream gameFileHashesStream;

					for(const std::string & gameFileSHA1 : gameFileInfo.hashes) {
						if(gameFileHashesStream.tellp() != 0) {
							gameFileHashesStream << ", ";
						}

						gameFileHashesStream << "'" << gameFileSHA1 << "'";
					}

					spdlog::error("'{}' '{}' game file SHA1 hash verification failed. Calculated '{}', but expected one of: {}.", gameVersion.getName(), gameFileInfo.fileName, calculatedGameFileSHA1, gameFileHashesStream.str());
				}

				return false;
			}

			return true;
		});

		if(!useFallback) {
			if(isRegularVersion) {
				for(const GameFileInformation & gameFileInfo : REGULAR_VERSION_GAME_FILE_INFO_LIST) {
					if(!gameFileSHA1VerificationFunction(gameFileInfo)) {
						if(!useFallback) {
							return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
						}

						return false;
					}
				}
			}
			else if(isAtomicEdition) {
				for(const GameFileInformation & gameFileInfo : ATOMIC_EDITION_GAME_FILE_INFO_LIST) {
					if(!gameFileSHA1VerificationFunction(gameFileInfo)) {
						if(!useFallback) {
							return installGame(gameVersion, destinationDirectoryPath, true, overwrite);
						}

						return false;
					}
				}
			}
		}

		if(isAtomicEdition) {
			std::string atomicEditionGameExecutablePath(Utilities::joinPaths(destinationDirectoryPath, gameVersion.getGameExecutableName()));

			spdlog::info("Checking '{}' game executable status...", gameVersion.getName(), atomicEditionGameExecutablePath);

			NoCDCracker::GameExecutableStatus gameExecutableStatus = NoCDCracker::getGameExecutableStatus(atomicEditionGameExecutablePath);

			if(None(gameExecutableStatus & NoCDCracker::GameExecutableStatus::Exists)) {
				spdlog::error("'{}' game executable does not exist at path: '{}'!", gameVersion.getName(), atomicEditionGameExecutablePath);
			}
			else if(Any(gameExecutableStatus & NoCDCracker::GameExecutableStatus::Invalid)) {
				spdlog::error("Invalid '{}' game executable at path: '{}'!", gameVersion.getName(), atomicEditionGameExecutablePath);
			}
			else if(Any(gameExecutableStatus & NoCDCracker::GameExecutableStatus::RegularVersion)) {
				spdlog::error("Found '{}' game executable instead of '{}' at path: '{}'!", GameVersion::ORIGINAL_REGULAR_VERSION.getName(), gameVersion.getName(), atomicEditionGameExecutablePath);
			}
			else if(Any(gameExecutableStatus & NoCDCracker::GameExecutableStatus::AtomicEdition)) {
				if(Any(gameExecutableStatus & NoCDCracker::GameExecutableStatus::Cracked)) {
					spdlog::info("'{}' game executable already cracked!", gameVersion.getName());
				}
				else if(NoCDCracker::crackGameExecutable(atomicEditionGameExecutablePath)) {
					spdlog::info("'{}' game executable cracked successfully! CD no longer required.", gameVersion.getName());
				}
				else {
					spdlog::error("Failed to crack '{}' game executable at path: '{}'.", gameVersion.getName(), atomicEditionGameExecutablePath);
				}
			}
		}
	}

	// don't need to install the group file or verify it separately when using a fallback download for either of the original game versions, since it's already installed & verified
	if (!isOriginalGameFallback && gameVersion.hasGroupFileInstallPath()) {
		std::string groupFileType(Utilities::areStringsEqual(gameVersion.getName(), GameVersion::ORIGINAL_REGULAR_VERSION.getName()) ? GameVersion::ORIGINAL_REGULAR_VERSION.getName() : GameVersion::ORIGINAL_ATOMIC_EDITION.getName());

		if (!installGroupFile(groupFileType, Utilities::joinPaths(destinationDirectoryPath, gameVersion.getGroupFileInstallPath().value()), false, overwrite)) {
			return false;
		}
	}

	return true;
}

bool GameManager::installGroupFile(const std::string & gameName, const std::string & directoryPath, bool useFallback, bool overwrite) const {
	if(!m_initialized) {
		spdlog::error("Game manager not initialized!");
		return false;
	}

	HTTPService * httpService = HTTPService::getInstance();

	if(!std::filesystem::is_directory(std::filesystem::path(directoryPath))) {
		spdlog::error("Destination directory path does not exist!");
		return false;
	}

	std::filesystem::path destinationGroupFilePath(Utilities::joinPaths(directoryPath, Group::DUKE_NUKEM_3D_GROUP_FILE_NAME));

	if(std::filesystem::is_regular_file(destinationGroupFilePath) && !overwrite) {
		spdlog::error("Duke Nukem 3D group file already exists at '{}', specify overwrite to replace.", destinationGroupFilePath.string());
		return false;
	}

	if(!useFallback && gameName == GameVersion::ORIGINAL_ATOMIC_EDITION.getName()) {
		GameLocator * gameLocator = GameLocator::getInstance();

		for(size_t i = 0; i < gameLocator->numberOfGamePaths(); i++) {
			std::string sourceGroupFilePath(Utilities::joinPaths(*gameLocator->getGamePath(i), Group::DUKE_NUKEM_3D_GROUP_FILE_NAME));

			if(!std::filesystem::is_regular_file(std::filesystem::path(sourceGroupFilePath))) {
				spdlog::warn("Duke Nukem 3D group file is missing at: '{}'.", sourceGroupFilePath);
				continue;
			}

			std::string groupSHA1(Utilities::getFileSHA1Hash(sourceGroupFilePath));

			if(groupSHA1.empty()) {
				spdlog::error("Failed to calculate SHA1 hash of Duke Nukem 3D group file: '{}'!", sourceGroupFilePath);
			}
			else if(groupSHA1 == Group::DUKE_NUKEM_3D_ATOMIC_EDITION_GROUP_SHA1_FILE_HASH) {
				spdlog::debug("Verified '{}' group file SHA1 hash.", GameVersion::ORIGINAL_ATOMIC_EDITION.getName());
			}
			else if(groupSHA1 == Group::DUKE_NUKEM_3D_WORLD_TOUR_GROUP_SHA1_FILE_HASH) {
				spdlog::debug("Verified '{}' group file SHA1 hash.", GameVersion::ORIGINAL_ATOMIC_EDITION.getName());
			}
			else if(groupSHA1 == Group::DUKE_NUKEM_3D_REGULAR_VERSION_GROUP_SHA1_FILE_HASH) {
				spdlog::error("Calculated '{}' SHA1 hash for Duke Nukem 3D group file '{}', when '{}' group file was expected! This may cause unexpected gameplay issues.", GameVersion::ORIGINAL_REGULAR_VERSION.getName(), sourceGroupFilePath, GameVersion::ORIGINAL_ATOMIC_EDITION.getName());
			}
			else {
				spdlog::warn("Unexpected SHA1 hash calculated for Duke Nukem 3D group file '{}'! Game data may be modified, and may cause gameplay issues.", sourceGroupFilePath);
			}

			spdlog::debug("Copying Duke Nukem 3D group file from '{}' to: '{}'...", sourceGroupFilePath, destinationGroupFilePath.string());

			std::error_code errorCode;
			std::filesystem::copy_file(std::filesystem::path(sourceGroupFilePath), destinationGroupFilePath, errorCode);

			if(errorCode) {
				spdlog::error("Failed to copy Duke Nukem 3D group file from '{}' to '{}' with error: '{}'.", sourceGroupFilePath, destinationGroupFilePath.string(), errorCode.message());
			}

			return true;
		}
	}

	std::string groupDownloadURL;

	if(useFallback) {
		groupDownloadURL = getFallbackGroupDownloadURL(gameName);

		spdlog::info("Using fallback Duke Nukem 3D group file download URL.");
	}
	else {
		groupDownloadURL = getGroupDownloadURL(gameName);
	}

	if(groupDownloadURL.empty()) {
		spdlog::error("Failed to determine group download URL for '{}'.", gameName);
		return false;
	}

	spdlog::info("Downloading Duke Nukem 3D {} group file from: '{}'...", gameName, groupDownloadURL);

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, groupDownloadURL));

	std::future<std::shared_ptr<HTTPResponse>> futureResponse(httpService->sendRequest(request));

	if(!futureResponse.valid()) {
		spdlog::error("Failed to create Duke Nukem 3D group file HTTP request!");
		return false;
	}

	futureResponse.wait();

	std::shared_ptr<HTTPResponse> response(futureResponse.get());

	if(response->isFailure()) {
		spdlog::error("Failed to download Duke Nukem 3D group file with error: {}", response->getErrorMessage());
		return false;
	}

	if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to download Duke Nukem 3D group file ({}{})!", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);

		if(!useFallback) {
			return installGroupFile(gameName, directoryPath, true, overwrite);
		}

		return false;
	}

	if(useFallback) {
		std::string responseSHA1(response->getBodySHA1());

		if(getFallbackGroupDownloadSHA1(gameName) == responseSHA1) {
			spdlog::debug("Duke Nukem 3D {} fallback group file archive SHA1 hash validated.", gameName);
		}
		else {
			spdlog::error("Duke Nukem 3D {} fallback group file archive '{}' SHA1 hash validation failed! Expected '{}', but calculated: '{}'.", gameName, getFallbackGroupDownloadURL(gameName), getFallbackGroupDownloadSHA1(gameName), responseSHA1);
			return false;
		}
	}
	else if(m_gameDownloads != nullptr) {
		std::shared_ptr<GameDownload> gameDownload(m_gameDownloads->getDownloadWithName(gameName));

		if(gameDownload != nullptr && gameDownload->numberOfVersions() != 0) {
			std::string groupFileDownloadFileName(Utilities::getFileName(groupDownloadURL));

			std::shared_ptr<GameDownloadVersion> gameDownloadVersion(gameDownload->getVersion(0));
			std::shared_ptr<GameDownloadFile> groupFileDownload(gameDownloadVersion->getFileWithName(groupFileDownloadFileName));

			if(groupFileDownload != nullptr) {
				std::string responseSHA1(response->getBodySHA1());

				if(responseSHA1 == groupFileDownload->getSHA1()) {
					spdlog::debug("Duke Nukem 3D {} group file archive SHA1 hash validated.", gameName);
				}
				else {
					spdlog::error("Duke Nukem 3D {} group file archive '{}' SHA1 hash validation failed! Expected '{}', but calculated: '{}'.", gameName, Utilities::getFileName(groupDownloadURL), groupFileDownload->getSHA1(), responseSHA1);

					if(!useFallback) {
						return installGroupFile(gameName, directoryPath, true, overwrite);
					}

					return false;
				}
			}
		}
	}

	spdlog::info("Duke Nukem 3D {} group file downloaded successfully after {} ms, extracting to '{}'...", gameName, response->getRequestDuration().value().count(), destinationGroupFilePath.string());

	std::unique_ptr<Archive> groupArchive(ArchiveFactoryRegistry::getInstance()->createArchiveFrom(response->transferBody(), std::string(Utilities::getFileExtension(groupDownloadURL))));

	if(groupArchive == nullptr) {
		spdlog::error("Failed to create archive handle from '{}' group package file!", gameName);

		if(!useFallback) {
			return installGroupFile(gameName, directoryPath, true, overwrite);
		}

		return false;
	}

	std::shared_ptr<ArchiveEntry> groupFileEntry(groupArchive->getFirstEntryWithName(Group::DUKE_NUKEM_3D_GROUP_FILE_NAME, true).lock());

	if(groupFileEntry == nullptr) {
		spdlog::error("Duke Nukem 3D {} group package file is missing group file entry!", gameName);

		if(!useFallback) {
			return installGroupFile(gameName, directoryPath, true, overwrite);
		}

		return false;
	}

	std::unique_ptr<ByteBuffer> groupFileData(groupFileEntry->getData());

	if(groupFileData == nullptr) {
		spdlog::error("Failed to obtain '{}' group file data from package file.", gameName);

		if(!useFallback) {
			return installGroupFile(gameName, directoryPath, true, overwrite);
		}

		return false;
	}

	std::string groupFileDestinationPath(Utilities::joinPaths(directoryPath, Group::DUKE_NUKEM_3D_GROUP_FILE_NAME));

	if(!groupFileData->writeTo(groupFileDestinationPath, overwrite)) {
		spdlog::error("Failed to write '{}' group file data from package file to '{}'.", gameName, groupFileDestinationPath);
		return false;
	}

	std::string calculatedGroupSHA1(Utilities::getFileSHA1Hash(Utilities::joinPaths(directoryPath, Group::DUKE_NUKEM_3D_GROUP_FILE_NAME)));

	std::string_view expectedGroupSHA1;

	if(gameName == GameVersion::ORIGINAL_REGULAR_VERSION.getName()) {
		expectedGroupSHA1 = Group::DUKE_NUKEM_3D_REGULAR_VERSION_GROUP_SHA1_FILE_HASH;
	}
	else {
		expectedGroupSHA1 = Group::DUKE_NUKEM_3D_ATOMIC_EDITION_GROUP_SHA1_FILE_HASH;
	}

	if(calculatedGroupSHA1.empty()) {
		spdlog::error("Failed to calculate SHA1 hash of downloaded Duke Nukem 3D group file!");
	}
	else if(calculatedGroupSHA1 == expectedGroupSHA1) {
		spdlog::debug("Verified downloaded '{}' group file SHA1 hash.", gameName);
	}
	else {
		spdlog::error("Extracted '{}' group file SHA1 hash verification failed! Expected '{}', but calculated '{}'.", gameName, expectedGroupSHA1, calculatedGroupSHA1);
		return false;
	}

	return true;
}
