#include "DOSBoxDownload.h"

#include "DOSBoxDownloadVersion.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

#include <array>
#include <string_view>

static constexpr const char * JSON_DOSBOX_DOWNLOAD_ID_PROPERTY_NAME = "id";
static constexpr const char * JSON_DOSBOX_DOWNLOAD_NAME_PROPERTY_NAME = "name";
static constexpr const char * JSON_DOSBOX_DOWNLOAD_VERSIONS_PROPERTY_NAME = "versions";
static const std::array<std::string_view, 3> JSON_DOSBOX_DOWNLOAD_PROPERTY_NAMES = {
	JSON_DOSBOX_DOWNLOAD_ID_PROPERTY_NAME,
	JSON_DOSBOX_DOWNLOAD_NAME_PROPERTY_NAME,
	JSON_DOSBOX_DOWNLOAD_VERSIONS_PROPERTY_NAME
};

DOSBoxDownload::DOSBoxDownload(const std::string & id, const std::string & name)
	: m_id(Utilities::trimString(id))
	, m_name(Utilities::trimString(name)) { }

DOSBoxDownload::DOSBoxDownload(DOSBoxDownload && v) noexcept
	: m_id(std::move(v.m_id))
	, m_name(std::move(v.m_name))
	, m_versions(std::move(v.m_versions)) {
	updateParent();
}

DOSBoxDownload::DOSBoxDownload(const DOSBoxDownload & d)
	: m_id(d.m_id)
	, m_name(d.m_name) {
	for(std::vector<std::shared_ptr<DOSBoxDownloadVersion>>::const_iterator i = d.m_versions.begin(); i != d.m_versions.end(); ++i) {
		m_versions.push_back(std::make_shared<DOSBoxDownloadVersion>(**i));
	}

	updateParent();
}

DOSBoxDownload & DOSBoxDownload::operator = (DOSBoxDownload && d) noexcept {
	if(this != &d) {
		m_id = std::move(d.m_id);
		m_name = std::move(d.m_name);
		m_versions = std::move(d.m_versions);

		updateParent();
	}

	return *this;
}

DOSBoxDownload & DOSBoxDownload::operator = (const DOSBoxDownload & d) {
	m_versions.clear();

	m_id = d.m_id;
	m_name = d.m_name;

	for(std::vector<std::shared_ptr<DOSBoxDownloadVersion>>::const_iterator i = d.m_versions.begin(); i != d.m_versions.end(); ++i) {
		m_versions.push_back(std::make_shared<DOSBoxDownloadVersion>(**i));
	}

	updateParent();

	return *this;
}

DOSBoxDownload::~DOSBoxDownload() { }

const std::string & DOSBoxDownload::getID() const {
	return m_id;
}

const std::string & DOSBoxDownload::getName() const {
	return m_name;
}

size_t DOSBoxDownload::numberOfVersions() const {
	return m_versions.size();
}

bool DOSBoxDownload::hasVersion(const DOSBoxDownloadVersion & file) const {
	for(std::vector<std::shared_ptr<DOSBoxDownloadVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), file.getVersion())) {
			return true;
		}
	}

	return false;
}

bool DOSBoxDownload::hasVersion(const std::string & fileName) const {
	if(fileName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<DOSBoxDownloadVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), fileName)) {
			return true;
		}
	}

	return false;
}

size_t DOSBoxDownload::indexOfVersion(const DOSBoxDownloadVersion & file) const {
	for(size_t i = 0; i < m_versions.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_versions[i]->getVersion(), file.getVersion())) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t DOSBoxDownload::indexOfVersion(const std::string & fileName) const {
	if(fileName.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_versions.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_versions[i]->getVersion(), fileName)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<DOSBoxDownloadVersion> DOSBoxDownload::getVersion(size_t index) const {
	if(index >= m_versions.size()) {
		return nullptr;
	}

	return m_versions[index];
}

std::shared_ptr<DOSBoxDownloadVersion> DOSBoxDownload::getVersion(const std::string & fileName) const {
	if(fileName.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<DOSBoxDownloadVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), fileName)) {
			return *i;
		}
	}

	return nullptr;
}

