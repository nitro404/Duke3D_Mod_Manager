#include "DOSBoxVersion.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/TimeUtilities.h>
#include <Utilities/RapidJSONUtilities.h>

#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <array>
#include <filesystem>
#include <sstream>

static constexpr const char * JSON_ID_PROPERTY_NAME = "id";
static constexpr const char * JSON_LONG_NAME_PROPERTY_NAME = "longName";
static constexpr const char * JSON_SHORT_NAME_PROPERTY_NAME = "shortName";
static constexpr const char * JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME = "installed";
static constexpr const char * JSON_LAST_LAUNCHED_TIMESTAMP_PROPERTY_NAME = "lastLaunched";
static constexpr const char * JSON_REMOVABLE_PROPERTY_NAME = "removable";
static constexpr const char * JSON_RENAMABLE_PROPERTY_NAME = "renamable";
static constexpr const char * JSON_EXECUTABLE_NAME_PROPERTY_NAME = "executableName";
static constexpr const char * JSON_LAUNCH_ARGUMENTS_PROPERTY_NAME = "launchArguments";
static constexpr const char * JSON_DIRECTORY_PATH_PROPERTY_NAME = "directoryPath";
static constexpr const char * JSON_WEBSITE_PROPERTY_NAME = "website";
static constexpr const char * JSON_SOURCE_CODE_URL_PROPERTY_NAME = "sourceCodeURL";
static constexpr const char * JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME = "supportedOperatingSystems";
static const std::array<std::string_view, 12> JSON_PROPERTY_NAMES = {
	JSON_ID_PROPERTY_NAME,
	JSON_LONG_NAME_PROPERTY_NAME,
	JSON_SHORT_NAME_PROPERTY_NAME,
	JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME,
	JSON_LAST_LAUNCHED_TIMESTAMP_PROPERTY_NAME,
	JSON_REMOVABLE_PROPERTY_NAME,
	JSON_EXECUTABLE_NAME_PROPERTY_NAME,
	JSON_LAUNCH_ARGUMENTS_PROPERTY_NAME,
	JSON_DIRECTORY_PATH_PROPERTY_NAME,
	JSON_WEBSITE_PROPERTY_NAME,
	JSON_SOURCE_CODE_URL_PROPERTY_NAME,
	JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME
};

static const std::string DEFAULT_EXECUTABLE_FILE_NAME("DOSBox.exe");

const DOSBoxVersion DOSBoxVersion::DOSBOX        ("dosbox",         "DOSBox",                            "DOSBox",         false, DEFAULT_EXECUTABLE_FILE_NAME, "", "https://www.dosbox.com",           "https://sourceforge.net/p/dosbox/code-0/HEAD/tree/dosbox/trunk", { DeviceInformationBridge::OperatingSystemType::Windows, DeviceInformationBridge::OperatingSystemType::Linux, DeviceInformationBridge::OperatingSystemType::MacOS });
const DOSBoxVersion DOSBoxVersion::DOSBOX_ECE    ("dosbox_ece",     "DOSBox Enhanced Community Edition", "DOSBox ECE",     true,  DEFAULT_EXECUTABLE_FILE_NAME, "", "https://yesterplay.net/dosboxece", "",                                                               { DeviceInformationBridge::OperatingSystemType::Windows, DeviceInformationBridge::OperatingSystemType::Linux });
const DOSBoxVersion DOSBoxVersion::DOSBOX_STAGING("dosbox_staging", "DOSBox Staging",                    "DOSBox Staging", true,  "dosbox.exe",                 "", "https://dosbox-staging.github.io", "https://github.com/dosbox-staging/dosbox-staging",               { DeviceInformationBridge::OperatingSystemType::Windows, DeviceInformationBridge::OperatingSystemType::Linux, DeviceInformationBridge::OperatingSystemType::MacOS });
const DOSBoxVersion DOSBoxVersion::DOSBOX_X      ("dosbox-x",       "DOSBox-X",                          "DOSBox-X",       true,  "dosbox-x.exe",               "", "https://dosbox-x.com",             "https://github.com/joncampbell123/dosbox-x",                     { DeviceInformationBridge::OperatingSystemType::Windows, DeviceInformationBridge::OperatingSystemType::Linux, DeviceInformationBridge::OperatingSystemType::MacOS });

const std::vector<const DOSBoxVersion *> DOSBoxVersion::DEFAULT_DOSBOX_VERSIONS = {
	&DOSBOX,
	&DOSBOX_ECE,
	&DOSBOX_STAGING,
	&DOSBOX_X
};

DOSBoxVersion::DOSBoxVersion()
	: m_removable(true)
	, m_executableName(DEFAULT_EXECUTABLE_FILE_NAME)
	, m_dosboxConfiguration(std::make_shared<DOSBoxConfiguration>())
	, m_modified(false) { }

DOSBoxVersion::DOSBoxVersion(const std::string & id, const std::string & longName, const std::string & shortName, bool removable, const std::string & executableName, const std::string & directoryPath, const std::string & website, const std::string & sourceCodeURL, const std::vector<DeviceInformationBridge::OperatingSystemType> & supportedOperatingSystems, const DOSBoxConfiguration & dosboxConfiguration)
	: m_id(id)
	, m_longName(longName)
	, m_shortName(shortName)
	, m_removable(removable)
	, m_executableName(executableName)
	, m_directoryPath(directoryPath)
	, m_website(website)
	, m_sourceCodeURL(sourceCodeURL)
	, m_dosboxConfiguration(std::make_shared<DOSBoxConfiguration>(dosboxConfiguration))
	, m_modified(false) {
	if(!m_directoryPath.empty()) {
		m_installedTimePoint = std::chrono::system_clock::now();
	}

	addSupportedOperatingSystems(supportedOperatingSystems);

	setModified(false);
}

