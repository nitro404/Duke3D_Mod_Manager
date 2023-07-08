#include "DOSBoxDownloadVersion.h"

#include "DOSBoxDownload.h"
#include "DOSBoxDownloadFile.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

#include <array>
#include <string_view>

static constexpr const char * JSON_DOSBOX_DOWNLOAD_VERSION_VERSION_PROPERTY_NAME = "version";
static constexpr const char * JSON_DOSBOX_DOWNLOAD_VERSION_RELEASE_DATE_PROPERTY_NAME = "releaseDate";
static constexpr const char * JSON_DOSBOX_DOWNLOAD_VERSION_FILES_PROPERTY_NAME = "files";
static const std::array<std::string_view, 3> JSON_DOSBOX_DOWNLOAD_VERSION_PROPERTY_NAMES = {
	JSON_DOSBOX_DOWNLOAD_VERSION_VERSION_PROPERTY_NAME,
	JSON_DOSBOX_DOWNLOAD_VERSION_RELEASE_DATE_PROPERTY_NAME,
	JSON_DOSBOX_DOWNLOAD_VERSION_FILES_PROPERTY_NAME
};

DOSBoxDownloadVersion::DOSBoxDownloadVersion(const std::string & version, const std::optional<Date> & releaseDate)
	: m_version(Utilities::trimString(version))
	, m_releaseDate(releaseDate)
	, m_parentDOSBoxDownload(nullptr) { }

DOSBoxDownloadVersion::DOSBoxDownloadVersion(DOSBoxDownloadVersion && v) noexcept
	: m_version(std::move(v.m_version))
	, m_releaseDate(v.m_releaseDate)
	, m_files(std::move(v.m_files))
	, m_parentDOSBoxDownload(nullptr) {
	updateParent();
}

DOSBoxDownloadVersion::DOSBoxDownloadVersion(const DOSBoxDownloadVersion & v)
	: m_version(v.m_version)
	, m_releaseDate(v.m_releaseDate)
	, m_parentDOSBoxDownload(nullptr) {
	for(std::vector<std::shared_ptr<DOSBoxDownloadFile>>::const_iterator i = v.m_files.begin(); i != v.m_files.end(); ++i) {
		m_files.push_back(std::make_shared<DOSBoxDownloadFile>(**i));
	}

	updateParent();
}

DOSBoxDownloadVersion & DOSBoxDownloadVersion::operator = (DOSBoxDownloadVersion && v) noexcept {
	if(this != &v) {
		m_version = std::move(v.m_version);
		m_releaseDate = v.m_releaseDate;
		m_files = std::move(v.m_files);

		updateParent();
	}

	return *this;
}

DOSBoxDownloadVersion & DOSBoxDownloadVersion::operator = (const DOSBoxDownloadVersion & v) {
	m_files.clear();

	m_version = v.m_version;
	m_releaseDate = v.m_releaseDate;

	for(std::vector<std::shared_ptr<DOSBoxDownloadFile>>::const_iterator i = v.m_files.begin(); i != v.m_files.end(); ++i) {
		m_files.push_back(std::make_shared<DOSBoxDownloadFile>(**i));
	}

	updateParent();

	return *this;
}

DOSBoxDownloadVersion::~DOSBoxDownloadVersion() {
	m_parentDOSBoxDownload = nullptr;
}

const std::string & DOSBoxDownloadVersion::getVersion() const {
	return m_version;
}

std::string DOSBoxDownloadVersion::getFullName() const {
	if(!DOSBoxDownload::isValid(m_parentDOSBoxDownload)) {
		return "";
	}

	return m_parentDOSBoxDownload->getName() + " (" + m_version + ")";
}

bool DOSBoxDownloadVersion::hasReleaseDate() const {
	return m_releaseDate.has_value();
}

std::optional<Date> DOSBoxDownloadVersion::getReleaseDate() const {
	return m_releaseDate;
}

const DOSBoxDownload * DOSBoxDownloadVersion::getParentDOSBoxDownload() const {
	return m_parentDOSBoxDownload;
}

void DOSBoxDownloadVersion::setParentDOSBoxDownload(const DOSBoxDownload * dosboxDownload) {
	m_parentDOSBoxDownload = dosboxDownload;
}

