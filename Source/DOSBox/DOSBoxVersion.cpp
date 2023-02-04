#include "DOSBoxVersion.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/RapidJSONUtilities.h>

#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <array>
#include <filesystem>
#include <sstream>

static constexpr const char * JSON_NAME_PROPERTY_NAME = "name";
static constexpr const char * JSON_REMOVABLE_PROPERTY_NAME = "removable";
static constexpr const char * JSON_RENAMABLE_PROPERTY_NAME = "renamable";
static constexpr const char * JSON_EXECUTABLE_NAME_PROPERTY_NAME = "executableName";
static constexpr const char * JSON_DIRECTORY_PATH_PROPERTY_NAME = "directoryPath";
static constexpr const char * JSON_WEBSITE_PROPERTY_NAME = "website";
static constexpr const char * JSON_SOURCE_CODE_URL_PROPERTY_NAME = "sourceCodeURL";
static constexpr const char * JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME = "supportedOperatingSystems";
static const std::array<std::string_view, 8> JSON_PROPERTY_NAMES = {
	JSON_NAME_PROPERTY_NAME,
	JSON_REMOVABLE_PROPERTY_NAME,
	JSON_RENAMABLE_PROPERTY_NAME,
	JSON_EXECUTABLE_NAME_PROPERTY_NAME,
	JSON_DIRECTORY_PATH_PROPERTY_NAME,
	JSON_WEBSITE_PROPERTY_NAME,
	JSON_SOURCE_CODE_URL_PROPERTY_NAME,
	JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME
};

static const std::string DEFAULT_EXECUTABLE_FILE_NAME("DOSBox.exe");

const DOSBoxVersion DOSBoxVersion::DOSBOX        ("DOSBox",         false, false, DEFAULT_EXECUTABLE_FILE_NAME, "", "https://www.dosbox.com",           "https://sourceforge.net/p/dosbox/code-0/HEAD/tree/dosbox/trunk", { DeviceInformationBridge::OperatingSystemType::Windows, DeviceInformationBridge::OperatingSystemType::Linux, DeviceInformationBridge::OperatingSystemType::MacOS });
const DOSBoxVersion DOSBoxVersion::DOSBOX_STAGING("DOSBox Staging", true,  false, "dosbox.exe",                 "", "https://dosbox-staging.github.io", "https://github.com/dosbox-staging/dosbox-staging",               { DeviceInformationBridge::OperatingSystemType::Windows, DeviceInformationBridge::OperatingSystemType::Linux, DeviceInformationBridge::OperatingSystemType::MacOS });
const DOSBoxVersion DOSBoxVersion::DOSBOX_X      ("DOSBox-X",       true,  false, "dosbox-x.exe",               "", "https://dosbox-x.com",             "https://github.com/joncampbell123/dosbox-x",                     { DeviceInformationBridge::OperatingSystemType::Windows, DeviceInformationBridge::OperatingSystemType::Linux, DeviceInformationBridge::OperatingSystemType::MacOS });

const std::vector<const DOSBoxVersion *> DOSBoxVersion::DEFAULT_DOSBOX_VERSIONS = {
	&DOSBOX,
	&DOSBOX_STAGING,
	&DOSBOX_X
};

DOSBoxVersion::DOSBoxVersion()
	: m_removable(true)
	, m_renamable(true)
	, m_executableName(DEFAULT_EXECUTABLE_FILE_NAME)
	, m_modified(false) { }

DOSBoxVersion::DOSBoxVersion(const std::string & name, bool removable, bool renamable, const std::string & executableName, const std::string & directoryPath, const std::string & website, const std::string & sourceCodeURL, const std::vector<DeviceInformationBridge::OperatingSystemType> & supportedOperatingSystems)
	: m_name(name)
	, m_removable(removable)
	, m_renamable(renamable)
	, m_executableName(executableName)
	, m_directoryPath(directoryPath)
	, m_website(website)
	, m_sourceCodeURL(sourceCodeURL)
	, m_modified(false) {
	addSupportedOperatingSystems(supportedOperatingSystems);

	setModified(false);
}

DOSBoxVersion::DOSBoxVersion(DOSBoxVersion && dosboxVersion) noexcept
	: m_name(std::move(dosboxVersion.m_name))
	, m_removable(dosboxVersion.m_removable)
	, m_renamable(dosboxVersion.m_renamable)
	, m_executableName(std::move(dosboxVersion.m_executableName))
	, m_website(std::move(dosboxVersion.m_website))
	, m_sourceCodeURL(std::move(dosboxVersion.m_sourceCodeURL))
	, m_supportedOperatingSystems(std::move(dosboxVersion.m_supportedOperatingSystems))
	, m_modified(false) { }