DOSBoxVersion::DOSBoxVersion(DOSBoxVersion && dosboxVersion) noexcept
	: m_id(std::move(dosboxVersion.m_id))
	, m_longName(std::move(dosboxVersion.m_longName))
	, m_shortName(std::move(dosboxVersion.m_shortName))
	, m_installedTimePoint(std::move(dosboxVersion.m_installedTimePoint))
	, m_lastLaunchedTimePoint(std::move(dosboxVersion.m_lastLaunchedTimePoint))
	, m_removable(dosboxVersion.m_removable)
	, m_executableName(std::move(dosboxVersion.m_executableName))
	, m_launchArguments(std::move(dosboxVersion.m_launchArguments))
	, m_website(std::move(dosboxVersion.m_website))
	, m_sourceCodeURL(std::move(dosboxVersion.m_sourceCodeURL))
	, m_supportedOperatingSystems(std::move(dosboxVersion.m_supportedOperatingSystems))
	, m_dosboxConfiguration(std::move(dosboxVersion.m_dosboxConfiguration))
	, m_modified(false) { }

DOSBoxVersion::DOSBoxVersion(const DOSBoxVersion & dosboxVersion)
	: m_id(dosboxVersion.m_id)
	, m_longName(dosboxVersion.m_longName)
	, m_shortName(dosboxVersion.m_shortName)
	, m_installedTimePoint(dosboxVersion.m_installedTimePoint)
	, m_lastLaunchedTimePoint(dosboxVersion.m_lastLaunchedTimePoint)
	, m_removable(dosboxVersion.m_removable)
	, m_executableName(dosboxVersion.m_executableName)
	, m_launchArguments(dosboxVersion.m_launchArguments)
	, m_directoryPath(dosboxVersion.m_directoryPath)
	, m_website(dosboxVersion.m_website)
	, m_sourceCodeURL(dosboxVersion.m_sourceCodeURL)
	, m_supportedOperatingSystems(dosboxVersion.m_supportedOperatingSystems)
	, m_dosboxConfiguration(dosboxVersion.m_dosboxConfiguration)
	, m_modified(false) { }

DOSBoxVersion & DOSBoxVersion::operator = (DOSBoxVersion && dosboxVersion) noexcept {
	if(this != &dosboxVersion) {
		m_id = std::move(dosboxVersion.m_id);
		m_longName = std::move(dosboxVersion.m_longName);
		m_shortName = std::move(dosboxVersion.m_shortName);
		m_installedTimePoint = std::move(dosboxVersion.m_installedTimePoint);
		m_lastLaunchedTimePoint = std::move(dosboxVersion.m_lastLaunchedTimePoint);
		m_removable = dosboxVersion.m_removable;
		m_executableName = std::move(dosboxVersion.m_executableName);
		m_launchArguments = std::move(dosboxVersion.m_launchArguments);
		m_directoryPath = std::move(dosboxVersion.m_directoryPath);
		m_website = std::move(dosboxVersion.m_website);
		m_sourceCodeURL = std::move(dosboxVersion.m_sourceCodeURL);
		m_supportedOperatingSystems = std::move(dosboxVersion.m_supportedOperatingSystems);
		m_dosboxConfiguration = std::move(dosboxVersion.m_dosboxConfiguration);

		setModified(true);
	}

	return *this;
}

DOSBoxVersion & DOSBoxVersion::operator = (const DOSBoxVersion & dosboxVersion) {
	m_id = dosboxVersion.m_id;
	m_longName = dosboxVersion.m_longName;
	m_shortName = dosboxVersion.m_shortName;
	m_installedTimePoint = dosboxVersion.m_installedTimePoint;
	m_lastLaunchedTimePoint = dosboxVersion.m_lastLaunchedTimePoint;
	m_removable = dosboxVersion.m_removable;
	m_executableName = dosboxVersion.m_executableName;
	m_launchArguments = dosboxVersion.m_launchArguments;
	m_directoryPath = dosboxVersion.m_directoryPath;
	m_website = dosboxVersion.m_website;
	m_sourceCodeURL = dosboxVersion.m_sourceCodeURL;
	m_supportedOperatingSystems = dosboxVersion.m_supportedOperatingSystems;
	m_dosboxConfiguration = dosboxVersion.m_dosboxConfiguration;

	setModified(true);

	return *this;
}

DOSBoxVersion::~DOSBoxVersion() = default;

bool DOSBoxVersion::isModified() const {
	return m_modified;
}

void DOSBoxVersion::setModified(bool value) {
	m_modified = value;

	modified(*this);
}

bool DOSBoxVersion::hasID() const {
	return !m_id.empty();
}

const std::string & DOSBoxVersion::getID() const {
	return m_id;
}

bool DOSBoxVersion::setID(const std::string & id) {
	if(Utilities::areStringsEqual(m_id, id)) {
		return true;
	}

	m_id = id;

	setModified(true);

	return true;
}

bool DOSBoxVersion::hasLongName() const {
	return !m_longName.empty();
}

const std::string & DOSBoxVersion::getLongName() const {
	return m_longName;
}

bool DOSBoxVersion::setLongName(const std::string & longName) {
	if(Utilities::areStringsEqual(m_longName, longName)) {
		return true;
	}

	m_longName = longName;

	setModified(true);

	return true;
}