size_t DOSBoxDownloadVersion::numberOfFiles() const {
	return m_files.size();
}

bool DOSBoxDownloadVersion::hasFile(const DOSBoxDownloadFile & file) const {
	for(std::vector<std::shared_ptr<DOSBoxDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), file.getFileName())) {
			return true;
		}
	}

	return false;
}

bool DOSBoxDownloadVersion::hasFileWithName(const std::string & fileName) const {
	if(fileName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<DOSBoxDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), fileName)) {
			return true;
		}
	}

	return false;
}

size_t DOSBoxDownloadVersion::indexOfFile(const DOSBoxDownloadFile & file) const {
	for(size_t i = 0; i < m_files.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_files[i]->getFileName(), file.getFileName())) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t DOSBoxDownloadVersion::indexOfFileWithName(const std::string & fileName) const {
	if(fileName.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_files.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_files[i]->getFileName(), fileName)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<DOSBoxDownloadFile> DOSBoxDownloadVersion::getFile(size_t index) const {
	if(index >= m_files.size()) {
		return nullptr;
	}

	return m_files[index];
}

std::shared_ptr<DOSBoxDownloadFile> DOSBoxDownloadVersion::getFileWithName(const std::string & fileName) const {
	if(fileName.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<DOSBoxDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), fileName)) {
			return *i;
		}
	}

	return nullptr;
}

std::shared_ptr<DOSBoxDownloadFile> DOSBoxDownloadVersion::findFirstMatchingFile(std::optional<DeviceInformationBridge::OperatingSystemType> operatingSystem, std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> processorArchitecture) const {
	if(!operatingSystem.has_value() && !processorArchitecture.has_value()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<DOSBoxDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(operatingSystem.has_value() && (*i)->getOperatingSystem() != operatingSystem.value()) {
			continue;
		}

		if(processorArchitecture.has_value() && (*i)->getProcessorArchitecture() != processorArchitecture.value()) {
			continue;
		}

		return *i;
	}

	return nullptr;
}

std::shared_ptr<DOSBoxDownloadFile> DOSBoxDownloadVersion::findLastMatchingFile(std::optional<DeviceInformationBridge::OperatingSystemType> operatingSystem, std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> processorArchitecture) const {
	if(!operatingSystem.has_value() && !processorArchitecture.has_value()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<DOSBoxDownloadFile>>::const_reverse_iterator i = m_files.rbegin(); i != m_files.rend(); ++i) {
		if(operatingSystem.has_value() && (*i)->getOperatingSystem() != operatingSystem.value()) {
			continue;
		}

		if(processorArchitecture.has_value() && (*i)->hasProcessorArchitecture() && (*i)->getProcessorArchitecture() != processorArchitecture.value()) {
			continue;
		}

		return *i;
	}

	return nullptr;
}

std::vector<std::shared_ptr<DOSBoxDownloadFile>> DOSBoxDownloadVersion::findAllMatchingFiles(std::optional<DeviceInformationBridge::OperatingSystemType> operatingSystem, std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> processorArchitecture) const {
	if(!operatingSystem.has_value() && !processorArchitecture.has_value()) {
		return {};
	}

	std::vector<std::shared_ptr<DOSBoxDownloadFile>> matchingFiles;

	for(std::vector<std::shared_ptr<DOSBoxDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(operatingSystem.has_value() && (*i)->getOperatingSystem() != operatingSystem.value()) {
			continue;
		}

		if(processorArchitecture.has_value() && (*i)->getProcessorArchitecture() != processorArchitecture.value()) {
			continue;
		}

		matchingFiles.push_back(*i);
	}

	return matchingFiles;
}

const std::vector<std::shared_ptr<DOSBoxDownloadFile>> & DOSBoxDownloadVersion::getFiles() const {
	return m_files;
}

bool DOSBoxDownloadVersion::addFile(const DOSBoxDownloadFile & file) {
	if(!file.isValid() || hasFile(file)) {
		return false;
	}

	std::shared_ptr<DOSBoxDownloadFile> newFile = std::make_shared<DOSBoxDownloadFile>(file);
	newFile->setParentDOSBoxDownloadVersion(this);

	m_files.push_back(newFile);

	return true;
}

