#include "DOSBoxManager.h"

#include <Archive/ArchiveFactoryRegistry.h>
#include <GitHub/GitHubService.h>
#include <Manager/SettingsManager.h>
#include <Network/HTTPService.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/TidyHTMLUtilities.h>
#include <Utilities/TinyXML2Utilities.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <filesystem>

using namespace std::chrono_literals;

DOSBoxManager::DOSBoxManager()
	: m_initialized(false)
	, m_dosboxVersions(std::make_shared<DOSBoxVersionCollection>()) { }

DOSBoxManager::~DOSBoxManager() = default;

bool DOSBoxManager::isInitialized() const {
	return m_initialized;
}

bool DOSBoxManager::initialize() {
	if(m_initialized) {
		return true;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	bool saveDOSBoxVersions = false;
	bool dosboxVersionsLoaded = m_dosboxVersions->loadFrom(settings->dosboxVersionsListFilePath);

	if(!dosboxVersionsLoaded || m_dosboxVersions->numberOfDOSBoxVersions() == 0) {
		if(!dosboxVersionsLoaded) {
			spdlog::warn("Missing or invalid DOSBox versions configuration file '{}', using default values.", settings->dosboxVersionsListFilePath);
		}
		else if(m_dosboxVersions->numberOfDOSBoxVersions() == 0) {
			spdlog::warn("Empty DOSBox versions configuration file '{}', using default values.", settings->dosboxVersionsListFilePath);
		}

		// use default DOSBox version configurations
		m_dosboxVersions->setDefaultDOSBoxVersions();
	}

	m_dosboxVersions->addMissingDefaultDOSBoxVersions();

	m_initialized = true;

	return true;
}

std::shared_ptr<DOSBoxVersionCollection> DOSBoxManager::getDOSBoxVersions() const {
	return m_dosboxVersions;
}

std::string DOSBoxManager::getLocalDOSBoxDownloadsListFilePath() const {
	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->remoteDOSBoxVersionsListFileName.empty()) {
		spdlog::error("Missing remote DOSBox list file name setting.");
		return {};
	}

	return Utilities::joinPaths(settings->downloadsDirectoryPath, settings->dosboxDownloadsDirectoryName, settings->remoteDOSBoxVersionsListFileName);
}

bool DOSBoxManager::shouldUpdateDOSBoxDownloadList() const {
	SettingsManager * settings = SettingsManager::getInstance();

	if(!settings->downloadThrottlingEnabled || !settings->dosboxDownloadListLastDownloadedTimestamp.has_value()) {
		return true;
	}

	std::string localDOSBoxDownloadsListFilePath(getLocalDOSBoxDownloadsListFilePath());

	if(!std::filesystem::is_regular_file(std::filesystem::path(localDOSBoxDownloadsListFilePath))) {
		return true;
	}

	return std::chrono::system_clock::now() - settings->dosboxDownloadListLastDownloadedTimestamp.value() > settings->dosboxDownloadListUpdateFrequency;
}

bool DOSBoxManager::loadOrUpdateDOSBoxDownloadList(bool forceUpdate) const {
	std::string localDOSBoxDownloadsListFilePath(getLocalDOSBoxDownloadsListFilePath());

	if(!forceUpdate && std::filesystem::is_regular_file(std::filesystem::path(localDOSBoxDownloadsListFilePath))) {
		std::unique_ptr<DOSBoxDownloadCollection> dosboxDownloads(std::make_unique<DOSBoxDownloadCollection>());

		if(dosboxDownloads->loadFrom(localDOSBoxDownloadsListFilePath) && DOSBoxDownloadCollection::isValid(dosboxDownloads.get())) {
			m_dosboxDownloads = std::move(dosboxDownloads);

			if(!shouldUpdateDOSBoxDownloadList()) {
				return true;
			}
		}
		else {
			spdlog::error("Failed to load DOSBox download collection from JSON file.");
		}
	}

	return updateDOSBoxDownloadList(forceUpdate);
}

