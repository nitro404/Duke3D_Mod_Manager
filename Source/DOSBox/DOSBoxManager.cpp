#include "DOSBoxManager.h"

#include <Archive/ArchiveFactoryRegistry.h>
#include <GitHub/GitHubService.h>
#include <Manager/SettingsManager.h>
#include <Network/HTTPRequest.h>
#include <Network/HTTPService.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
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
	, m_localMode(false)
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

	if(m_localMode && !loadOrUpdateDOSBoxDownloadList()) {
		return false;
	}

	m_initialized = true;

	return true;
}

bool DOSBoxManager::isUsingLocalMode() const {
	return m_localMode;
}

void DOSBoxManager::setLocalMode(bool localMode) {
	if(m_initialized) {
		spdlog::error("Cannot change local mode after initialization.");
		return;
	}

	m_localMode = localMode;
}

std::shared_ptr<DOSBoxVersionCollection> DOSBoxManager::getDOSBoxVersions() const {
	return m_dosboxVersions;
}

std::string DOSBoxManager::getDOSBoxDownloadsListFilePath() const {
	SettingsManager * settings = SettingsManager::getInstance();

	if(m_localMode) {
		if(settings->dosboxDownloadsListFilePath.empty()) {
			spdlog::error("Missing local DOSBox downloads file path setting.");
			return {};
		}

		return settings->dosboxDownloadsListFilePath;
	}

	if(settings->downloadsDirectoryPath.empty()) {
		spdlog::error("Missing downloads directory path setting.");
		return {};
	}

	if(settings->dosboxDownloadsDirectoryName.empty()) {
		spdlog::error("Missing DOSBox downloads directory name setting.");
		return {};
	}

	if(settings->remoteDOSBoxVersionsListFileName.empty()) {
		spdlog::error("Missing remote DOSBox list file name setting.");
		return {};
	}

	return Utilities::joinPaths(settings->downloadsDirectoryPath, settings->dosboxDownloadsDirectoryName, settings->remoteDOSBoxVersionsListFileName);
}