bool DOSBoxDownloadVersion::removeFile(size_t index) {
	if(index >= m_files.size()) {
		return false;
	}

	m_files[index]->setParentDOSBoxDownloadVersion(nullptr);
	m_files.erase(m_files.begin() + index);

	return true;
}

bool DOSBoxDownloadVersion::removeFile(const DOSBoxDownloadFile & file) {
	for(std::vector<std::shared_ptr<DOSBoxDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), file.getFileName())) {
			(*i)->setParentDOSBoxDownloadVersion(nullptr);
			m_files.erase(i);

			return true;
		}
	}

	return false;
}

bool DOSBoxDownloadVersion::removeFileWithName(const std::string & fileName) {
	if(fileName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<DOSBoxDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), fileName)) {
			(*i)->setParentDOSBoxDownloadVersion(nullptr);
			m_files.erase(i);

			return true;
		}
	}

	return false;
}

void DOSBoxDownloadVersion::clearFiles() {
	m_files.clear();
}

void DOSBoxDownloadVersion::updateParent() {
	for(std::vector<std::shared_ptr<DOSBoxDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		(*i)->setParentDOSBoxDownloadVersion(this);
	}
}

rapidjson::Value DOSBoxDownloadVersion::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value dosboxDownloadVersionValue(rapidjson::kObjectType);

	rapidjson::Value versionValue(m_version.c_str(), allocator);
	dosboxDownloadVersionValue.AddMember(rapidjson::StringRef(JSON_DOSBOX_DOWNLOAD_VERSION_VERSION_PROPERTY_NAME), versionValue, allocator);

	if(m_releaseDate.has_value()) {
		rapidjson::Value releaseDateValue(m_releaseDate->toString().c_str(), allocator);
		dosboxDownloadVersionValue.AddMember(rapidjson::StringRef(JSON_DOSBOX_DOWNLOAD_VERSION_RELEASE_DATE_PROPERTY_NAME), releaseDateValue, allocator);
	}

	rapidjson::Value filesValue(rapidjson::kArrayType);
	filesValue.Reserve(m_files.size(), allocator);

	for(const std::shared_ptr<DOSBoxDownloadFile> & file : m_files) {
		filesValue.PushBack(file->toJSON(allocator), allocator);
	}

	dosboxDownloadVersionValue.AddMember(rapidjson::StringRef(JSON_DOSBOX_DOWNLOAD_VERSION_FILES_PROPERTY_NAME), filesValue, allocator);

	return dosboxDownloadVersionValue;
}