bool DOSBoxManager::updateDOSBoxDownloadList(bool force) const {
	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		return false;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	std::string dosboxListRemoteFilePath(Utilities::joinPaths(settings->remoteDownloadsDirectoryName, settings->remoteDOSBoxDownloadsDirectoryName, settings->remoteDOSBoxVersionsListFileName));
	std::string dosboxListURL(Utilities::joinPaths(httpService->getBaseURL(), dosboxListRemoteFilePath));

	spdlog::info("Downloading DOSBox version download list from: '{}'...", dosboxListURL);

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, dosboxListURL));

	std::map<std::string, std::string>::const_iterator fileETagIterator = settings->fileETags.find(settings->remoteDOSBoxVersionsListFileName);

	if(!force && fileETagIterator != settings->fileETags.end() && !fileETagIterator->second.empty()) {
		request->setIfNoneMatchETag(fileETagIterator->second);
	}

	std::shared_ptr<HTTPResponse> response(httpService->sendRequestAndWait(request));

	if(response->isFailure()) {
		spdlog::error("Failed to download DOSBox version download list with error: {}", response->getErrorMessage());
		return false;
	}

	if(response->getStatusCode() == magic_enum::enum_integer(HTTPStatusCode::NotModified)) {
		spdlog::info("DOSBox version download list is already up to date!");

		settings->dosboxDownloadListLastDownloadedTimestamp = std::chrono::system_clock::now();

		return true;
	}
	else if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to download DOSBox version download list ({}{})!", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return false;
	}

	spdlog::info("DOSBox version download list downloaded successfully after {} ms.", response->getRequestDuration().value().count());

	std::unique_ptr<rapidjson::Document> dosboxDownloadCollectionDocument(response->getBodyAsJSON());

	if(dosboxDownloadCollectionDocument == nullptr) {
		spdlog::error("Failed to parse DOSBox version download collection JSON data.");
		return false;
	}

	std::unique_ptr<DOSBoxDownloadCollection> dosboxDownloads(DOSBoxDownloadCollection::parseFrom(*dosboxDownloadCollectionDocument));

	if(!DOSBoxDownloadCollection::isValid(dosboxDownloads.get())) {
		spdlog::error("Failed to parse DOSBox version download collection from JSON data.");
		return false;
	}

	std::string localDOSBoxDownloadsListFilePath(getLocalDOSBoxDownloadsListFilePath());

	if(!response->getBody()->writeTo(localDOSBoxDownloadsListFilePath, true)) {
		spdlog::error("Failed to write DOSBox version download collection JSON data to file: '{}'.", localDOSBoxDownloadsListFilePath);
		return false;
	}

	m_dosboxDownloads = std::move(dosboxDownloads);

	settings->fileETags.emplace(settings->remoteDOSBoxVersionsListFileName, response->getETag());
	settings->dosboxDownloadListLastDownloadedTimestamp = std::chrono::system_clock::now();

	return true;
}

