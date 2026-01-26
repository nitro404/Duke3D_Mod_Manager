#include "DOSBoxDownloadCollection.h"

#include "DOSBoxDownload.h"

#include <Date.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/Utilities.h>

#include <fmt/core.h>
#include <magic_enum/magic_enum.hpp>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

static constexpr const char * JSON_FILE_TYPE_PROPERTY_NAME = "fileType";
static constexpr const char * JSON_FILE_FORMAT_VERSION_PROPERTY_NAME = "fileFormatVersion";
static constexpr const char * JSON_FILE_REVISION_PROPERTY_NAME = "fileRevision";
static constexpr const char * JSON_DOSBOX_DOWNLOADS_PROPERTY_NAME = "dosboxDownloads";

const std::string DOSBoxDownloadCollection::FILE_TYPE = "DOSBox Downloads";
const uint32_t DOSBoxDownloadCollection::FILE_FORMAT_VERSION = 1;

DOSBoxDownloadCollection::DOSBoxDownloadCollection(uint32_t fileRevision)
	: m_fileRevision(fileRevision) { }

DOSBoxDownloadCollection::DOSBoxDownloadCollection(DOSBoxDownloadCollection && c) noexcept
	: m_fileRevision(c.m_fileRevision)
	, m_downloads(std::move(c.m_downloads)) { }

DOSBoxDownloadCollection::DOSBoxDownloadCollection(const DOSBoxDownloadCollection & c)
	: m_fileRevision(c.m_fileRevision) {
	for(std::vector<std::shared_ptr<DOSBoxDownload>>::const_iterator i = c.m_downloads.begin(); i != c.m_downloads.end(); ++i) {
		m_downloads.push_back(std::make_shared<DOSBoxDownload>(**i));
	}
}

DOSBoxDownloadCollection & DOSBoxDownloadCollection::operator = (DOSBoxDownloadCollection && c) noexcept {
	if(this != &c) {
		m_fileRevision = c.m_fileRevision;
		m_downloads = std::move(c.m_downloads);
	}

	return *this;
}

DOSBoxDownloadCollection & DOSBoxDownloadCollection::operator = (const DOSBoxDownloadCollection & c) {
	m_downloads.clear();

	m_fileRevision = c.m_fileRevision;

	for(std::vector<std::shared_ptr<DOSBoxDownload>>::const_iterator i = c.m_downloads.begin(); i != c.m_downloads.end(); ++i) {
		m_downloads.push_back(std::make_shared<DOSBoxDownload>(**i));
	}

	return *this;
}

DOSBoxDownloadCollection::~DOSBoxDownloadCollection() { }

uint32_t DOSBoxDownloadCollection::getFileRevision() const {
	return m_fileRevision;
}

size_t DOSBoxDownloadCollection::numberOfDownloads() const {
	return m_downloads.size();
}

bool DOSBoxDownloadCollection::hasDownload(const DOSBoxDownload & download) const {
	for(std::vector<std::shared_ptr<DOSBoxDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), download.getID())) {
			return true;
		}
	}

	return false;
}

bool DOSBoxDownloadCollection::hasDownloadWithID(const std::string & dosboxVersionID) const {
	if(dosboxVersionID.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<DOSBoxDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), dosboxVersionID)) {
			return true;
		}
	}

	return false;
}

size_t DOSBoxDownloadCollection::indexOfDownload(const DOSBoxDownload & download) const {
	for(size_t i = 0; i < m_downloads.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_downloads[i]->getID(), download.getID())) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t DOSBoxDownloadCollection::indexOfDownloadWithID(const std::string & dosboxVersionID) const {
	if(dosboxVersionID.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_downloads.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_downloads[i]->getID(), dosboxVersionID)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<DOSBoxDownload> DOSBoxDownloadCollection::getDownload(size_t index) const {
	if(index >= m_downloads.size()) {
		return nullptr;
	}

	return m_downloads[index];
}

std::shared_ptr<DOSBoxDownload> DOSBoxDownloadCollection::getDownloadWithID(const std::string & dosboxVersionID) const {
	if(dosboxVersionID.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<DOSBoxDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), dosboxVersionID)) {
			return *i;
		}
	}

	return nullptr;
}