bool DOSBoxVersion::hasShortName() const {
	return !m_shortName.empty();
}

const std::string & DOSBoxVersion::getShortName() const {
	return m_shortName;
}

bool DOSBoxVersion::setShortName(const std::string & shortName) {
	if(Utilities::areStringsEqual(m_shortName, shortName)) {
		return true;
	}

	m_shortName = shortName;

	setModified(true);

	return true;
}

bool DOSBoxVersion::hasInstalledTimePoint() const {
	return m_installedTimePoint.has_value();
}

const std::optional<std::chrono::time_point<std::chrono::system_clock>> & DOSBoxVersion::getInstalledTimePoint() const {
	return m_installedTimePoint;
}

void DOSBoxVersion::setInstalledTimePoint(std::chrono::time_point<std::chrono::system_clock> installedTimePoint) {
	if(m_installedTimePoint == installedTimePoint) {
		return;
	}

	m_installedTimePoint = installedTimePoint;

	setModified(true);
}

void DOSBoxVersion::clearInstalledTimePoint() {
	if(!m_installedTimePoint.has_value()) {
		return;
	}

	m_installedTimePoint.reset();

	setModified(true);
}

bool DOSBoxVersion::hasLastLaunchedTimePoint() const {
	return m_lastLaunchedTimePoint.has_value();
}

const std::optional<std::chrono::time_point<std::chrono::system_clock>> & DOSBoxVersion::getLastLaunchedTimePoint() const {
	return m_lastLaunchedTimePoint;
}

void DOSBoxVersion::setLastLaunchedTimePoint(std::chrono::time_point<std::chrono::system_clock> lastLaunchedTimePoint) {
	if(m_lastLaunchedTimePoint == lastLaunchedTimePoint) {
		return;
	}

	m_lastLaunchedTimePoint = lastLaunchedTimePoint;

	setModified(true);
}

void DOSBoxVersion::updateLastLaunchedTimePoint() {
	setLastLaunchedTimePoint(std::chrono::system_clock::now());
}

void DOSBoxVersion::clearLastLaunchedTimePoint() {
	if(!m_lastLaunchedTimePoint.has_value()) {
		return;
	}

	m_lastLaunchedTimePoint.reset();

	setModified(true);
}

bool DOSBoxVersion::isRemovable() const {
	return m_removable;
}

const std::string & DOSBoxVersion::getExecutableName() const {
	return m_executableName;
}

void DOSBoxVersion::setExecutableName(const std::string & executableName) {
	if(Utilities::areStringsEqual(m_executableName, executableName)) {
		return;
	}

	m_executableName = executableName;

	setModified(true);
}

bool DOSBoxVersion::hasLaunchArguments() const {
	return !m_launchArguments.empty();
}

const std::string & DOSBoxVersion::getLaunchArguments() const {
	return m_launchArguments;
}

void DOSBoxVersion::setLaunchArguments(const std::string & arguments) {
	if(Utilities::areStringsEqual(m_launchArguments, arguments)) {
		return;
	}

	m_launchArguments = arguments;

	setModified(true);
}

void DOSBoxVersion::clearLaunchArguments() {
	if(m_launchArguments.empty()) {
		return;
	}

	m_launchArguments = "";

	setModified(true);
}

bool DOSBoxVersion::hasDirectoryPath() const {
	return !m_directoryPath.empty();
}

const std::string & DOSBoxVersion::getDirectoryPath() const {
	return m_directoryPath;
}

void DOSBoxVersion::setDirectoryPath(const std::string & directoryPath) {
	if(Utilities::areStringsEqual(m_directoryPath, directoryPath)) {
		return;
	}

	m_directoryPath = directoryPath;

	if(m_directoryPath.empty()) {
		m_installedTimePoint.reset();
	}
	else {
		m_installedTimePoint = std::chrono::system_clock::now();
	}

	setModified(true);
}

const std::string & DOSBoxVersion::getWebsite() const {
	return m_website;
}

void DOSBoxVersion::setWebsite(const std::string & website) {
	if(Utilities::areStringsEqual(m_website, website)) {
		return;
	}

	m_website = website;

	setModified(true);
}

const std::string & DOSBoxVersion::getSourceCodeURL() const {
	return m_sourceCodeURL;
}

void DOSBoxVersion::setSourceCodeURL(const std::string & sourceCodeURL) {
	if(Utilities::areStringsEqual(m_sourceCodeURL, sourceCodeURL)) {
		return;
	}

	m_sourceCodeURL = sourceCodeURL;

	setModified(true);
}

std::shared_ptr<DOSBoxConfiguration> DOSBoxVersion::getDOSBoxConfiguration() const {
	return m_dosboxConfiguration;
}

bool DOSBoxVersion::setDOSBoxConfiguration(const DOSBoxConfiguration & dosboxConfiguration) {
	return m_dosboxConfiguration->setConfiguration(dosboxConfiguration);
}

void DOSBoxVersion::resetDOSBoxConfigurationToDefault() {
	std::shared_ptr<const DOSBoxConfiguration> defaultDOSBoxConfiguration(getDefaultDOSBoxConfiguration());

	if(defaultDOSBoxConfiguration != nullptr) {
		m_dosboxConfiguration->setConfiguration(*defaultDOSBoxConfiguration);
	}
	else {
		m_dosboxConfiguration->clear();
	}
}