DOSBoxVersion::DOSBoxVersion(const DOSBoxVersion & dosboxVersion)
	: m_name(dosboxVersion.m_name)
	, m_removable(dosboxVersion.m_removable)
	, m_renamable(dosboxVersion.m_renamable)
	, m_executableName(dosboxVersion.m_executableName)
	, m_directoryPath(dosboxVersion.m_directoryPath)
	, m_website(dosboxVersion.m_website)
	, m_sourceCodeURL(dosboxVersion.m_sourceCodeURL)
	, m_supportedOperatingSystems(dosboxVersion.m_supportedOperatingSystems)
	, m_modified(false) { }

DOSBoxVersion & DOSBoxVersion::operator = (DOSBoxVersion && dosboxVersion) noexcept {
	if(this != &dosboxVersion) {
		m_name = std::move(dosboxVersion.m_name);
		m_removable = dosboxVersion.m_removable;
		m_renamable = dosboxVersion.m_renamable;
		m_executableName = std::move(dosboxVersion.m_executableName);
		m_directoryPath = std::move(dosboxVersion.m_directoryPath);
		m_website = std::move(dosboxVersion.m_website);
		m_sourceCodeURL = std::move(dosboxVersion.m_sourceCodeURL);
		m_supportedOperatingSystems = std::move(dosboxVersion.m_supportedOperatingSystems);

		setModified(true);
	}

	return *this;
}

DOSBoxVersion & DOSBoxVersion::operator = (const DOSBoxVersion & dosboxVersion) {
	m_name = dosboxVersion.m_name;
	m_removable = dosboxVersion.m_removable;
	m_renamable = dosboxVersion.m_renamable;
	m_executableName = dosboxVersion.m_executableName;
	m_directoryPath = dosboxVersion.m_directoryPath;
	m_website = dosboxVersion.m_website;
	m_sourceCodeURL = dosboxVersion.m_sourceCodeURL;
	m_supportedOperatingSystems = dosboxVersion.m_supportedOperatingSystems;

	setModified(true);

	return *this;
}

DOSBoxVersion::~DOSBoxVersion() = default;

bool DOSBoxVersion::isModified() const {
	return m_modified;
}

void DOSBoxVersion::setModified(bool modified) {
	m_modified = modified;

	notifyModified();
}

bool DOSBoxVersion::hasName() const {
	return !m_name.empty();
}

const std::string & DOSBoxVersion::getName() const {
	return m_name;
}

bool DOSBoxVersion::setName(const std::string & name) {
	if(!m_renamable) {
		return false;
	}

	if(Utilities::areStringsEqual(m_name, name)) {
		return true;
	}

	m_name = name;

	setModified(true);

	return true;
}

bool DOSBoxVersion::isRemovable() const {
	return m_removable;
}