std::shared_ptr<DOSBoxDownloadFile> DOSBoxDownloadCollection::getLatestDOSBoxDownloadFile(const std::string & dosboxVersionID, DeviceInformationBridge::OperatingSystemType operatingSystem, std::optional<DeviceInformationBridge::ArchitectureType> optionalProcessorArchitecture) const {
	if(dosboxVersionID.empty()) {
		return nullptr;
	}

	std::shared_ptr<DOSBoxDownload> dosboxDownload(getDownloadWithID(dosboxVersionID));

	if(dosboxDownload == nullptr) {
		return nullptr;
	}

	return dosboxDownload->getLatestDOSBoxDownloadFile(operatingSystem, optionalProcessorArchitecture);
}

const std::vector<std::shared_ptr<DOSBoxDownload>> & DOSBoxDownloadCollection::getDownloads() const {
	return m_downloads;
}

bool DOSBoxDownloadCollection::addDownload(const DOSBoxDownload & download) {
	if(!download.isValid() || hasDownload(download)) {
		return false;
	}

	m_downloads.push_back(std::make_shared<DOSBoxDownload>(download));

	updated(*this);

	return true;
}

bool DOSBoxDownloadCollection::removeDownload(size_t index) {
	if(index >= m_downloads.size()) {
		return false;
	}

	m_downloads.erase(m_downloads.begin() + index);

	updated(*this);

	return true;
}

bool DOSBoxDownloadCollection::removeDownload(const DOSBoxDownload & download) {
	for(std::vector<std::shared_ptr<DOSBoxDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), download.getID())) {
			m_downloads.erase(i);

			updated(*this);

			return true;
		}
	}

	return false;
}

bool DOSBoxDownloadCollection::removeDownloadWithID(const std::string & dosboxVersionID) {
	if(dosboxVersionID.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<DOSBoxDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), dosboxVersionID)) {
			m_downloads.erase(i);

			updated(*this);

			return true;
		}
	}

	return false;
}

void DOSBoxDownloadCollection::clearDownloads() {
	m_downloads.clear();

	updated(*this);
}