std::shared_ptr<const DOSBoxConfiguration> DOSBoxVersion::getDefaultDOSBoxConfiguration() const {
	for(const DOSBoxVersion * dosboxVersion : DEFAULT_DOSBOX_VERSIONS) {
		if(Utilities::areStringsEqualIgnoreCase(m_id, dosboxVersion->getID())) {
			return dosboxVersion->getDOSBoxConfiguration();
		}
	}

	return nullptr;
}

size_t DOSBoxVersion::numberOfSupportedOperatingSystems() const {
	return m_supportedOperatingSystems.size();
}

bool DOSBoxVersion::hasSupportedOperatingSystem(DeviceInformationBridge::OperatingSystemType operatingSystem) const {
	return std::find(m_supportedOperatingSystems.cbegin(), m_supportedOperatingSystems.cend(), operatingSystem) != m_supportedOperatingSystems.cend();
}

bool DOSBoxVersion::hasSupportedOperatingSystemWithName(const std::string & operatingSystemName) const {
	std::optional<DeviceInformationBridge::OperatingSystemType> optionalOperatingSystem(magic_enum::enum_cast<DeviceInformationBridge::OperatingSystemType>(operatingSystemName));

	if(!optionalOperatingSystem.has_value()) {
		return std::numeric_limits<size_t>::max();
	}

	return std::find(m_supportedOperatingSystems.cbegin(), m_supportedOperatingSystems.cend(), optionalOperatingSystem.value()) != m_supportedOperatingSystems.cend();
}