bool DOSBoxVersion::isRenamable() const {
	return m_renamable;
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

bool DOSBoxVersion::hasDirectoryPath() const {
	return !m_directoryPath.empty();
}

const std::string & DOSBoxVersion::getDirectoryPath() const {
	return m_directoryPath;
}

void DOSBoxVersion::setDirectoryPath(const std::string & directoryPath) {
	m_directoryPath = directoryPath;

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

bool DOSBoxVersion::checkForMissingExecutable() const {
	if(!isConfigured()) {
		return false;
	}

	std::string fullExecutablePath(Utilities::joinPaths(m_directoryPath, m_executableName));

	if(!std::filesystem::is_regular_file(std::filesystem::path(fullExecutablePath))) {
		spdlog::error("Missing '{}' executable: '{}'.", m_name, fullExecutablePath);
		return true;
	}

	return false;
}

rapidjson::Value DOSBoxVersion::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value dosboxVersionValue(rapidjson::kObjectType);

	rapidjson::Value nameValue(m_name.c_str(), allocator);
	dosboxVersionValue.AddMember(rapidjson::StringRef(JSON_NAME_PROPERTY_NAME), nameValue, allocator);

	dosboxVersionValue.AddMember(rapidjson::StringRef(JSON_REMOVABLE_PROPERTY_NAME), rapidjson::Value(m_removable), allocator);

	dosboxVersionValue.AddMember(rapidjson::StringRef(JSON_RENAMABLE_PROPERTY_NAME), rapidjson::Value(m_renamable), allocator);

	rapidjson::Value executableNameValue(m_executableName.c_str(), allocator);
	dosboxVersionValue.AddMember(rapidjson::StringRef(JSON_EXECUTABLE_NAME_PROPERTY_NAME), executableNameValue, allocator);

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

	// parse DOSBox version name
	if(!dosboxVersionValue.HasMember(JSON_NAME_PROPERTY_NAME)) {
		spdlog::error("DOSBox version is missing '{}' property.", JSON_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & nameValue = dosboxVersionValue[JSON_NAME_PROPERTY_NAME];

	if(!nameValue.IsString()) {
		spdlog::error("DOSBox version has an invalid '{}' property type: '{}', expected 'string'.", JSON_NAME_PROPERTY_NAME, Utilities::typeToString(nameValue.GetType()));
		return nullptr;
	}

	std::string name(Utilities::trimString(nameValue.GetString()));

	if(name.empty()) {
		spdlog::error("DOSBox version '{}' property cannot be empty.", JSON_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse removable property
	bool removable = true;

	if(dosboxVersionValue.HasMember(JSON_REMOVABLE_PROPERTY_NAME)) {
		const rapidjson::Value & removableValue = dosboxVersionValue[JSON_REMOVABLE_PROPERTY_NAME];

		if(!removableValue.IsBool()) {
			spdlog::error("DOSBox version has an invalid '{}' property type: '{}', expected 'boolean'.", JSON_REMOVABLE_PROPERTY_NAME, Utilities::typeToString(removableValue.GetType()));
			return nullptr;
		}

		removable = removableValue.GetBool();
	}
	else {
		spdlog::warn("DOSBox version is missing '{}' property.", JSON_REMOVABLE_PROPERTY_NAME);
	}

	// parse renamable property
	bool renamable = true;

	if(dosboxVersionValue.HasMember(JSON_RENAMABLE_PROPERTY_NAME)) {
		const rapidjson::Value & renamableValue = dosboxVersionValue[JSON_RENAMABLE_PROPERTY_NAME];

		if(!renamableValue.IsBool()) {
			spdlog::error("DOSBox version has an invalid '{}' property type: '{}', expected 'boolean'.", JSON_RENAMABLE_PROPERTY_NAME, Utilities::typeToString(renamableValue.GetType()));
			return nullptr;
		}

		renamable = renamableValue.GetBool();
	}
	else {
		spdlog::warn("DOSBox version is missing '{}' property.", JSON_RENAMABLE_PROPERTY_NAME);
	}

	// parse DOSBox version executable name
	if(!dosboxVersionValue.HasMember(JSON_EXECUTABLE_NAME_PROPERTY_NAME)) {
		spdlog::error("DOSBox version is missing '{}' property.", JSON_EXECUTABLE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & executableNameValue = dosboxVersionValue[JSON_EXECUTABLE_NAME_PROPERTY_NAME];

	if(!executableNameValue.IsString()) {
		spdlog::error("DOSBox version has an invalid '{}' property type: '{}', expected 'string'.", JSON_EXECUTABLE_NAME_PROPERTY_NAME, Utilities::typeToString(executableNameValue.GetType()));
		return nullptr;
	}

	std::string executableName(Utilities::trimString(executableNameValue.GetString()));

	if(executableName.empty()) {
		spdlog::error("DOSBox version '{}' property cannot be empty.", JSON_EXECUTABLE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse DOSBox version directory path
	if(!dosboxVersionValue.HasMember(JSON_DIRECTORY_PATH_PROPERTY_NAME)) {
		spdlog::error("DOSBox version is missing '{}' property.", JSON_DIRECTORY_PATH_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & directoryPathValue = dosboxVersionValue[JSON_DIRECTORY_PATH_PROPERTY_NAME];

	if(!directoryPathValue.IsString()) {
		spdlog::error("DOSBox version has an invalid '{}' property type: '{}', expected 'string'.", JSON_DIRECTORY_PATH_PROPERTY_NAME, Utilities::typeToString(directoryPathValue.GetType()));
		return nullptr;
	}

	std::string directoryPath(Utilities::trimString(directoryPathValue.GetString()));

	// parse the DOSBox version website property
	std::string website;

	if(dosboxVersionValue.HasMember(JSON_WEBSITE_PROPERTY_NAME)) {
		const rapidjson::Value & websiteValue = dosboxVersionValue[JSON_WEBSITE_PROPERTY_NAME];

		if(!websiteValue.IsString()) {
			spdlog::error("DOSBox version '{}' property has invalid type: '{}', expected 'string'.", JSON_WEBSITE_PROPERTY_NAME, Utilities::typeToString(websiteValue.GetType()));
			return nullptr;
		}

		website = websiteValue.GetString();
	}

	// parse the DOSBox version source code url property
	std::string sourceCodeURL;

	if(dosboxVersionValue.HasMember(JSON_SOURCE_CODE_URL_PROPERTY_NAME)) {
		const rapidjson::Value & sourceCodeURLValue = dosboxVersionValue[JSON_SOURCE_CODE_URL_PROPERTY_NAME];

		if(!sourceCodeURLValue.IsString()) {
			spdlog::error("DOSBox version '{}' property has invalid type: '{}', expected 'string'.", JSON_SOURCE_CODE_URL_PROPERTY_NAME, Utilities::typeToString(sourceCodeURLValue.GetType()));
			return nullptr;
		}

		sourceCodeURL = sourceCodeURLValue.GetString();
	}

	// initialize the DOSBox version
	std::unique_ptr<DOSBoxVersion> newDOSBoxVersion(std::make_unique<DOSBoxVersion>(name, removable, renamable, executableName, directoryPath, website, sourceCodeURL));

	// parse the supported operating systems property
	if(dosboxVersionValue.HasMember(JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME)) {
		const rapidjson::Value & supportedOperatingSystemsValue = dosboxVersionValue[JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME];

		if(!supportedOperatingSystemsValue.IsArray()) {
			spdlog::error("DOSBox version '{}' property has invalid type: '{}', expected 'array'.", JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, Utilities::typeToString(supportedOperatingSystemsValue.GetType()));
			return nullptr;
		}

		for(rapidjson::Value::ConstValueIterator i = supportedOperatingSystemsValue.Begin(); i != supportedOperatingSystemsValue.End(); ++i) {
			if(!i->IsString()) {
				spdlog::error("DOSBox version '{}' property contains invalid supported operating system type: '{}', expected 'string'.", JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, Utilities::typeToString(i->GetType()));
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

				spdlog::error("DOSBox version '{}' property contains invalid supported operating system value: '{}', expected one of: {}.", JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, supportedOperatingSystemName, validOperatingSystems.str());
				return nullptr;
			}

			if(newDOSBoxVersion->hasSupportedOperatingSystem(optionalSupportedOperatingSystem.value())) {
				spdlog::warn("DOSBox version '{}' property contains duplicate supported operating system: '{}'.", JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, supportedOperatingSystemName);
				continue;
			}

			newDOSBoxVersion->m_supportedOperatingSystems.push_back(optionalSupportedOperatingSystem.value());
		}
	}

	return newDOSBoxVersion;
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
	return !m_name.empty() &&
		   !m_executableName.empty() &&
		   !m_supportedOperatingSystems.empty();
}

bool DOSBoxVersion::isValid(const DOSBoxVersion * dosboxVersion) {
	return dosboxVersion != nullptr &&
		   dosboxVersion->isValid();
}

DOSBoxVersion::Listener::~Listener() { }

size_t DOSBoxVersion::numberOfListeners() const {
	return m_listeners.size();
}

bool DOSBoxVersion::hasListener(const Listener & listener) const {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			return true;
		}
	}

	return false;
}

size_t DOSBoxVersion::indexOfListener(const Listener & listener) const {
	for(size_t i = 0; i < m_listeners.size(); i++) {
		if(m_listeners[i] == &listener) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

DOSBoxVersion::Listener * DOSBoxVersion::getListener(size_t index) const {
	if(index >= m_listeners.size()) {
		return nullptr;
	}

	return m_listeners[index];
}

bool DOSBoxVersion::addListener(Listener & listener) {
	if(!hasListener(listener)) {
		m_listeners.push_back(&listener);

		return true;
	}

	return false;
}

bool DOSBoxVersion::removeListener(size_t index) {
	if(index >= m_listeners.size()) {
		return false;
	}

	m_listeners.erase(m_listeners.cbegin() + index);

	return true;
}

bool DOSBoxVersion::removeListener(const Listener & listener) {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			m_listeners.erase(i);

			return true;
		}
	}

	return false;
}

void DOSBoxVersion::clearListeners() {
	m_listeners.clear();
}

void DOSBoxVersion::notifyModified() {
	for(Listener * listener : m_listeners) {
		listener->dosboxVersionModified(*this);
	}
}

bool DOSBoxVersion::operator == (const DOSBoxVersion & dosboxVersion) const {
	return m_removable == dosboxVersion.m_removable &&
		   m_renamable == dosboxVersion.m_renamable &&
		   Utilities::areStringsEqual(m_name, dosboxVersion.m_name) &&
		   Utilities::areStringsEqual(m_executableName, dosboxVersion.m_executableName) &&
		   Utilities::areStringsEqual(m_directoryPath, dosboxVersion.m_directoryPath) &&
		   Utilities::areStringsEqual(m_website, dosboxVersion.m_website) &&
		   Utilities::areStringsEqual(m_sourceCodeURL, dosboxVersion.m_sourceCodeURL) &&
		   m_supportedOperatingSystems != dosboxVersion.m_supportedOperatingSystems;
}

bool DOSBoxVersion::operator != (const DOSBoxVersion & dosboxVersion) const {
	return !operator == (dosboxVersion);
}