std::string DOSBoxManager::getLatestDOSBoxDownloadURL(const DOSBoxVersion & dosboxVersion, std::string * latestVersion) const {
	if(!m_initialized) {
		spdlog::error("DOSBox manager not initialized!");
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

	return getLatestDOSBoxDownloadURL(dosboxVersion, optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value(), latestVersion);
}

std::string DOSBoxManager::getLatestDOSBoxDownloadURL(const DOSBoxVersion & dosboxVersion, DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion) const {
	if(Utilities::areStringsEqualIgnoreCase(dosboxVersion.getName(), DOSBoxVersion::DOSBOX.getName())) {
		return getLatestOriginalDOSBoxDownloadURL(operatingSystemType, operatingSystemArchitectureType, latestVersion);
	}
	else if(Utilities::areStringsEqualIgnoreCase(dosboxVersion.getName(), DOSBoxVersion::DOSBOX_STAGING.getName())) {
		return getLatestDOSBoxStagingDownloadURL(operatingSystemType, operatingSystemArchitectureType, latestVersion);
	}
	else if(Utilities::areStringsEqualIgnoreCase(dosboxVersion.getName(), DOSBoxVersion::DOSBOX_X.getName())) {
		return getLatestDOSBoxXDownloadURL(operatingSystemType, operatingSystemArchitectureType, latestVersion);
	}

	return {};
}

std::string DOSBoxManager::getRemoteDOSBoxDownloadsBaseURL() const {
	if(!m_initialized) {
		spdlog::error("DOSBox manager not initialized!");
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

	if(settings->remoteDOSBoxDownloadsDirectoryName.empty()) {
		spdlog::error("Missing remote DOSBox downloads directory name setting.");
		return {};
	}

	return Utilities::joinPaths(settings->apiBaseURL, settings->remoteDownloadsDirectoryName, settings->remoteDOSBoxDownloadsDirectoryName);
}

std::string DOSBoxManager::getLatestDOSBoxDownloadFallbackURL(const DOSBoxVersion & dosboxVersion, DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion) const {
	if(!loadOrUpdateDOSBoxDownloadList()) {
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

	std::shared_ptr<DOSBoxDownloadFile> dosboxDownloadFile(m_dosboxDownloads->getLatestDOSBoxDownloadFile(dosboxVersion.getName(), optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value()));

	if(dosboxDownloadFile == nullptr) {
		switch(optionalOperatingSystemArchitectureType.value()) {
			case DeviceInformationBridge::OperatingSystemArchitectureType::x86: {
				spdlog::error("Could not find '{}' file download information for '{}' ({}).", dosboxVersion.getName(), magic_enum::enum_name(optionalOperatingSystemType.value()), magic_enum::enum_name(optionalOperatingSystemArchitectureType.value()));
				return {};
			}
			case DeviceInformationBridge::OperatingSystemArchitectureType::x64: {
				dosboxDownloadFile = m_dosboxDownloads->getLatestDOSBoxDownloadFile(dosboxVersion.getName(), optionalOperatingSystemType.value(), DeviceInformationBridge::OperatingSystemArchitectureType::x86);

				if(dosboxDownloadFile == nullptr) {
					spdlog::error("Could not find '{}' file download information for '{}' ({} or {}).", dosboxVersion.getName(), magic_enum::enum_name(optionalOperatingSystemType.value()), magic_enum::enum_name(DeviceInformationBridge::OperatingSystemArchitectureType::x64), magic_enum::enum_name(DeviceInformationBridge::OperatingSystemArchitectureType::x86));
					return {};
				}

				break;
			}
		}
	}

	return Utilities::joinPaths(getRemoteDOSBoxDownloadsBaseURL(), dosboxDownloadFile->getFileName());
}

std::string DOSBoxManager::getLatestOriginalDOSBoxVersion() const {
	static const std::string DOSBOX_DOWNLOAD_PAGE_URL("https://www.dosbox.com/download.php?main=1");

	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		return {};
	}

	std::shared_ptr<HTTPRequest> downloadPageRequest(httpService->createRequest(HTTPRequest::Method::Get, DOSBOX_DOWNLOAD_PAGE_URL));
	downloadPageRequest->setConnectionTimeout(5s);
	downloadPageRequest->setNetworkTimeout(10s);

	std::shared_ptr<HTTPResponse> downloadPageResponse(httpService->sendRequestAndWait(downloadPageRequest));

	if(downloadPageResponse->isFailure()) {
		spdlog::error("Failed to retrieve DOSBox download page with error: {}", downloadPageResponse->getErrorMessage());
		return {};
	}
	else if(downloadPageResponse->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(downloadPageResponse->getStatusCode()));
		spdlog::error("Failed to get DOSBox download page ({}{})!", downloadPageResponse->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return {};
	}

	std::string downloadPageHTML(Utilities::tidyHTML(downloadPageResponse->getBodyAsString()));

	if(downloadPageHTML.empty()) {
		spdlog::error("Failed to tidy DOSBox download page HTML.");
		return {};
	}

	downloadPageResponse.reset();

	tinyxml2::XMLDocument document;

	if(document.Parse(downloadPageHTML.c_str(), downloadPageHTML.length()) != tinyxml2::XML_SUCCESS) {
		spdlog::error("Failed to parse DOSBox download page XHTML with error: '{}'.", document.ErrorStr());
		return {};
	}

	const tinyxml2::XMLElement * latestVersionContainerElement = Utilities::findFirstXMLElementContainingText(document.RootElement(), "latest version", false);

	if(latestVersionContainerElement == nullptr) {
		spdlog::error("DOSBox download page parsing failed, could not find latest version container element.");
		return {};
	}

	const tinyxml2::XMLElement * latestVersionLinkElement = Utilities::findFirstXMLElementWithName(latestVersionContainerElement, "a");

	if(latestVersionLinkElement == nullptr) {
		spdlog::error("DOSBox download page parsing failed, could not find latest version link element.");
		return {};
	}

	std::string latestDOSBoxVersion(Utilities::trimString(latestVersionLinkElement->GetText()));

	if(latestDOSBoxVersion.empty()) {
		spdlog::error("Failed to determine latest DOSBox version from download page information.");
		return {};
	}

	return latestDOSBoxVersion;
}

std::string DOSBoxManager::getOriginalDOSBoxDownloadURL(const std::string & version) {
	static const std::string DOSBOX_WINDOWS_INSTALLER_DOWNLOAD_URL_TEMPLATE("https://versaweb.dl.sourceforge.net/project/dosbox/dosbox/{0}/DOSBox{0}-win32-installer.exe");

	if(version.empty()) {
		return {};
	}

	return fmt::format(DOSBOX_WINDOWS_INSTALLER_DOWNLOAD_URL_TEMPLATE, version);
}

std::string DOSBoxManager::getLatestOriginalDOSBoxDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion) const {
	if(!m_initialized) {
		spdlog::error("DOSBox manager not initialized!");
		return {};
	}

	std::string latestOriginalDOSBoxVersion(getLatestOriginalDOSBoxVersion());

	if(latestVersion != nullptr) {
		*latestVersion = latestOriginalDOSBoxVersion;
	}

	return getOriginalDOSBoxDownloadURL(latestOriginalDOSBoxVersion);
}

std::string DOSBoxManager::getLatestDOSBoxStagingDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion) const {
	if(!m_initialized) {
		spdlog::error("DOSBox manager not initialized!");
		return {};
	}

	std::unique_ptr<GitHubRelease> latestRelease(GitHubService::getInstance()->getLatestRelease(DOSBoxVersion::DOSBOX_STAGING.getSourceCodeURL()));

	if(latestRelease == nullptr) {
		return {};
	}

	std::shared_ptr<GitHubReleaseAsset> currentReleaseAsset;
	std::shared_ptr<GitHubReleaseAsset> latestReleaseAsset;

	for(size_t i = 0; i < latestRelease->numberOfAssets(); i++) {
		currentReleaseAsset = latestRelease->getAsset(i);

		if(Utilities::toLowerCase(currentReleaseAsset->getFileName()).find(Utilities::toLowerCase(magic_enum::enum_name(operatingSystemType))) != std::string::npos) {
			latestReleaseAsset = currentReleaseAsset;
			break;
		}
	}

	if(latestReleaseAsset == nullptr) {
		spdlog::error("Could not find '{}' GitHub release asset with matching download file name for '{}'.", DOSBoxVersion::DOSBOX_STAGING.getName(), magic_enum::enum_name(operatingSystemType));
		return {};
	}

	if(latestVersion != nullptr) {
		*latestVersion = latestRelease->getTagName()[0] == 'v' ? latestRelease->getTagName().substr(1) : latestRelease->getTagName();
	}

	return latestReleaseAsset->getDownloadURL();
}