std::unique_ptr<DOSBoxDownloadVersion> DOSBoxDownloadVersion::parseFrom(const rapidjson::Value & dosboxDownloadVersionValue) {
	if(!dosboxDownloadVersionValue.IsObject()) {
		spdlog::error("Invalid dosbox download version type: '{}', expected 'object'.", Utilities::typeToString(dosboxDownloadVersionValue.GetType()));
		return nullptr;
	}

	// check for unhandled dosbox download version properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = dosboxDownloadVersionValue.MemberBegin(); i != dosboxDownloadVersionValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_DOSBOX_DOWNLOAD_VERSION_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("DOSBox download version has unexpected property '{}'.", i->name.GetString());
		}
	}

	// parse the version property
	if(!dosboxDownloadVersionValue.HasMember(JSON_DOSBOX_DOWNLOAD_VERSION_VERSION_PROPERTY_NAME)) {
		spdlog::error("DOSBox download version is missing '{}' property.", JSON_DOSBOX_DOWNLOAD_VERSION_VERSION_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & versionValue = dosboxDownloadVersionValue[JSON_DOSBOX_DOWNLOAD_VERSION_VERSION_PROPERTY_NAME];

	if(!versionValue.IsString()) {
		spdlog::error("DOSBox download version '{}' property has invalid type: '{}', expected 'string'.", JSON_DOSBOX_DOWNLOAD_VERSION_VERSION_PROPERTY_NAME, Utilities::typeToString(versionValue.GetType()));
		return nullptr;
	}

	std::string version(versionValue.GetString());

	// parse the release date property
	std::optional<Date> optionalReleaseDate;

	if(dosboxDownloadVersionValue.HasMember(JSON_DOSBOX_DOWNLOAD_VERSION_RELEASE_DATE_PROPERTY_NAME)) {
		const rapidjson::Value & releaseDateValue = dosboxDownloadVersionValue[JSON_DOSBOX_DOWNLOAD_VERSION_RELEASE_DATE_PROPERTY_NAME];

		if(!releaseDateValue.IsString()) {
			spdlog::error("DOSBox download version '{}' property has invalid type: '{}', expected 'string'.", JSON_DOSBOX_DOWNLOAD_VERSION_RELEASE_DATE_PROPERTY_NAME, Utilities::typeToString(releaseDateValue.GetType()));
			return nullptr;
		}

		optionalReleaseDate = Date::parseFrom(releaseDateValue.GetString());

		if(!optionalReleaseDate.has_value()) {
			spdlog::error("DOSBox download version '{}' property has invalid value: '{}'.", JSON_DOSBOX_DOWNLOAD_VERSION_RELEASE_DATE_PROPERTY_NAME, Utilities::valueToString(releaseDateValue));
			return nullptr;
		}
	}

	// initialize the dosbox download version
	std::unique_ptr<DOSBoxDownloadVersion> newDOSBoxDownloadVersion(std::make_unique<DOSBoxDownloadVersion>(version, optionalReleaseDate));

	// parse the files property
	if(!dosboxDownloadVersionValue.HasMember(JSON_DOSBOX_DOWNLOAD_VERSION_FILES_PROPERTY_NAME)) {
		spdlog::error("DOSBox download version is missing '{}' property.", JSON_DOSBOX_DOWNLOAD_VERSION_FILES_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & filesValue = dosboxDownloadVersionValue[JSON_DOSBOX_DOWNLOAD_VERSION_FILES_PROPERTY_NAME];

	if(!filesValue.IsArray()) {
		spdlog::error("DOSBox download version '{}' property has invalid type: '{}', expected 'array'.", JSON_DOSBOX_DOWNLOAD_VERSION_FILES_PROPERTY_NAME, Utilities::typeToString(filesValue.GetType()));
		return nullptr;
	}

	std::shared_ptr<DOSBoxDownloadFile> newFile;

	for(rapidjson::Value::ConstValueIterator i = filesValue.Begin(); i != filesValue.End(); ++i) {
		newFile = std::shared_ptr<DOSBoxDownloadFile>(DOSBoxDownloadFile::parseFrom(*i).release());

		if(!DOSBoxDownloadFile::isValid(newFile.get())) {
			spdlog::error("Failed to parse mod file #{}.", newDOSBoxDownloadVersion->m_files.size() + 1);
			return nullptr;
		}

		newFile->setParentDOSBoxDownloadVersion(newDOSBoxDownloadVersion.get());

		if(newDOSBoxDownloadVersion->hasFile(*newFile)) {
			spdlog::error("Encountered duplicate mod file #{}.", newDOSBoxDownloadVersion->m_files.size() + 1);
			return nullptr;
		}

		newDOSBoxDownloadVersion->m_files.push_back(newFile);
	}

	return newDOSBoxDownloadVersion;
}

bool DOSBoxDownloadVersion::isValid() const {
	if(m_version.empty() ||
	   m_files.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<DOSBoxDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(!(*i)->isValid()) {
			return false;
		}

		if((*i)->getParentDOSBoxDownloadVersion() != this) {
			return false;
		}

		for(std::vector<std::shared_ptr<DOSBoxDownloadFile>>::const_iterator j = i + 1; j != m_files.end(); ++j) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), (*j)->getFileName())) {
				return false;
			}
		}
	}

	return true;
}

bool DOSBoxDownloadVersion::isValid(const DOSBoxDownloadVersion * v) {
	return v != nullptr && v->isValid();
}

bool DOSBoxDownloadVersion::operator == (const DOSBoxDownloadVersion & v) const {
	if(m_files.size() != v.m_files.size() ||
	   !Utilities::areStringsEqualIgnoreCase(m_version, v.m_version)) {
		return false;
	}

	for(size_t i = 0; i < m_files.size(); i++) {
		if(m_files[i] != v.m_files[i]) {
			return false;
		}
	}

	return true;
}

bool DOSBoxDownloadVersion::operator != (const DOSBoxDownloadVersion & v) const {
	return !operator == (v);
}