bool DOSBoxManager::shouldUpdateDOSBoxDownloadList() const {
	if(m_localMode) {
		return false;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	if(!settings->downloadThrottlingEnabled || !settings->dosboxDownloadListLastDownloadedTimestamp.has_value()) {
		return true;
	}

	std::string dosboxDownloadsListFilePath(getDOSBoxDownloadsListFilePath());

	if(dosboxDownloadsListFilePath.empty()) {
		spdlog::error("Failed to determine DOSBox downloads list file path. Are your settings configured correctly?");
		return false;
	}

	if(!std::filesystem::is_regular_file(std::filesystem::path(dosboxDownloadsListFilePath))) {
		return true;
	}

	return std::chrono::system_clock::now() - settings->dosboxDownloadListLastDownloadedTimestamp.value() > settings->dosboxDownloadListUpdateFrequency;
}

bool DOSBoxManager::loadOrUpdateDOSBoxDownloadList(bool forceUpdate) const {
	std::string dosboxDownloadsListFilePath(getDOSBoxDownloadsListFilePath());

	if(dosboxDownloadsListFilePath.empty()) {
		spdlog::error("Failed to determine DOSBox downloads list file path. Are your settings configured correctly?");
		return false;
	}

	if(m_localMode) {
		installStatusChanged("Loading DOSBox downloads list.");

		std::unique_ptr<DOSBoxDownloadCollection> dosboxDownloads(std::make_unique<DOSBoxDownloadCollection>());

		spdlog::info("Loading local DOSBox downloads list file...");

		if(dosboxDownloads->loadFrom(dosboxDownloadsListFilePath) && DOSBoxDownloadCollection::isValid(dosboxDownloads.get())) {
			m_dosboxDownloads = std::move(dosboxDownloads);
			return true;
		}

		spdlog::error("Failed to load local DOSBox downloads list file.");

		return false;
	}

	if(!forceUpdate && std::filesystem::is_regular_file(std::filesystem::path(dosboxDownloadsListFilePath))) {
		installStatusChanged("Loading DOSBox downloads list.");

		std::unique_ptr<DOSBoxDownloadCollection> dosboxDownloads(std::make_unique<DOSBoxDownloadCollection>());

		if(dosboxDownloads->loadFrom(dosboxDownloadsListFilePath) && DOSBoxDownloadCollection::isValid(dosboxDownloads.get())) {
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
	if(m_localMode) {
		return false;
	}

	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		return false;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->remoteDownloadsDirectoryName.empty()) {
		spdlog::error("Missing remote downloads directory name setting.");
		return {};
	}

	if(settings->remoteDOSBoxDownloadsDirectoryName.empty()) {
		spdlog::error("Missing remote DOSBox downloads directory name setting.");
		return {};
	}

	if(settings->remoteDOSBoxVersionsListFileName.empty()) {
		spdlog::error("Missing remote DOSBox versions list file name setting.");
		return {};
	}

	std::string dosboxListRemoteFilePath(Utilities::joinPaths(settings->remoteDownloadsDirectoryName, settings->remoteDOSBoxDownloadsDirectoryName, settings->remoteDOSBoxVersionsListFileName));
	std::string dosboxListURL(Utilities::joinPaths(httpService->getBaseURL(), dosboxListRemoteFilePath));

	installStatusChanged("Downloading DOSBox downloads list.");

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
	else if(response->getStatusCode() == magic_enum::enum_integer(HTTPStatusCode::NotModified)) {
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

	std::string dosboxDownloadsListFilePath(getDOSBoxDownloadsListFilePath());

	if(dosboxDownloadsListFilePath.empty()) {
		spdlog::error("Failed to determine DOSBox downloads list file path. Are your settings configured correctly?");
		return false;
	}

	if(!response->getBody()->writeTo(dosboxDownloadsListFilePath, true)) {
		spdlog::error("Failed to write DOSBox version download collection JSON data to file: '{}'.", dosboxDownloadsListFilePath);
		return false;
	}

	m_dosboxDownloads = std::move(dosboxDownloads);

	settings->fileETags.emplace(settings->remoteDOSBoxVersionsListFileName, response->getETag());
	settings->dosboxDownloadListLastDownloadedTimestamp = std::chrono::system_clock::now();
	settings->save();

	return true;
}

std::string DOSBoxManager::getLatestDOSBoxDownloadURL(const std::string & dosboxVersionID, std::string * latestVersion) const {
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

	return getLatestDOSBoxDownloadURL(dosboxVersionID, optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value(), latestVersion);
}

std::string DOSBoxManager::getLatestDOSBoxDownloadURL(const std::string & dosboxVersionID, DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion) const {
	if(Utilities::areStringsEqualIgnoreCase(dosboxVersionID, DOSBoxVersion::DOSBOX.getID())) {
		return getLatestOriginalDOSBoxDownloadURL(operatingSystemType, operatingSystemArchitectureType, latestVersion);
	}
	else if(Utilities::areStringsEqualIgnoreCase(dosboxVersionID, DOSBoxVersion::DOSBOX_ECE.getID())) {
		return getLatestDOSBoxEnhancedCommunityEditionDownloadURL(operatingSystemType, operatingSystemArchitectureType, latestVersion);
	}
	else if(Utilities::areStringsEqualIgnoreCase(dosboxVersionID, DOSBoxVersion::DOSBOX_STAGING.getID())) {
		return getLatestDOSBoxStagingDownloadURL(operatingSystemType, operatingSystemArchitectureType, latestVersion);
	}
	else if(Utilities::areStringsEqualIgnoreCase(dosboxVersionID, DOSBoxVersion::DOSBOX_X.getID())) {
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

std::string DOSBoxManager::getLatestDOSBoxDownloadFallbackURL(const std::string & dosboxVersionID, DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion) const {
	if(!loadOrUpdateDOSBoxDownloadList()) {
		return {};
	}

	std::shared_ptr<DOSBoxVersion> dosboxVersion(m_dosboxVersions->getDOSBoxVersionWithID(dosboxVersionID));

	if(dosboxVersion == nullptr) {
		spdlog::error("Failed to obtain latest DOSBox download fallback URL for missing or invalid DOSBox version ID: '{}'.", dosboxVersionID);
		return false;
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

	std::shared_ptr<DOSBoxDownloadFile> dosboxDownloadFile(m_dosboxDownloads->getLatestDOSBoxDownloadFile(dosboxVersionID, optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value()));

	if(dosboxDownloadFile == nullptr) {
		switch(optionalOperatingSystemArchitectureType.value()) {
			case DeviceInformationBridge::OperatingSystemArchitectureType::x86: {
				spdlog::error("Could not find '{}' file download information for '{}' ({}).", dosboxVersion->getLongName(), magic_enum::enum_name(optionalOperatingSystemType.value()), magic_enum::enum_name(optionalOperatingSystemArchitectureType.value()));
				return {};
			}
			case DeviceInformationBridge::OperatingSystemArchitectureType::x64: {
				dosboxDownloadFile = m_dosboxDownloads->getLatestDOSBoxDownloadFile(dosboxVersionID, optionalOperatingSystemType.value(), DeviceInformationBridge::OperatingSystemArchitectureType::x86);

				if(dosboxDownloadFile == nullptr) {
					spdlog::error("Could not find '{}' file download information for '{}' ({} or {}).", dosboxVersion->getLongName(), magic_enum::enum_name(optionalOperatingSystemType.value()), magic_enum::enum_name(DeviceInformationBridge::OperatingSystemArchitectureType::x64), magic_enum::enum_name(DeviceInformationBridge::OperatingSystemArchitectureType::x86));
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

struct DOSBoxECEDownload {
	std::string name;
	std::string path;
	std::string version;
	uint64_t fileSize = 0;
	std::chrono::time_point<std::chrono::system_clock> modifiedTimestamp;
	DeviceInformationBridge::OperatingSystemType operatingSystemType = DeviceInformationBridge::OperatingSystemType::Windows;
};

static std::vector<DOSBoxECEDownload> getLatestDOSBoxEnhancedCommunityEditionDownloads() {
	static const std::string DOSBOX_ECE_DOWNLOADS_API_URL("https://yesterplay.net/dosboxece/download/index.php?do=list");
	static const std::string JSON_SUCCESS_PROPERTY_NAME("success");
	static const std::string JSON_DOWNLOAD_LIST_PROPERTY_NAME("results");
	static const std::string JSON_DOWNLOAD_NAME_PROPERTY_NAME("name");
	static const std::string JSON_DOWNLOAD_PATH_PROPERTY_NAME("path");
	static const std::string JSON_DOWNLOAD_FILE_SIZE_PROPERTY_NAME("size");
	static const std::string JSON_DOWNLOAD_MODIFIED_TIMESTAMP_PROPERTY_NAME("mtime");
	static const std::string JSON_DOWNLOAD_DIRECTORY_FLAG_PROPERTY_NAME("is_dir");
	static const std::string LINUX_DOWNLOAD_IDENTIFIER("linux");
	static const std::string SOURCE_CODE_DOWNLOAD_IDENTIFIER("source");
	static const std::string ECE_DOWNLOAD_IDENTIFIER("ece");

	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		return {};
	}

	std::shared_ptr<HTTPRequest> downloadInformationRequest(httpService->createRequest(HTTPRequest::Method::Get, DOSBOX_ECE_DOWNLOADS_API_URL));
	downloadInformationRequest->setConnectionTimeout(5s);
	downloadInformationRequest->setNetworkTimeout(10s);

	std::shared_ptr<HTTPResponse> downloadInformationResponse(httpService->sendRequestAndWait(downloadInformationRequest));

	if(downloadInformationResponse->isFailure()) {
		spdlog::error("Failed to retrieve DOSBox Enhanced Community Edition download list with error: {}", downloadInformationResponse->getErrorMessage());
		return {};
	}
	else if(downloadInformationResponse->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(downloadInformationResponse->getStatusCode()));
		spdlog::error("Failed to get DOSBox Enhanced Community Edition download list ({}{})!", downloadInformationResponse->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return {};
	}

	std::unique_ptr<rapidjson::Document> downloadInformationDocument(downloadInformationResponse->getBodyAsJSON());

	if(downloadInformationDocument == nullptr) {
		spdlog::error("Failed to parse DOSBox Enhanced Community Edition JSON download information.");
		return {};
	}

	downloadInformationResponse.reset();

	if(downloadInformationDocument->HasMember(JSON_SUCCESS_PROPERTY_NAME.c_str())) {
		const rapidjson::Value & successValue = (*downloadInformationDocument)[JSON_SUCCESS_PROPERTY_NAME.c_str()];

		if(!successValue.IsBool()) {
			spdlog::error("Invalid DOSBox Enhanced Community Edition download information '{}' property type '{}', expected 'boolean'.", Utilities::typeToString(successValue.GetType()), JSON_SUCCESS_PROPERTY_NAME);
			return {};
		}

		if(!successValue.GetBool()) {
			spdlog::error("DOSBox Enhanced Community Edition download information request failed, '{}' flag set to false.", JSON_SUCCESS_PROPERTY_NAME);
			return {};
		}
	}
	else {
		spdlog::warn("DOSBox Enhanced Community Edition download information is missing '{}' property, unable to validate if request succeeded.", JSON_SUCCESS_PROPERTY_NAME);
	}

	if(!downloadInformationDocument->HasMember(JSON_DOWNLOAD_LIST_PROPERTY_NAME.c_str())) {
		spdlog::error("DOSBox Enhanced Community Edition download information is missing '{}' property, unable to obtain download list.", JSON_DOWNLOAD_LIST_PROPERTY_NAME);
		return {};
	}

	const rapidjson::Value & downloadListValue = (*downloadInformationDocument)[JSON_DOWNLOAD_LIST_PROPERTY_NAME.c_str()];

	if(!downloadListValue.IsArray()) {
		spdlog::error("Invalid DOSBox Enhanced Community Edition download information '{}' property type '{}', expected 'array'.", JSON_DOWNLOAD_LIST_PROPERTY_NAME, Utilities::typeToString(downloadListValue.GetType()));
		return {};
	}

	std::vector<DOSBoxECEDownload> downloads;

	for(rapidjson::Value::ConstValueIterator i = downloadListValue.Begin(); i != downloadListValue.End(); ++i) {
		const rapidjson::Value & downloadValue = *i;

		if(!downloadValue.IsObject()) {
			spdlog::error("Invalid DOSBox Enhanced Community Edition download information '{}' entry type '{}', expected 'object'.", JSON_DOWNLOAD_LIST_PROPERTY_NAME, Utilities::typeToString(downloadValue.GetType()));
			continue;
		}

		if(downloadValue.HasMember(JSON_DOWNLOAD_DIRECTORY_FLAG_PROPERTY_NAME.c_str()) && downloadValue[JSON_DOWNLOAD_DIRECTORY_FLAG_PROPERTY_NAME.c_str()].GetBool()) {
			continue;
		}

		// parse download name
		if(!downloadValue.HasMember(JSON_DOWNLOAD_NAME_PROPERTY_NAME.c_str())) {
			spdlog::error("DOSBox Enhanced Community Edition download information '{}' entry #{} is missing '{}' property.", JSON_DOWNLOAD_LIST_PROPERTY_NAME, i - downloadListValue.Begin(), JSON_DOWNLOAD_NAME_PROPERTY_NAME);
			continue;
		}

		const rapidjson::Value & nameValue = downloadValue[JSON_DOWNLOAD_NAME_PROPERTY_NAME.c_str()];

		if(!nameValue.IsString()) {
			spdlog::error("Invalid DOSBox Enhanced Community Edition download information '{}' entry #{} '{}' property type '{}', expected 'string'.", JSON_DOWNLOAD_LIST_PROPERTY_NAME, i - downloadListValue.Begin(), JSON_DOWNLOAD_NAME_PROPERTY_NAME, Utilities::typeToString(nameValue.GetType()));
			continue;
		}

		std::string name(nameValue.GetString());

		if(name.empty()) {
			spdlog::error("Invalid empty DOSBox Enhanced Community Edition download information '{}' entry #{} '{}' property value.", JSON_DOWNLOAD_LIST_PROPERTY_NAME, i - downloadListValue.Begin(), JSON_DOWNLOAD_NAME_PROPERTY_NAME);
			continue;
		}

		std::string lowerCaseName(Utilities::toLowerCase(name));

		if(lowerCaseName.find(SOURCE_CODE_DOWNLOAD_IDENTIFIER) != std::string::npos ||
		   lowerCaseName.find(ECE_DOWNLOAD_IDENTIFIER) == std::string::npos) {
			continue;
		}

		// parse download path
		if(!downloadValue.HasMember(JSON_DOWNLOAD_PATH_PROPERTY_NAME.c_str())) {
			spdlog::error("DOSBox Enhanced Community Edition download information '{}' entry #{} is missing '{}' property.", JSON_DOWNLOAD_LIST_PROPERTY_NAME, i - downloadListValue.Begin(), JSON_DOWNLOAD_PATH_PROPERTY_NAME);
			continue;
		}

		const rapidjson::Value & pathValue = downloadValue[JSON_DOWNLOAD_PATH_PROPERTY_NAME.c_str()];

		if(!pathValue.IsString()) {
			spdlog::error("Invalid DOSBox Enhanced Community Edition download information '{}' entry #{} '{}' property type '{}', expected 'string'.", JSON_DOWNLOAD_LIST_PROPERTY_NAME, i - downloadListValue.Begin(), JSON_DOWNLOAD_PATH_PROPERTY_NAME, Utilities::typeToString(pathValue.GetType()));
			continue;
		}

		std::string path(pathValue.GetString());

		if(path.empty()) {
			spdlog::error("Invalid empty DOSBox Enhanced Community Edition download information '{}' entry #{} '{}' property value.", JSON_DOWNLOAD_LIST_PROPERTY_NAME, i - downloadListValue.Begin(), JSON_DOWNLOAD_PATH_PROPERTY_NAME);
			continue;
		}

		// parse download file size
		if(!downloadValue.HasMember(JSON_DOWNLOAD_FILE_SIZE_PROPERTY_NAME.c_str())) {
			spdlog::error("DOSBox Enhanced Community Edition download information '{}' entry #{} is missing '{}' property.", JSON_DOWNLOAD_LIST_PROPERTY_NAME, i - downloadListValue.Begin(), JSON_DOWNLOAD_FILE_SIZE_PROPERTY_NAME);
			continue;
		}

		const rapidjson::Value & downloadFileSizeValue = downloadValue[JSON_DOWNLOAD_FILE_SIZE_PROPERTY_NAME.c_str()];

		if(!downloadFileSizeValue.IsUint64()) {
			spdlog::error("Invalid DOSBox Enhanced Community Edition download information '{}' entry #{} '{}' property type '{}', expected unsigned integer 'number'.", JSON_DOWNLOAD_LIST_PROPERTY_NAME, i - downloadListValue.Begin(), JSON_DOWNLOAD_FILE_SIZE_PROPERTY_NAME, Utilities::typeToString(downloadFileSizeValue.GetType()));
			continue;
		}

		uint64_t fileSize = downloadFileSizeValue.GetUint64();

		// parse download file modified timestamp
		if(!downloadValue.HasMember(JSON_DOWNLOAD_MODIFIED_TIMESTAMP_PROPERTY_NAME.c_str())) {
			spdlog::error("DOSBox Enhanced Community Edition download information '{}' entry #{} is missing '{}' property.", JSON_DOWNLOAD_LIST_PROPERTY_NAME, i - downloadListValue.Begin(), JSON_DOWNLOAD_MODIFIED_TIMESTAMP_PROPERTY_NAME);
			continue;
		}

		const rapidjson::Value & modifiedTimestampValue = downloadValue[JSON_DOWNLOAD_MODIFIED_TIMESTAMP_PROPERTY_NAME.c_str()];

		if(!modifiedTimestampValue.IsUint64()) {
			spdlog::error("Invalid DOSBox Enhanced Community Edition download information '{}' entry #{} '{}' property type '{}', expected unsigned integer 'number'.", JSON_DOWNLOAD_LIST_PROPERTY_NAME, i - downloadListValue.Begin(), JSON_DOWNLOAD_MODIFIED_TIMESTAMP_PROPERTY_NAME, Utilities::typeToString(modifiedTimestampValue.GetType()));
			continue;
		}

		std::chrono::time_point<std::chrono::system_clock> modifiedTimestamp(std::chrono::system_clock::from_time_t(time_t{0}) + std::chrono::milliseconds(modifiedTimestampValue.GetUint64()));

		DeviceInformationBridge::OperatingSystemType operatingSystemType = lowerCaseName.find(LINUX_DOWNLOAD_IDENTIFIER) != std::string::npos ? DeviceInformationBridge::OperatingSystemType::Linux : DeviceInformationBridge::OperatingSystemType::Windows;

		// parse download version
		std::string version;
		size_t versionPlatformNameEndIndex = lowerCaseName.find_last_of("(");
		size_t versionEndIndex = versionPlatformNameEndIndex != std::string::npos ? versionPlatformNameEndIndex - 1 : lowerCaseName.find_last_of(".");
		size_t versionStartIndex = versionPlatformNameEndIndex != std::string::npos ? lowerCaseName.find_last_of(" ", versionEndIndex - 1) : lowerCaseName.find_last_of(" ");

		if(versionStartIndex != std::string::npos && versionEndIndex != std::string::npos) {
			versionStartIndex++;
			versionEndIndex--;
			version = lowerCaseName.substr(versionStartIndex, versionEndIndex - versionStartIndex + 1);
		}

		downloads.push_back(DOSBoxECEDownload{ name, path, version, fileSize, modifiedTimestamp, operatingSystemType });
	}

	return downloads;
}

std::string DOSBoxManager::getLatestDOSBoxEnhancedCommunityEditionDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion) const {
	static const std::string DOSBOX_ECE_DOWNLOAD_BASE_URL("https://yesterplay.net/dosboxece/download");

	std::vector<DOSBoxECEDownload> latestDownloads(getLatestDOSBoxEnhancedCommunityEditionDownloads());

	if(latestDownloads.empty()) {
		spdlog::error("Failed to obtain latest DOSBox Enhanced Community Edition download information.");
		return {};
	}

	const DOSBoxECEDownload * latestDownload = nullptr;

	for(const DOSBoxECEDownload & download : latestDownloads) {
		if(download.operatingSystemType == operatingSystemType) {
			latestDownload = &download;
			break;
		}
	}

	if(latestDownload == nullptr) {
		spdlog::error("DOSBox Enhanced Community Edition is not supported on {}.", magic_enum::enum_name(operatingSystemType));
		return {};
	}

	if(latestVersion != nullptr) {
		*latestVersion = latestDownload->version;
	}

	return Utilities::joinPaths(DOSBOX_ECE_DOWNLOAD_BASE_URL, latestDownload->path);
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
		spdlog::error("Could not find '{}' GitHub release asset with matching download file name for '{}'.", DOSBoxVersion::DOSBOX_STAGING.getLongName(), magic_enum::enum_name(operatingSystemType));
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
			spdlog::error("Could not find '{}' GitHub release asset with matching download file name for '{}' ({} or {}).", DOSBoxVersion::DOSBOX_X.getLongName(), magic_enum::enum_name(operatingSystemType), magic_enum::enum_name(DeviceInformationBridge::OperatingSystemArchitectureType::x64), magic_enum::enum_name(DeviceInformationBridge::OperatingSystemArchitectureType::x86));
			return {};
		}
	}

	if(latestReleaseAsset == nullptr) {
		spdlog::error("Could not find '{}' GitHub release asset with matching download file name for '{}'.", DOSBoxVersion::DOSBOX_X.getLongName(), magic_enum::enum_name(operatingSystemType));
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

std::unique_ptr<Archive> DOSBoxManager::downloadLatestDOSBoxVersion(const std::string & dosboxVersionID, std::string * latestVersion) {
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

	return downloadLatestDOSBoxVersion(dosboxVersionID, optionalOperatingSystemType.value(), optionalOperatingSystemArchitectureType.value(), false, latestVersion);
}

std::unique_ptr<Archive> DOSBoxManager::downloadLatestDOSBoxVersion(const std::string & dosboxVersionID, DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, bool useFallback, std::string * latestVersion) {
	if(!m_initialized) {
		spdlog::error("DOSBox manager not initialized!");
		return nullptr;
	}

	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		spdlog::error("HTTP service not initialized!");
		return nullptr;
	}

	std::shared_ptr<DOSBoxVersion> dosboxVersion(m_dosboxVersions->getDOSBoxVersionWithID(dosboxVersionID));

	if(dosboxVersion == nullptr) {
		spdlog::error("Cannot download latest DOSBox version with missing or invalid ID: '{}'.", dosboxVersionID);
		return false;
	}

	std::string latestDOSBoxDownloadURL;

	if(useFallback) {
		spdlog::info("Using fallback {} application package file download URL.", dosboxVersion->getLongName());

		latestDOSBoxDownloadURL = getLatestDOSBoxDownloadFallbackURL(dosboxVersion->getID(), operatingSystemType, operatingSystemArchitectureType, latestVersion);

		installStatusChanged(fmt::format("Re-trying '{}' application files download using fallback URL.", dosboxVersion->getLongName()));
	}
	else {
		latestDOSBoxDownloadURL = getLatestDOSBoxDownloadURL(dosboxVersion->getID(), operatingSystemType, operatingSystemArchitectureType, latestVersion);

		installStatusChanged(fmt::format("Downloading '{}' application files.", dosboxVersion->getLongName()));
	}

	if(latestDOSBoxDownloadURL.empty()) {
		if(!useFallback) {
			return downloadLatestDOSBoxVersion(dosboxVersion->getID(), operatingSystemType, operatingSystemArchitectureType, true, latestVersion);
		}

		return nullptr;
	}

	spdlog::info("Downloading {} {} application archive file from: '{}'...", dosboxVersion->getLongName(), latestVersion != nullptr ? *latestVersion : "", latestDOSBoxDownloadURL);

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, latestDOSBoxDownloadURL));

	boost::signals2::connection progressConnection(request->progress.connect(std::bind(&DOSBoxManager::onDOSBoxDownloadProgress, this, *dosboxVersion, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

	std::shared_ptr<HTTPResponse> response(httpService->sendRequestAndWait(request));

	progressConnection.disconnect();

	if(response->isFailure()) {
		spdlog::error("Failed to download {} {} application archive file with error: {}", dosboxVersion->getLongName(), latestVersion != nullptr ? *latestVersion : "", response->getErrorMessage());

		if(!useFallback) {
			return downloadLatestDOSBoxVersion(dosboxVersion->getID(), operatingSystemType, operatingSystemArchitectureType, true, latestVersion);
		}

		return nullptr;
	}
	else if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to download {} {} application files package ({}{})!", dosboxVersion->getLongName(), latestVersion != nullptr ? *latestVersion : "", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);

		if(!useFallback) {
			return downloadLatestDOSBoxVersion(dosboxVersion->getID(), operatingSystemType, operatingSystemArchitectureType, true, latestVersion);
		}

		return nullptr;
	}

	std::unique_ptr<Archive> dosboxArchive(ArchiveFactoryRegistry::getInstance()->createArchiveFrom(response->transferBody(), latestDOSBoxDownloadURL));

	if(dosboxArchive == nullptr) {
		spdlog::error("Failed to create archive for {} {} application archive.", dosboxVersion->getLongName(), latestVersion != nullptr ? *latestVersion : "");

		if(!useFallback) {
			return downloadLatestDOSBoxVersion(dosboxVersion->getID(), operatingSystemType, operatingSystemArchitectureType, true, latestVersion);
		}

		return nullptr;
	}

	return std::move(dosboxArchive);
}

bool DOSBoxManager::installLatestDOSBoxVersion(const std::string & dosboxVersionID, const std::string & destinationDirectoryPath, bool overwrite) {
	if(!m_initialized) {
		spdlog::error("DOSBox manager not initialized!");
		return false;
	}

	std::shared_ptr<DOSBoxVersion> dosboxVersion(m_dosboxVersions->getDOSBoxVersionWithID(dosboxVersionID));

	if(dosboxVersion == nullptr) {
		spdlog::error("Cannot install latest DOSBox version with missing or invalid ID: '{}'.", dosboxVersionID);
		return false;
	}

	if(!std::filesystem::exists(std::filesystem::path(destinationDirectoryPath))) {
		std::error_code errorCode;
		std::filesystem::create_directories(destinationDirectoryPath, errorCode);

		if(errorCode) {
			spdlog::error("Failed to create {} installation destination directory structure for path '{}': {}", dosboxVersion->getLongName(), destinationDirectoryPath, errorCode.message());
			return false;
		}
	}

	std::string latestVersion;
	std::unique_ptr<Archive> dosboxArchive(downloadLatestDOSBoxVersion(dosboxVersion->getID(), &latestVersion));

	if(dosboxArchive == nullptr) {
		spdlog::error("Failed to download and create archive for {} {} application archive.", dosboxVersion->getLongName(), latestVersion);
		return false;
	}

	installStatusChanged(fmt::format("Extracting '{}' application files to destination directory.", dosboxVersion->getLongName()));

	if(Utilities::areStringsEqualIgnoreCase(dosboxVersion->getID(), DOSBoxVersion::DOSBOX_STAGING.getID())) {
		std::shared_ptr<ArchiveEntry> executableFileArchiveEntry(dosboxArchive->getFirstEntryWithName(dosboxVersion->getExecutableName()));

		if(executableFileArchiveEntry == nullptr) {
			spdlog::error("{} archive is missing executable with file name: '{}'.", dosboxVersion->getLongName(), dosboxVersion->getExecutableName());
			return false;
		}

		std::string applicationArchiveBasePath(executableFileArchiveEntry->getBasePath());

		return dosboxArchive->extractAllEntriesInSubdirectory(destinationDirectoryPath, applicationArchiveBasePath, true, true, overwrite) != 0;
	}
	else if(Utilities::areStringsEqualIgnoreCase(dosboxVersion->getID(), DOSBoxVersion::DOSBOX_X.getID())) {
		std::vector<std::shared_ptr<ArchiveEntry>> executableFileArchiveEntries(dosboxArchive->getEntriesWithName(dosboxVersion->getExecutableName()));
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
			spdlog::error("{} archive is missing SDL2 executable with file name: '{}'.", dosboxVersion->getLongName(), dosboxVersion->getExecutableName());
			return false;
		}

		std::string applicationArchiveBasePath(sdl2ExecutableArchiveFileEntry->getBasePath());

		return dosboxArchive->extractAllEntriesInSubdirectory(destinationDirectoryPath, applicationArchiveBasePath, true, true, overwrite) != 0;
	}

	return dosboxArchive->extractAllEntries(destinationDirectoryPath, overwrite) != 0;
}

bool DOSBoxManager::isDOSBoxVersionDownloadable(const DOSBoxVersion & dosboxVersion) {
	return isDOSBoxVersionDownloadable(dosboxVersion.getID());
}

bool DOSBoxManager::isDOSBoxVersionDownloadable(const std::string & dosboxVersionID) {
	if(dosboxVersionID.empty()) {
		return false;
	}

	return Utilities::areStringsEqualIgnoreCase(dosboxVersionID, DOSBoxVersion::DOSBOX.getID()) ||
		   Utilities::areStringsEqualIgnoreCase(dosboxVersionID, DOSBoxVersion::DOSBOX_ECE.getID()) ||
		   Utilities::areStringsEqualIgnoreCase(dosboxVersionID, DOSBoxVersion::DOSBOX_STAGING.getID()) ||
		   Utilities::areStringsEqualIgnoreCase(dosboxVersionID, DOSBoxVersion::DOSBOX_X.getID());
}

void DOSBoxManager::onDOSBoxDownloadProgress(DOSBoxVersion & dosboxVersion, HTTPRequest & request, size_t numberOfBytesDownloaded, size_t totalNumberOfBytes) {
	dosboxDownloadProgress(dosboxVersion, request, numberOfBytesDownloaded, totalNumberOfBytes);
}