std::string DOSBoxManager::getLatestDOSBoxXDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion) const {
	static const std::string WINDOWS_IDENTIFIER("win");
	static const std::string WINDOWS_X86_ARCHITECTURE_IDENTIFIER("win32");
	static const std::string WINDOWS_X64_ARCHITECTURE_IDENTIFIER("win64");
	static const std::string VISUAL_STUDIO_BUILD_IDENTIFIER("vsbuild");

	if(!m_initialized) {
		spdlog::error("DOSBox manager not initialized!");
		return {};
	}

	std::unique_ptr<GitHubRelease> latestRelease(GitHubService::getInstance()->getLatestRelease(DOSBoxVersion::DOSBOX_X.getSourceCodeURL()));

	if(latestRelease == nullptr) {
		return {};
	}

	std::shared_ptr<GitHubReleaseAsset> currentReleaseAsset;
	std::shared_ptr<GitHubReleaseAsset> latestReleaseAsset;
	std::string operatingSystemIdentifier;
	std::string architectureIdentifier;

	if(operatingSystemType == DeviceInformationBridge::OperatingSystemType::Windows) {
		operatingSystemIdentifier = WINDOWS_IDENTIFIER;

		switch(operatingSystemArchitectureType) {
			case DeviceInformationBridge::OperatingSystemArchitectureType::x86: {
				architectureIdentifier = WINDOWS_X86_ARCHITECTURE_IDENTIFIER;
				break;
			}
			case DeviceInformationBridge::OperatingSystemArchitectureType::x64: {
				architectureIdentifier = WINDOWS_X64_ARCHITECTURE_IDENTIFIER;
				break;
			}
		}
	}
	else {
		operatingSystemIdentifier = Utilities::toLowerCase(magic_enum::enum_name(operatingSystemType));
		architectureIdentifier = Utilities::toLowerCase(magic_enum::enum_name(operatingSystemArchitectureType));
	}

	for(size_t i = 0; i < latestRelease->numberOfAssets(); i++) {
		currentReleaseAsset = latestRelease->getAsset(i);

		if(currentReleaseAsset->getFileName().find(operatingSystemIdentifier) != std::string::npos &&
		   currentReleaseAsset->getFileName().find(architectureIdentifier) != std::string::npos) {

			if(operatingSystemType == DeviceInformationBridge::OperatingSystemType::Windows &&
			   currentReleaseAsset->getFileName().find(VISUAL_STUDIO_BUILD_IDENTIFIER) == std::string::npos) {
				continue;
			}

			latestReleaseAsset = currentReleaseAsset;
			break;
		}
	}

	if(latestReleaseAsset == nullptr &&
	   operatingSystemType == DeviceInformationBridge::OperatingSystemType::Windows &&
	   operatingSystemArchitectureType == DeviceInformationBridge::OperatingSystemArchitectureType::x64) {
		for(size_t i = 0; i < latestRelease->numberOfAssets(); i++) {
			currentReleaseAsset = latestRelease->getAsset(i);

			if(currentReleaseAsset->getFileName().find(operatingSystemIdentifier) != std::string::npos &&
			   currentReleaseAsset->getFileName().find(WINDOWS_X86_ARCHITECTURE_IDENTIFIER) != std::string::npos &&
			   currentReleaseAsset->getFileName().find(VISUAL_STUDIO_BUILD_IDENTIFIER) != std::string::npos) {
				latestReleaseAsset = currentReleaseAsset;
				break;
			}
		}

		if(latestReleaseAsset == nullptr) {
			spdlog::error("Could not find '{}' GitHub release asset with matching download file name for '{}' ({} or {}).", DOSBoxVersion::DOSBOX_X.getName(), magic_enum::enum_name(operatingSystemType), magic_enum::enum_name(DeviceInformationBridge::OperatingSystemArchitectureType::x64), magic_enum::enum_name(DeviceInformationBridge::OperatingSystemArchitectureType::x86));
			return {};
		}
	}

	if(latestReleaseAsset == nullptr) {
		spdlog::error("Could not find '{}' GitHub release asset with matching download file name for '{}'.", DOSBoxVersion::DOSBOX_X.getName(), magic_enum::enum_name(operatingSystemType));
		return {};
	}

	if(latestVersion != nullptr) {
		size_t versionStartIndex = latestRelease->getTagName().find_first_of("0123456789");

		if(versionStartIndex != std::string::npos) {
			*latestVersion = latestRelease->getTagName().substr(versionStartIndex);
		}
		else {
			*latestVersion = latestRelease->getTagName();
		}
	}

	return latestReleaseAsset->getDownloadURL();
}