size_t DOSBoxVersion::indexOfSupportedOperatingSystem(DeviceInformationBridge::OperatingSystemType operatingSystem) const {
	const auto operatingSystemIterator = std::find(m_supportedOperatingSystems.cbegin(), m_supportedOperatingSystems.cend(), operatingSystem);

	if(operatingSystemIterator == m_supportedOperatingSystems.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return operatingSystemIterator - m_supportedOperatingSystems.cbegin();
}

size_t DOSBoxVersion::indexOfSupportedOperatingSystemWithName(const std::string & operatingSystemName) const {
	std::optional<DeviceInformationBridge::OperatingSystemType> optionalOperatingSystem(magic_enum::enum_cast<DeviceInformationBridge::OperatingSystemType>(operatingSystemName));

	if(!optionalOperatingSystem.has_value()) {
		return std::numeric_limits<size_t>::max();
	}

	return indexOfSupportedOperatingSystem(optionalOperatingSystem.value());
}

std::optional<DeviceInformationBridge::OperatingSystemType> DOSBoxVersion::getSupportedOperatingSystem(size_t index) const {
	if(index < 0 || index >= m_supportedOperatingSystems.size()) {
		return {};
	}

	return m_supportedOperatingSystems[index];
}

const std::vector<DeviceInformationBridge::OperatingSystemType> & DOSBoxVersion::getSupportedOperatingSystems() const {
	return m_supportedOperatingSystems;
}

bool DOSBoxVersion::addSupportedOperatingSystem(DeviceInformationBridge::OperatingSystemType operatingSystem) {
	if(hasSupportedOperatingSystem(operatingSystem)) {
		return false;
	}

	m_supportedOperatingSystems.push_back(operatingSystem);

	setModified(true);

	return true;
}

bool DOSBoxVersion::addSupportedOperatingSystemWithName(const std::string & operatingSystemName) {
	std::optional<DeviceInformationBridge::OperatingSystemType> optionalOperatingSystem(magic_enum::enum_cast<DeviceInformationBridge::OperatingSystemType>(operatingSystemName));

	if(!optionalOperatingSystem.has_value()) {
		return false;
	}

	return addSupportedOperatingSystem(optionalOperatingSystem.value());
}

size_t DOSBoxVersion::addSupportedOperatingSystems(const std::vector<DeviceInformationBridge::OperatingSystemType> & operatingSystems) {
	size_t numberOfOperatingSystemsAdded = 0;

	for(DeviceInformationBridge::OperatingSystemType operatingSystem : operatingSystems) {
		if(addSupportedOperatingSystem(operatingSystem)) {
			numberOfOperatingSystemsAdded++;
		}
	}

	return numberOfOperatingSystemsAdded;
}

bool DOSBoxVersion::removeSupportedOperatingSystem(size_t index) {
	if(index < 0 || index >= m_supportedOperatingSystems.size()) {
		return false;
	}

	m_supportedOperatingSystems.erase(m_supportedOperatingSystems.cbegin() + index);

	setModified(true);

	return true;
}

bool DOSBoxVersion::removeSupportedOperatingSystem(DeviceInformationBridge::OperatingSystemType operatingSystem) {
	return removeSupportedOperatingSystem(indexOfSupportedOperatingSystem(operatingSystem));
}

bool DOSBoxVersion::removeSupportedOperatingSystemWithName(const std::string & operatingSystemName) {
	return removeSupportedOperatingSystem(indexOfSupportedOperatingSystemWithName(operatingSystemName));
}

void DOSBoxVersion::clearSupportedOperatingSystems() {
	if(m_supportedOperatingSystems.empty()) {
		return;
	}

	m_supportedOperatingSystems.clear();

	setModified(true);
}

void DOSBoxVersion::addMetadata(std::map<std::string, std::any> & metadata) const {
	metadata["dosboxID"] = m_id;
	metadata["longName"] = m_longName;
	metadata["shortName"] = m_shortName;

	if(m_installedTimePoint.has_value()) {
		metadata["installedAt"] = Utilities::timePointToString(m_installedTimePoint.value(), Utilities::TimeFormat::ISO8601);
	}

	if(m_lastLaunchedTimePoint.has_value()) {
		metadata["lastLaunchedAt"] = Utilities::timePointToString(m_lastLaunchedTimePoint.value(), Utilities::TimeFormat::ISO8601);
	}

	metadata["executableName"] = m_executableName;

	if(!m_launchArguments.empty()) {
		metadata["launchArguments"] = m_launchArguments;
	}

	metadata["hasDirectoryPath"] = !m_directoryPath.empty();
	metadata["numberOfSupportedOperatingSystems"] = m_supportedOperatingSystems.size();
}

bool DOSBoxVersion::checkForMissingExecutable() const {
	if(!isConfigured()) {
		return false;
	}

	std::string fullExecutablePath(Utilities::joinPaths(m_directoryPath, m_executableName));

	if(!std::filesystem::is_regular_file(std::filesystem::path(fullExecutablePath))) {
		spdlog::error("Missing '{}' executable: '{}'.", m_longName, fullExecutablePath);
		return true;
	}

	return false;
}

rapidjson::Value DOSBoxVersion::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value dosboxVersionValue(rapidjson::kObjectType);

	rapidjson::Value idValue(m_id.c_str(), allocator);
	dosboxVersionValue.AddMember(rapidjson::StringRef(JSON_ID_PROPERTY_NAME), idValue, allocator);

	rapidjson::Value longNameValue(m_longName.c_str(), allocator);
	dosboxVersionValue.AddMember(rapidjson::StringRef(JSON_LONG_NAME_PROPERTY_NAME), longNameValue, allocator);

	rapidjson::Value shortNameValue(m_shortName.c_str(), allocator);
	dosboxVersionValue.AddMember(rapidjson::StringRef(JSON_SHORT_NAME_PROPERTY_NAME), shortNameValue, allocator);

	if(m_installedTimePoint.has_value()) {
		rapidjson::Value installedTimestampValue(Utilities::timePointToString(m_installedTimePoint.value(), Utilities::TimeFormat::ISO8601).c_str(), allocator);
		dosboxVersionValue.AddMember(rapidjson::StringRef(JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME), installedTimestampValue, allocator);
	}

	if(m_lastLaunchedTimePoint.has_value()) {
		rapidjson::Value lastLaunchedTimestampValue(Utilities::timePointToString(m_lastLaunchedTimePoint.value(), Utilities::TimeFormat::ISO8601).c_str(), allocator);
		dosboxVersionValue.AddMember(rapidjson::StringRef(JSON_LAST_LAUNCHED_TIMESTAMP_PROPERTY_NAME), lastLaunchedTimestampValue, allocator);
	}

	dosboxVersionValue.AddMember(rapidjson::StringRef(JSON_REMOVABLE_PROPERTY_NAME), rapidjson::Value(m_removable), allocator);

	rapidjson::Value executableNameValue(m_executableName.c_str(), allocator);
	dosboxVersionValue.AddMember(rapidjson::StringRef(JSON_EXECUTABLE_NAME_PROPERTY_NAME), executableNameValue, allocator);

	if(!m_launchArguments.empty()) {
		rapidjson::Value launchArgumentsValue(m_launchArguments.c_str(), allocator);
		dosboxVersionValue.AddMember(rapidjson::StringRef(JSON_LAUNCH_ARGUMENTS_PROPERTY_NAME), launchArgumentsValue, allocator);
	}

	rapidjson::Value directoryPathValue(m_directoryPath.c_str(), allocator);
	dosboxVersionValue.AddMember(rapidjson::StringRef(JSON_DIRECTORY_PATH_PROPERTY_NAME), directoryPathValue, allocator);

	if(!m_website.empty()) {
		rapidjson::Value websiteValue(m_website.c_str(), allocator);
		dosboxVersionValue.AddMember(rapidjson::StringRef(JSON_WEBSITE_PROPERTY_NAME), websiteValue, allocator);
	}

	if(!m_sourceCodeURL.empty()) {
		rapidjson::Value sourceCodeURLValue(m_sourceCodeURL.c_str(), allocator);
		dosboxVersionValue.AddMember(rapidjson::StringRef(JSON_SOURCE_CODE_URL_PROPERTY_NAME), sourceCodeURLValue, allocator);
	}

	if(!m_supportedOperatingSystems.empty()) {
		rapidjson::Value supportedOperatingSystemsValue(rapidjson::kArrayType);
		supportedOperatingSystemsValue.Reserve(m_supportedOperatingSystems.size(), allocator);

		for(std::vector<DeviceInformationBridge::OperatingSystemType>::const_iterator i = m_supportedOperatingSystems.cbegin(); i != m_supportedOperatingSystems.cend(); ++i) {
			rapidjson::Value supportedOperatingSystemValue(std::string(magic_enum::enum_name(*i)).c_str(), allocator);
			supportedOperatingSystemsValue.PushBack(supportedOperatingSystemValue, allocator);
		}

		dosboxVersionValue.AddMember(rapidjson::StringRef(JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME), supportedOperatingSystemsValue, allocator);
	}

	return dosboxVersionValue;
}