rapidjson::Document DOSBoxDownloadCollection::toJSON() const {
	rapidjson::Document dosboxDownloadCollectionValue(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = dosboxDownloadCollectionValue.GetAllocator();

	rapidjson::Value fileTypeValue(FILE_TYPE.c_str(), allocator);
	dosboxDownloadCollectionValue.AddMember(rapidjson::StringRef(JSON_FILE_TYPE_PROPERTY_NAME), fileTypeValue, allocator);

	dosboxDownloadCollectionValue.AddMember(rapidjson::StringRef(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME), rapidjson::Value(FILE_FORMAT_VERSION), allocator);

	dosboxDownloadCollectionValue.AddMember(rapidjson::StringRef(JSON_FILE_REVISION_PROPERTY_NAME), rapidjson::Value(m_fileRevision), allocator);

	rapidjson::Value dosboxDownloadsValue(rapidjson::kArrayType);
	dosboxDownloadsValue.Reserve(m_downloads.size(), allocator);

	for(const std::shared_ptr<DOSBoxDownload> & download : m_downloads) {
		dosboxDownloadsValue.PushBack(download->toJSON(allocator), allocator);
	}

	dosboxDownloadCollectionValue.AddMember(rapidjson::StringRef(JSON_DOSBOX_DOWNLOADS_PROPERTY_NAME), dosboxDownloadsValue, allocator);

	return dosboxDownloadCollectionValue;
}

std::unique_ptr<DOSBoxDownloadCollection> DOSBoxDownloadCollection::parseFrom(const rapidjson::Value & dosboxDownloadCollectionValue) {
	if(!dosboxDownloadCollectionValue.IsObject()) {
		spdlog::error("Invalid DOSBox download collection type: '{}', expected 'object'.", Utilities::typeToString(dosboxDownloadCollectionValue.GetType()));
		return nullptr;
	}

	if(dosboxDownloadCollectionValue.HasMember(JSON_FILE_TYPE_PROPERTY_NAME)) {
		const rapidjson::Value & fileTypeValue = dosboxDownloadCollectionValue[JSON_FILE_TYPE_PROPERTY_NAME];

		if(!fileTypeValue.IsString()) {
			spdlog::error("Invalid DOSBox download collection file type type: '{}', expected: 'string'.", Utilities::typeToString(fileTypeValue.GetType()));
			return nullptr;
		}

		if(!Utilities::areStringsEqualIgnoreCase(fileTypeValue.GetString(), FILE_TYPE)) {
			spdlog::error("Incorrect DOSBox download collection file type: '{}', expected: '{}'.", fileTypeValue.GetString(), FILE_TYPE);
			return nullptr;
		}
	}
	else {
		spdlog::warn("DOSBox download collection JSON data is missing file type, and may fail to load correctly!");
	}

	if(dosboxDownloadCollectionValue.HasMember(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & fileFormatVersionValue = dosboxDownloadCollectionValue[JSON_FILE_FORMAT_VERSION_PROPERTY_NAME];

		if(!fileFormatVersionValue.IsUint()) {
			spdlog::error("Invalid DOSBox download collection file format version type: '{}', expected unsigned integer 'number'.", Utilities::typeToString(fileFormatVersionValue.GetType()));
			return nullptr;
		}

		if(fileFormatVersionValue.GetUint() != FILE_FORMAT_VERSION) {
			spdlog::error("Unsupported DOSBox download collection file format version: {}, only version {} is supported.", fileFormatVersionValue.GetUint(), FILE_FORMAT_VERSION);
			return nullptr;
		}
	}
	else {
		spdlog::warn("DOSBox download collection JSON data is missing file format version, and may fail to load correctly!");
	}

	uint32_t fileRevision = 1;

	if(dosboxDownloadCollectionValue.HasMember(JSON_FILE_REVISION_PROPERTY_NAME)) {
		const rapidjson::Value & fileRevisionValue = dosboxDownloadCollectionValue[JSON_FILE_REVISION_PROPERTY_NAME];

		if(!fileRevisionValue.IsUint()) {
			spdlog::error("Invalid DOSBox download collection file revision type: '{}', expected unsigned integer 'number'.", Utilities::typeToString(fileRevisionValue.GetType()));
			return nullptr;
		}

		fileRevision = fileRevisionValue.GetUint();
	}
	else {
		spdlog::warn("DOSBox download collection JSON data is missing file revision!");
	}

	if(!dosboxDownloadCollectionValue.HasMember(JSON_DOSBOX_DOWNLOADS_PROPERTY_NAME)) {
		spdlog::error("DOSBox download collection is missing '{}' property.", JSON_DOSBOX_DOWNLOADS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & dosboxDownloadsValue = dosboxDownloadCollectionValue[JSON_DOSBOX_DOWNLOADS_PROPERTY_NAME];

	if(!dosboxDownloadsValue.IsArray()) {
		spdlog::error("Invalid DOSBox download collection entry list type: '{}', expected 'array'.", Utilities::typeToString(dosboxDownloadsValue.GetType()));
		return nullptr;
	}

	std::unique_ptr<DOSBoxDownloadCollection> newDOSBoxDownloadCollection(std::make_unique<DOSBoxDownloadCollection>(fileRevision));

	if(dosboxDownloadsValue.Empty()) {
		return newDOSBoxDownloadCollection;
	}

	std::unique_ptr<DOSBoxDownload> newDownload;

	for(rapidjson::Value::ConstValueIterator i = dosboxDownloadsValue.Begin(); i != dosboxDownloadsValue.End(); ++i) {
		newDownload = DOSBoxDownload::parseFrom(*i);

		if(!DOSBoxDownload::isValid(newDownload.get())) {
			spdlog::error("Failed to parse dosbox download #{}{}!", newDOSBoxDownloadCollection->m_downloads.size() + 1, newDOSBoxDownloadCollection->numberOfDownloads() == 0 ? "" : fmt::format(" (after dosbox download with ID '{}')", newDOSBoxDownloadCollection->getDownload(newDOSBoxDownloadCollection->numberOfDownloads() - 1)->getID()));
			return nullptr;
		}

		if(newDOSBoxDownloadCollection->hasDownload(*newDownload)) {
			spdlog::warn("Encountered duplicate dosbox download #{}{}.", newDOSBoxDownloadCollection->m_downloads.size() + 1, newDOSBoxDownloadCollection->numberOfDownloads() == 0 ? "" : fmt::format(" (after dosbox download with ID '{}')", newDOSBoxDownloadCollection->getDownload(newDOSBoxDownloadCollection->numberOfDownloads() - 1)->getID()));
		}

		newDOSBoxDownloadCollection->m_downloads.emplace_back(std::move(newDownload));
	}

	return newDOSBoxDownloadCollection;
}

bool DOSBoxDownloadCollection::loadFrom(const std::string & filePath) {
	if(filePath.empty()) {
		return false;
	}

	std::string fileExtension(Utilities::getFileExtension(filePath));

	if(fileExtension.empty()) {
		return false;
	}
	else if(Utilities::areStringsEqualIgnoreCase(fileExtension, "json")) {
		return loadFromJSON(filePath);
	}

	return false;
}

bool DOSBoxDownloadCollection::loadFromJSON(const std::string & filePath) {
	if(filePath.empty() || !std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return false;
	}

	std::ifstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::Document modsValue;
	rapidjson::IStreamWrapper fileStreamWrapper(fileStream);
	if(modsValue.ParseStream(fileStreamWrapper).HasParseError()) {
		return false;
	}

	fileStream.close();

	std::unique_ptr<DOSBoxDownloadCollection> modCollection(parseFrom(modsValue));

	if(!DOSBoxDownloadCollection::isValid(modCollection.get())) {
		spdlog::error("Failed to parse mod collection from JSON file '{}'.", filePath);
		return false;
	}

	m_downloads = std::move(modCollection->m_downloads);

	updated(*this);

	return true;
}

bool DOSBoxDownloadCollection::saveTo(const std::string & filePath, bool overwrite) const {
	if(filePath.empty()) {
		return false;
	}

	std::string fileExtension(Utilities::getFileExtension(filePath));

	if(fileExtension.empty()) {
		return false;
	}
	else if(Utilities::areStringsEqualIgnoreCase(fileExtension, "json")) {
		return saveToJSON(filePath, overwrite);
	}

	return false;
}

bool DOSBoxDownloadCollection::saveToJSON(const std::string & filePath, bool overwrite) const {
	if(!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		spdlog::warn("File '{}' already exists, use overwrite to force write.", filePath);
		return false;
	}

	std::ofstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::Document mods(toJSON());

	rapidjson::OStreamWrapper fileStreamWrapper(fileStream);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> fileStreamWriter(fileStreamWrapper);
	fileStreamWriter.SetIndent('\t', 1);
	mods.Accept(fileStreamWriter);
	fileStream.close();

	return true;
}

bool DOSBoxDownloadCollection::isValid() const {
	for(std::vector<std::shared_ptr<DOSBoxDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(!(*i)->isValid()) {
			return false;
		}
	}

	return true;
}

bool DOSBoxDownloadCollection::isValid(const DOSBoxDownloadCollection * c) {
	return c != nullptr && c->isValid();
}

bool DOSBoxDownloadCollection::operator == (const DOSBoxDownloadCollection & c) const {
	if(m_fileRevision != c.m_fileRevision ||
	   m_downloads.size() != c.m_downloads.size()) {
		return false;
	}

	for(size_t i = 0; i < c.m_downloads.size(); i++) {
		if(*m_downloads[i] != *c.m_downloads[i]) {
			return false;
		}
	}

	return true;
}

bool DOSBoxDownloadCollection::operator != (const DOSBoxDownloadCollection & c) const {
	return !operator == (c);
}