std::unique_ptr<Archive> DOSBoxManager::downloadLatestDOSBoxVersion(const DOSBoxVersion & dosboxVersion, std::string * latestVersion) const {
	if(!m_initialized) {
		spdlog::error("DOSBox manager not initialized!");
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

	return downloadLatestDOSBoxVersion(dosboxVersion, optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value(), false, latestVersion);
}

std::unique_ptr<Archive> DOSBoxManager::downloadLatestDOSBoxVersion(const DOSBoxVersion & dosboxVersion, DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, bool useFallback, std::string * latestVersion) const {
	if(!m_initialized) {
		spdlog::error("DOSBox manager not initialized!");
		return nullptr;
	}

	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		spdlog::error("HTTP service not initialized!");
		return nullptr;
	}

	std::string latestDOSBoxDownloadURL;

	if(useFallback) {
		spdlog::info("Using fallback {} application package file download URL.", dosboxVersion.getName());

		latestDOSBoxDownloadURL = getLatestDOSBoxDownloadFallbackURL(dosboxVersion, operatingSystemType, operatingSystemArchitectureType, latestVersion);
	}
	else {
		latestDOSBoxDownloadURL = getLatestDOSBoxDownloadURL(dosboxVersion, operatingSystemType, operatingSystemArchitectureType, latestVersion);
	}

	if(latestDOSBoxDownloadURL.empty()) {
		if(!useFallback) {
			return downloadLatestDOSBoxVersion(dosboxVersion, operatingSystemType, operatingSystemArchitectureType, true, latestVersion);
		}

		return nullptr;
	}

	spdlog::info("Downloading {} {} application archive file from: '{}'...", dosboxVersion.getName(), latestVersion != nullptr ? *latestVersion : "", latestDOSBoxDownloadURL);

	std::shared_ptr<HTTPRequest> installerRequest(httpService->createRequest(HTTPRequest::Method::Get, latestDOSBoxDownloadURL));

	std::shared_ptr<HTTPResponse> installerResponse(httpService->sendRequestAndWait(installerRequest));

	if(installerResponse->isFailure()) {
		spdlog::error("Failed to download {} {} application archive file with error: {}", dosboxVersion.getName(), latestVersion != nullptr ? *latestVersion : "", installerResponse->getErrorMessage());

		if(!useFallback) {
			return downloadLatestDOSBoxVersion(dosboxVersion, operatingSystemType, operatingSystemArchitectureType, true, latestVersion);
		}

		return nullptr;
	}
	else if(installerResponse->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(installerResponse->getStatusCode()));
		spdlog::error("Failed to download {} {} application files package ({}{})!", dosboxVersion.getName(), latestVersion != nullptr ? *latestVersion : "", installerResponse->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);

		if(!useFallback) {
			return downloadLatestDOSBoxVersion(dosboxVersion, operatingSystemType, operatingSystemArchitectureType, true, latestVersion);
		}

		return nullptr;
	}

	std::unique_ptr<Archive> dosboxArchive(ArchiveFactoryRegistry::getInstance()->createArchiveFrom(installerResponse->transferBody(), latestDOSBoxDownloadURL));

	if(dosboxArchive == nullptr) {
		spdlog::error("Failed to create archive for {} {} application archive.", dosboxVersion.getName(), latestVersion != nullptr ? *latestVersion : "");

		if(!useFallback) {
			return downloadLatestDOSBoxVersion(dosboxVersion, operatingSystemType, operatingSystemArchitectureType, true, latestVersion);
		}

		return nullptr;
	}

	return std::move(dosboxArchive);
}