std::unique_ptr<DOSBoxVersion> DOSBoxVersion::parseFrom(const rapidjson::Value & dosboxVersionValue) {
	if(!dosboxVersionValue.IsObject()) {
		spdlog::error("Invalid DOSBox version type: '{}', expected 'object'.", Utilities::typeToString(dosboxVersionValue.GetType()));
		return nullptr;
	}

	// check for unhandled DOSBox version properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = dosboxVersionValue.MemberBegin(); i != dosboxVersionValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("DOSBox version has unexpected property '{}'.", i->name.GetString());
		}
	}

	// parse DOSBox version id
	if(!dosboxVersionValue.HasMember(JSON_ID_PROPERTY_NAME)) {
		spdlog::error("DOSBox version is missing '{}' property.", JSON_ID_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & idValue = dosboxVersionValue[JSON_ID_PROPERTY_NAME];

	if(!idValue.IsString()) {
		spdlog::error("DOSBox version has an invalid '{}' property type: '{}', expected 'string'.", JSON_ID_PROPERTY_NAME, Utilities::typeToString(idValue.GetType()));
		return nullptr;
	}

	std::string id(Utilities::trimString(idValue.GetString()));

	if(id.empty()) {
		spdlog::error("DOSBox version '{}' property cannot be empty.", JSON_ID_PROPERTY_NAME);
		return nullptr;
	}

	// parse DOSBox version long name
	if(!dosboxVersionValue.HasMember(JSON_LONG_NAME_PROPERTY_NAME)) {
		spdlog::error("DOSBox version is missing '{}' property.", JSON_LONG_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & longNameValue = dosboxVersionValue[JSON_LONG_NAME_PROPERTY_NAME];

	if(!longNameValue.IsString()) {
		spdlog::error("DOSBox version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_LONG_NAME_PROPERTY_NAME, Utilities::typeToString(longNameValue.GetType()));
		return nullptr;
	}

	std::string longName(Utilities::trimString(longNameValue.GetString()));

	if(longName.empty()) {
		spdlog::error("DOSBox version with ID '{}' '{}' property cannot be empty.", id, JSON_LONG_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse DOSBox version short name
	if(!dosboxVersionValue.HasMember(JSON_SHORT_NAME_PROPERTY_NAME)) {
		spdlog::error("DOSBox version with ID '{}' is missing '{}' property.", id, JSON_SHORT_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & shortNameValue = dosboxVersionValue[JSON_SHORT_NAME_PROPERTY_NAME];

	if(!shortNameValue.IsString()) {
		spdlog::error("DOSBox version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_SHORT_NAME_PROPERTY_NAME, Utilities::typeToString(shortNameValue.GetType()));
		return nullptr;
	}

	std::string shortName(Utilities::trimString(shortNameValue.GetString()));

	if(shortName.empty()) {
		spdlog::error("DOSBox version with ID '{}' '{}' property cannot be empty.", id, JSON_SHORT_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse removable property
	bool removable = true;

	if(dosboxVersionValue.HasMember(JSON_REMOVABLE_PROPERTY_NAME)) {
		const rapidjson::Value & removableValue = dosboxVersionValue[JSON_REMOVABLE_PROPERTY_NAME];

		if(!removableValue.IsBool()) {
			spdlog::error("DOSBox version with ID '{}' has an invalid '{}' property type: '{}', expected 'boolean'.", id, JSON_REMOVABLE_PROPERTY_NAME, Utilities::typeToString(removableValue.GetType()));
			return nullptr;
		}

		removable = removableValue.GetBool();
	}
	else {
		spdlog::warn("DOSBox version with ID '{}' is missing '{}' property.", id, JSON_REMOVABLE_PROPERTY_NAME);
	}

	// parse DOSBox version executable name
	if(!dosboxVersionValue.HasMember(JSON_EXECUTABLE_NAME_PROPERTY_NAME)) {
		spdlog::error("DOSBox version with ID '{}' is missing '{}' property.", id, JSON_EXECUTABLE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & executableNameValue = dosboxVersionValue[JSON_EXECUTABLE_NAME_PROPERTY_NAME];

	if(!executableNameValue.IsString()) {
		spdlog::error("DOSBox version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_EXECUTABLE_NAME_PROPERTY_NAME, Utilities::typeToString(executableNameValue.GetType()));
		return nullptr;
	}

	std::string executableName(Utilities::trimString(executableNameValue.GetString()));

	if(executableName.empty()) {
		spdlog::error("DOSBox version with ID '{}' '{}' property cannot be empty.", id, JSON_EXECUTABLE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse DOSBox version launch arguments
	std::string launchArguments;

	if(dosboxVersionValue.HasMember(JSON_LAUNCH_ARGUMENTS_PROPERTY_NAME)) {
		const rapidjson::Value & launchArgumentsValue = dosboxVersionValue[JSON_LAUNCH_ARGUMENTS_PROPERTY_NAME];

		if(!launchArgumentsValue.IsString()) {
			spdlog::error("DOSBox version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_LAUNCH_ARGUMENTS_PROPERTY_NAME, Utilities::typeToString(launchArgumentsValue.GetType()));
			return nullptr;
		}

		launchArguments = Utilities::trimString(launchArgumentsValue.GetString());

		if(launchArguments.empty()) {
			spdlog::error("DOSBox version with ID '{}' '{}' property cannot be empty.", id, JSON_LAUNCH_ARGUMENTS_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse DOSBox version directory path
	if(!dosboxVersionValue.HasMember(JSON_DIRECTORY_PATH_PROPERTY_NAME)) {
		spdlog::error("DOSBox version with ID '{}' is missing '{}' property.", id, JSON_DIRECTORY_PATH_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & directoryPathValue = dosboxVersionValue[JSON_DIRECTORY_PATH_PROPERTY_NAME];

	if(!directoryPathValue.IsString()) {
		spdlog::error("DOSBox version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_DIRECTORY_PATH_PROPERTY_NAME, Utilities::typeToString(directoryPathValue.GetType()));
		return nullptr;
	}

	std::string directoryPath(Utilities::trimString(directoryPathValue.GetString()));

	// parse the DOSBox version website property
	std::string website;

	if(dosboxVersionValue.HasMember(JSON_WEBSITE_PROPERTY_NAME)) {
		const rapidjson::Value & websiteValue = dosboxVersionValue[JSON_WEBSITE_PROPERTY_NAME];

		if(!websiteValue.IsString()) {
			spdlog::error("DOSBox version with ID '{}' '{}' property has invalid type: '{}', expected 'string'.", id, JSON_WEBSITE_PROPERTY_NAME, Utilities::typeToString(websiteValue.GetType()));
			return nullptr;
		}

		website = websiteValue.GetString();
	}

	// parse the DOSBox version source code url property
	std::string sourceCodeURL;

	if(dosboxVersionValue.HasMember(JSON_SOURCE_CODE_URL_PROPERTY_NAME)) {
		const rapidjson::Value & sourceCodeURLValue = dosboxVersionValue[JSON_SOURCE_CODE_URL_PROPERTY_NAME];

		if(!sourceCodeURLValue.IsString()) {
			spdlog::error("DOSBox version with ID '{}' '{}' property has invalid type: '{}', expected 'string'.", id, JSON_SOURCE_CODE_URL_PROPERTY_NAME, Utilities::typeToString(sourceCodeURLValue.GetType()));
			return nullptr;
		}

		sourceCodeURL = sourceCodeURLValue.GetString();
	}

	// initialize the DOSBox version
	std::unique_ptr<DOSBoxVersion> newDOSBoxVersion(std::make_unique<DOSBoxVersion>(id, longName, shortName, removable, executableName, directoryPath, website, sourceCodeURL));

	if(!launchArguments.empty()) {
		newDOSBoxVersion->setLaunchArguments(launchArguments);
	}

	// parse DOSBox version installed timestamp
	std::optional<std::chrono::time_point<std::chrono::system_clock>> optionalInstalledTimePoint;

	if(dosboxVersionValue.HasMember(JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME)) {
		const rapidjson::Value & installedTimestampValue = dosboxVersionValue[JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME];

		if(!installedTimestampValue.IsString()) {
			spdlog::error("DOSBox version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME, Utilities::typeToString(installedTimestampValue.GetType()));
			return nullptr;
		}

		optionalInstalledTimePoint = Utilities::parseTimePointFromString(installedTimestampValue.GetString());

		if(!optionalInstalledTimePoint.has_value()) {
			spdlog::error("DOSBox version with ID '{}' has an invalid '{}' timestamp value: '{}'.", id, JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME, installedTimestampValue.GetString());
			return nullptr;
		}

		newDOSBoxVersion->setInstalledTimePoint(optionalInstalledTimePoint.value());
	}

	// parse DOSBox version last launched timestamp
	std::optional<std::chrono::time_point<std::chrono::system_clock>> optionalLastLaunchedTimePoint;

	if(dosboxVersionValue.HasMember(JSON_LAST_LAUNCHED_TIMESTAMP_PROPERTY_NAME)) {
		const rapidjson::Value & lastLaunchedTimestampValue = dosboxVersionValue[JSON_LAST_LAUNCHED_TIMESTAMP_PROPERTY_NAME];

		if(!lastLaunchedTimestampValue.IsString()) {
			spdlog::error("DOSBox version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_LAST_LAUNCHED_TIMESTAMP_PROPERTY_NAME, Utilities::typeToString(lastLaunchedTimestampValue.GetType()));
			return nullptr;
		}

		optionalLastLaunchedTimePoint = Utilities::parseTimePointFromString(lastLaunchedTimestampValue.GetString());

		if(!optionalLastLaunchedTimePoint.has_value()) {
			spdlog::error("DOSBox version with ID '{}' has an invalid '{}' timestamp value: '{}'.", id, JSON_LAST_LAUNCHED_TIMESTAMP_PROPERTY_NAME, lastLaunchedTimestampValue.GetString());
			return nullptr;
		}

		newDOSBoxVersion->setLastLaunchedTimePoint(optionalLastLaunchedTimePoint.value());
	}

	// parse the supported operating systems property
	if(dosboxVersionValue.HasMember(JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME)) {
		const rapidjson::Value & supportedOperatingSystemsValue = dosboxVersionValue[JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME];

		if(!supportedOperatingSystemsValue.IsArray()) {
			spdlog::error("DOSBox version with ID '{}' '{}' property has invalid type: '{}', expected 'array'.", id, JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, Utilities::typeToString(supportedOperatingSystemsValue.GetType()));
			return nullptr;
		}

		for(rapidjson::Value::ConstValueIterator i = supportedOperatingSystemsValue.Begin(); i != supportedOperatingSystemsValue.End(); ++i) {
			if(!i->IsString()) {
				spdlog::error("DOSBox version with ID '{}' '{}' property contains invalid supported operating system type: '{}', expected 'string'.", id, JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, Utilities::typeToString(i->GetType()));
				return nullptr;
			}

			std::string supportedOperatingSystemName(Utilities::trimString(i->GetString()));

			std::optional<DeviceInformationBridge::OperatingSystemType> optionalSupportedOperatingSystem(magic_enum::enum_cast<DeviceInformationBridge::OperatingSystemType>(supportedOperatingSystemName));

			if(!optionalSupportedOperatingSystem.has_value()) {
				std::stringstream validOperatingSystems;

				for(const auto operatingSystem : magic_enum::enum_names<DeviceInformationBridge::OperatingSystemType>()) {
					if(validOperatingSystems.tellp() != 0) {
						validOperatingSystems << ", ";
					}

					validOperatingSystems << "'" << operatingSystem << "'";
				}

				spdlog::error("DOSBox version with ID '{}' '{}' property contains invalid supported operating system value: '{}', expected one of: {}.", id, JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, supportedOperatingSystemName, validOperatingSystems.str());
				return nullptr;
			}

			if(newDOSBoxVersion->hasSupportedOperatingSystem(optionalSupportedOperatingSystem.value())) {
				spdlog::warn("DOSBox version with ID '{}' '{}' property contains duplicate supported operating system: '{}'.", id, JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, supportedOperatingSystemName);
				continue;
			}

			newDOSBoxVersion->m_supportedOperatingSystems.push_back(optionalSupportedOperatingSystem.value());
		}
	}

	return newDOSBoxVersion;
}

std::string DOSBoxVersion::getDOSBoxConfigurationFileName() const {
	if(m_id.empty()) {
		return {};
	}

	return m_id + "." + DOSBoxConfiguration::FILE_EXTENSION;
}

bool DOSBoxVersion::loadDOSBoxConfigurationFrom(const std::string & directoryPath, bool * loaded) {
	std::string dosboxConfigurationFileName(getDOSBoxConfigurationFileName());

	if(dosboxConfigurationFileName.empty()) {
		return true;
	}

	std::string dosboxConfigurationFilePath(Utilities::joinPaths(directoryPath, dosboxConfigurationFileName));
	m_dosboxConfiguration->setFilePath(dosboxConfigurationFilePath);

	if(!std::filesystem::is_regular_file(std::filesystem::path(dosboxConfigurationFilePath))) {
		return true;
	}

	std::unique_ptr<DOSBoxConfiguration> dosboxConfiguration(DOSBoxConfiguration::loadFrom(dosboxConfigurationFilePath));

	if(dosboxConfiguration == nullptr) {
		spdlog::error("Failed to load DOSBox configuration from file: '{}'.", dosboxConfigurationFilePath);
		return false;
	}

	*m_dosboxConfiguration = std::move(*dosboxConfiguration);
	m_dosboxConfiguration->setModified(false);

	if(loaded != nullptr) {
		*loaded = true;
	}

	return true;
}

bool DOSBoxVersion::saveDOSBoxConfigurationTo(const std::string & directoryPath, bool * saved) const {
	if(m_dosboxConfiguration == nullptr || m_dosboxConfiguration->isEmpty()) {
		return true;
	}

	if(!m_dosboxConfiguration->isValid()) {
		spdlog::error("Failed to save invalid DOSBox configuration to file.");
		return false;
	}

	std::string dosboxConfigurationFileName(getDOSBoxConfigurationFileName());

	if(dosboxConfigurationFileName.empty()) {
		return true;
	}

	m_dosboxConfiguration->setFilePath(Utilities::joinPaths(directoryPath, dosboxConfigurationFileName));

	if(!m_dosboxConfiguration->save()) {
		spdlog::error("Failed to save DOSBox configuration to file: '{}'.", m_dosboxConfiguration->getFilePath());
		return false;
	}

	m_dosboxConfiguration->setModified(false);

	if(saved != nullptr) {
		*saved = true;
	}

	return true;
}

bool DOSBoxVersion::isConfigured() const {
	return isValid() &&
		   !m_directoryPath.empty();
}

bool DOSBoxVersion::isConfigured(const DOSBoxVersion * dosboxVersion) {
	return dosboxVersion != nullptr &&
		   dosboxVersion->isConfigured();
}

bool DOSBoxVersion::isValid() const {
	return !m_id.empty() &&
		   !m_longName.empty() &&
		   !m_shortName.empty() &&
		   !m_executableName.empty() &&
		   !m_supportedOperatingSystems.empty();
}

bool DOSBoxVersion::isValid(const DOSBoxVersion * dosboxVersion) {
	return dosboxVersion != nullptr &&
		   dosboxVersion->isValid();
}

bool DOSBoxVersion::operator == (const DOSBoxVersion & dosboxVersion) const {
	return m_removable == dosboxVersion.m_removable &&
		   Utilities::areStringsEqual(m_id, dosboxVersion.m_id) &&
		   Utilities::areStringsEqual(m_longName, dosboxVersion.m_longName) &&
		   Utilities::areStringsEqual(m_shortName, dosboxVersion.m_shortName) &&
		   m_installedTimePoint == dosboxVersion.m_installedTimePoint &&
		   m_lastLaunchedTimePoint == dosboxVersion.m_lastLaunchedTimePoint &&
		   Utilities::areStringsEqual(m_executableName, dosboxVersion.m_executableName) &&
		   Utilities::areStringsEqual(m_launchArguments, dosboxVersion.m_launchArguments) &&
		   Utilities::areStringsEqual(m_directoryPath, dosboxVersion.m_directoryPath) &&
		   Utilities::areStringsEqual(m_website, dosboxVersion.m_website) &&
		   Utilities::areStringsEqual(m_sourceCodeURL, dosboxVersion.m_sourceCodeURL) &&
		   m_supportedOperatingSystems != dosboxVersion.m_supportedOperatingSystems &&
		   (m_dosboxConfiguration == nullptr && dosboxVersion.m_dosboxConfiguration == nullptr || (m_dosboxConfiguration != nullptr && dosboxVersion.m_dosboxConfiguration != nullptr && *m_dosboxConfiguration != *dosboxVersion.m_dosboxConfiguration));
}

bool DOSBoxVersion::operator != (const DOSBoxVersion & dosboxVersion) const {
	return !operator == (dosboxVersion);
}