std::shared_ptr<DOSBoxDownloadFile> DOSBoxDownload::getLatestDOSBoxDownloadFile(DeviceInformationBridge::OperatingSystemType operatingSystem, std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> optionalProcessorArchitecture) const {
	std::shared_ptr<DOSBoxDownloadFile> matchingDownloadFile;
	std::shared_ptr<DOSBoxDownloadFile> latestDOSBoxDownloadFile;
	std::optional<Date> latestReleaseDate;

	for(std::vector<std::shared_ptr<DOSBoxDownloadVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		matchingDownloadFile = (*i)->findFirstMatchingFile(operatingSystem, optionalProcessorArchitecture);

		if(matchingDownloadFile == nullptr || ((*i)->hasReleaseDate() && latestReleaseDate.has_value() && (*i)->getReleaseDate().value() < latestReleaseDate.value())) {
			continue;
		}

		latestDOSBoxDownloadFile = matchingDownloadFile;
		latestReleaseDate = (*i)->getReleaseDate();
	}

	return latestDOSBoxDownloadFile;
}

const std::vector<std::shared_ptr<DOSBoxDownloadVersion>> & DOSBoxDownload::getVersions() const {
	return m_versions;
}

bool DOSBoxDownload::addVersion(const DOSBoxDownloadVersion & file) {
	if(!file.isValid() || hasVersion(file)) {
		return false;
	}

	std::shared_ptr<DOSBoxDownloadVersion> newDOSBoxDownloadVersion(std::make_shared<DOSBoxDownloadVersion>(file));
	newDOSBoxDownloadVersion->setParentDOSBoxDownload(this);

	m_versions.push_back(newDOSBoxDownloadVersion);

	return true;
}

bool DOSBoxDownload::removeVersion(size_t index) {
	if(index >= m_versions.size()) {
		return false;
	}

	m_versions[index]->setParentDOSBoxDownload(nullptr);
	m_versions.erase(m_versions.begin() + index);

	return true;
}

bool DOSBoxDownload::removeVersion(const DOSBoxDownloadVersion & file) {
	for(std::vector<std::shared_ptr<DOSBoxDownloadVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), file.getVersion())) {
			(*i)->setParentDOSBoxDownload(nullptr);
			m_versions.erase(i);

			return true;
		}
	}

	return false;
}

bool DOSBoxDownload::removeVersion(const std::string & fileName) {
	if(fileName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<DOSBoxDownloadVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), fileName)) {
			(*i)->setParentDOSBoxDownload(nullptr);
			m_versions.erase(i);

			return true;
		}
	}

	return false;
}

void DOSBoxDownload::clearVersions() {
	m_versions.clear();
}

void DOSBoxDownload::updateParent() {
	for(std::vector<std::shared_ptr<DOSBoxDownloadVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		(*i)->setParentDOSBoxDownload(this);
	}
}

rapidjson::Value DOSBoxDownload::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value dosboxDownloadVersionValue(rapidjson::kObjectType);

	rapidjson::Value idValue(m_id.c_str(), allocator);
	dosboxDownloadVersionValue.AddMember(rapidjson::StringRef(JSON_DOSBOX_DOWNLOAD_ID_PROPERTY_NAME), idValue, allocator);

	rapidjson::Value nameValue(m_name.c_str(), allocator);
	dosboxDownloadVersionValue.AddMember(rapidjson::StringRef(JSON_DOSBOX_DOWNLOAD_NAME_PROPERTY_NAME), nameValue, allocator);

	rapidjson::Value versionsValue(rapidjson::kArrayType);
	versionsValue.Reserve(m_versions.size(), allocator);

	for(const std::shared_ptr<DOSBoxDownloadVersion> & version : m_versions) {
		versionsValue.PushBack(version->toJSON(allocator), allocator);
	}

	dosboxDownloadVersionValue.AddMember(rapidjson::StringRef(JSON_DOSBOX_DOWNLOAD_VERSIONS_PROPERTY_NAME), versionsValue, allocator);

	return dosboxDownloadVersionValue;
}