bool DOSBoxManager::installLatestDOSBoxVersion(const DOSBoxVersion & dosboxVersion, const std::string & destinationDirectoryPath, bool overwrite) const {
	if(!m_initialized) {
		spdlog::error("DOSBox manager not initialized!");
		return false;
	}

	if(!std::filesystem::exists(std::filesystem::path(destinationDirectoryPath))) {
		std::error_code errorCode;
		std::filesystem::create_directories(destinationDirectoryPath, errorCode);

		if(errorCode) {
			spdlog::error("Failed to create {} installation destination directory structure for path '{}': {}", dosboxVersion.getName(), destinationDirectoryPath, errorCode.message());
			return false;
		}
	}

	std::string latestVersion;
	std::unique_ptr<Archive> dosboxArchive(downloadLatestDOSBoxVersion(dosboxVersion, &latestVersion));

	if(dosboxArchive == nullptr) {
		spdlog::error("Failed to download and create archive for {} {} application archive.", dosboxVersion.getName(), latestVersion);
		return false;
	}

	if(Utilities::areStringsEqualIgnoreCase(dosboxVersion.getName(), DOSBoxVersion::DOSBOX_STAGING.getName())) {
		std::shared_ptr<ArchiveEntry> executableFileArchiveEntry(dosboxArchive->getFirstEntryWithName(dosboxVersion.getExecutableName()));

		if(executableFileArchiveEntry == nullptr) {
			spdlog::error("{} archive is missing executable with file name: '{}'.", dosboxVersion.getName(), dosboxVersion.getExecutableName());
			return false;
		}

		std::string applicationArchiveBasePath(executableFileArchiveEntry->getBasePath());

		return dosboxArchive->extractAllEntriesInSubdirectory(destinationDirectoryPath, applicationArchiveBasePath, true, true, overwrite) != 0;
	}
	else if(Utilities::areStringsEqualIgnoreCase(dosboxVersion.getName(), DOSBoxVersion::DOSBOX_X.getName())) {
		std::vector<std::shared_ptr<ArchiveEntry>> executableFileArchiveEntries(dosboxArchive->getEntriesWithName(dosboxVersion.getExecutableName()));
		std::shared_ptr<ArchiveEntry> sdl2ExecutableArchiveFileEntry;

		for(const std::shared_ptr<ArchiveEntry> executableFileArchiveEntry : executableFileArchiveEntries) {
			if(executableFileArchiveEntry == nullptr) {
				continue;
			}

			if(Utilities::toUpperCase(executableFileArchiveEntry->getBasePath()).find("SDL2") != std::string::npos) {
				sdl2ExecutableArchiveFileEntry = executableFileArchiveEntry;
				break;
			}
		}

		if(sdl2ExecutableArchiveFileEntry == nullptr) {
			spdlog::error("{} archive is missing SDL2 executable with file name: '{}'.", dosboxVersion.getName(), dosboxVersion.getExecutableName());
			return false;
		}

		std::string applicationArchiveBasePath(sdl2ExecutableArchiveFileEntry->getBasePath());

		return dosboxArchive->extractAllEntriesInSubdirectory(destinationDirectoryPath, applicationArchiveBasePath, true, true, overwrite) != 0;
	}

	return dosboxArchive->extractAllEntries(destinationDirectoryPath, overwrite) != 0;
}

bool DOSBoxManager::isDOSBoxVersionDownloadable(const std::string & dosboxVersionName) {
	return Utilities::areStringsEqualIgnoreCase(dosboxVersionName, DOSBoxVersion::DOSBOX.getName()) ||
		   Utilities::areStringsEqualIgnoreCase(dosboxVersionName, DOSBoxVersion::DOSBOX_STAGING.getName()) ||
		   Utilities::areStringsEqualIgnoreCase(dosboxVersionName, DOSBoxVersion::DOSBOX_X.getName());
}