std::unique_ptr<DOSBoxDownload> DOSBoxDownload::parseFrom(const rapidjson::Value & dosboxDownloadVersionValue) {
	if(!dosboxDownloadVersionValue.IsObject()) {
		spdlog::error("Invalid dosbox download type: '{}', expected 'object'.", Utilities::typeToString(dosboxDownloadVersionValue.GetType()));
		return nullptr;
	}

	// check for unhandled dosbox download properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = dosboxDownloadVersionValue.MemberBegin(); i != dosboxDownloadVersionValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_DOSBOX_DOWNLOAD_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("DOSBox download has unexpected property '{}'.", i->name.GetString());
		}
	}

	// parse the id property
	if(!dosboxDownloadVersionValue.HasMember(JSON_DOSBOX_DOWNLOAD_ID_PROPERTY_NAME)) {
		spdlog::error("DOSBox download is missing '{}' property.", JSON_DOSBOX_DOWNLOAD_ID_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & idValue = dosboxDownloadVersionValue[JSON_DOSBOX_DOWNLOAD_ID_PROPERTY_NAME];

	if(!idValue.IsString()) {
		spdlog::error("DOSBox download '{}' property has invalid type: '{}', expected 'string'.", JSON_DOSBOX_DOWNLOAD_ID_PROPERTY_NAME, Utilities::typeToString(idValue.GetType()));
		return nullptr;
	}

	std::string id(idValue.GetString());

	if(id.empty()) {
		spdlog::error("DOSBox download '{}' property cannot be empty.", JSON_DOSBOX_DOWNLOAD_ID_PROPERTY_NAME);
		return nullptr;
	}

	// parse the name property
	if(!dosboxDownloadVersionValue.HasMember(JSON_DOSBOX_DOWNLOAD_NAME_PROPERTY_NAME)) {
		spdlog::error("DOSBox download is missing '{}' property.", JSON_DOSBOX_DOWNLOAD_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & nameValue = dosboxDownloadVersionValue[JSON_DOSBOX_DOWNLOAD_NAME_PROPERTY_NAME];

	if(!nameValue.IsString()) {
		spdlog::error("DOSBox download '{}' property has invalid type: '{}', expected 'string'.", JSON_DOSBOX_DOWNLOAD_NAME_PROPERTY_NAME, Utilities::typeToString(nameValue.GetType()));
		return nullptr;
	}

	std::string name(nameValue.GetString());

	if(name.empty()) {
		spdlog::error("DOSBox download '{}' property cannot be empty.", JSON_DOSBOX_DOWNLOAD_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// initialize the DOSBox download
	std::unique_ptr<DOSBoxDownload> newDOSBoxDownload(std::make_unique<DOSBoxDownload>(id, name));

	// parse the versions property
	if(!dosboxDownloadVersionValue.HasMember(JSON_DOSBOX_DOWNLOAD_VERSIONS_PROPERTY_NAME)) {
		spdlog::error("DOSBox download is missing '{}' property.", JSON_DOSBOX_DOWNLOAD_VERSIONS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & versionsValue = dosboxDownloadVersionValue[JSON_DOSBOX_DOWNLOAD_VERSIONS_PROPERTY_NAME];

	if(!versionsValue.IsArray()) {
		spdlog::error("DOSBox download '{}' property has invalid type: '{}', expected 'array'.", JSON_DOSBOX_DOWNLOAD_VERSIONS_PROPERTY_NAME, Utilities::typeToString(versionsValue.GetType()));
		return nullptr;
	}

	std::shared_ptr<DOSBoxDownloadVersion> newVersion;

	for(rapidjson::Value::ConstValueIterator i = versionsValue.Begin(); i != versionsValue.End(); ++i) {
		newVersion = std::shared_ptr<DOSBoxDownloadVersion>(DOSBoxDownloadVersion::parseFrom(*i).release());

		if(!DOSBoxDownloadVersion::isValid(newVersion.get())) {
			spdlog::error("Failed to parse mod file #{}.", newDOSBoxDownload->m_versions.size() + 1);
			return nullptr;
		}

		newVersion->setParentDOSBoxDownload(newDOSBoxDownload.get());

		if(newDOSBoxDownload->hasVersion(*newVersion)) {
			spdlog::error("Encountered duplicate mod file #{}.", newDOSBoxDownload->m_versions.size() + 1);
			return nullptr;
		}

		newDOSBoxDownload->m_versions.push_back(newVersion);
	}

	return newDOSBoxDownload;
}

bool DOSBoxDownload::isValid() const {
	if(m_id.empty() ||
	   m_name.empty() ||
	   m_versions.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<DOSBoxDownloadVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		if(!(*i)->isValid()) {
			return false;
		}

		if((*i)->getParentDOSBoxDownload() != this) {
			return false;
		}

		for(std::vector<std::shared_ptr<DOSBoxDownloadVersion>>::const_iterator j = i + 1; j != m_versions.end(); ++j) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), (*j)->getVersion())) {
				return false;
			}
		}
	}

	return true;
}

bool DOSBoxDownload::isValid(const DOSBoxDownload * d) {
	return d != nullptr && d->isValid();
}

bool DOSBoxDownload::operator == (const DOSBoxDownload & v) const {
	if(m_versions.size() != v.m_versions.size() ||
	   !Utilities::areStringsEqual(m_id, v.m_id) ||
	   !Utilities::areStringsEqual(m_name, v.m_name)) {
		return false;
	}

	for(size_t i = 0; i < m_versions.size(); i++) {
		if(m_versions[i] != v.m_versions[i]) {
			return false;
		}
	}

	return true;
}

bool DOSBoxDownload::operator != (const DOSBoxDownload & d) const {
	return !operator == (d);
}
