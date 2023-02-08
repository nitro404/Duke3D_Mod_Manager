#include "DOSBoxDownloadCollection.h"

#include "DOSBoxDownload.h"

#include <Date.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/Utilities.h>

#include <fmt/core.h>
#include <magic_enum.hpp>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

static constexpr const char * JSON_FILE_FORMAT_VERSION_PROPERTY_NAME = "fileFormatVersion";
static constexpr const char * JSON_DOSBOX_DOWNLOADS_PROPERTY_NAME = "dosboxDownloads";

const std::string DOSBoxDownloadCollection::FILE_FORMAT_VERSION = "1.0.0";

DOSBoxDownloadCollection::DOSBoxDownloadCollection() { }

DOSBoxDownloadCollection::DOSBoxDownloadCollection(DOSBoxDownloadCollection && c) noexcept
	: m_downloads(std::move(c.m_downloads)) { }

DOSBoxDownloadCollection::DOSBoxDownloadCollection(const DOSBoxDownloadCollection & c) {
	for(std::vector<std::shared_ptr<DOSBoxDownload>>::const_iterator i = c.m_downloads.begin(); i != c.m_downloads.end(); ++i) {
		m_downloads.push_back(std::make_shared<DOSBoxDownload>(**i));
	}
}

DOSBoxDownloadCollection & DOSBoxDownloadCollection::operator = (DOSBoxDownloadCollection && c) noexcept {
	if(this != &c) {
		m_downloads = std::move(c.m_downloads);
	}

	return *this;
}

DOSBoxDownloadCollection & DOSBoxDownloadCollection::operator = (const DOSBoxDownloadCollection & c) {
	m_downloads.clear();

	for(std::vector<std::shared_ptr<DOSBoxDownload>>::const_iterator i = c.m_downloads.begin(); i != c.m_downloads.end(); ++i) {
		m_downloads.push_back(std::make_shared<DOSBoxDownload>(**i));
	}

	return *this;
}

DOSBoxDownloadCollection::~DOSBoxDownloadCollection() { }

size_t DOSBoxDownloadCollection::numberOfDownloads() const {
	return m_downloads.size();
}

bool DOSBoxDownloadCollection::hasDownload(const DOSBoxDownload & mod) const {
	for(std::vector<std::shared_ptr<DOSBoxDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), mod.getName())) {
			return true;
		}
	}

	return false;
}

bool DOSBoxDownloadCollection::hasDownloadWithName(const std::string & name) const {
	if(name.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<DOSBoxDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			return true;
		}
	}

	return false;
}

size_t DOSBoxDownloadCollection::indexOfDownload(const DOSBoxDownload & download) const {
	for(size_t i = 0; i < m_downloads.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_downloads[i]->getName(), download.getName())) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t DOSBoxDownloadCollection::indexOfDownloadWithName(const std::string & name) const {
	if(name.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_downloads.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_downloads[i]->getName(), name)) {
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

std::shared_ptr<DOSBoxDownload> DOSBoxDownloadCollection::getDownloadWithName(const std::string & name) const {
	if(name.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<DOSBoxDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			return *i;
		}
	}

	return nullptr;
}

std::shared_ptr<DOSBoxDownloadFile> DOSBoxDownloadCollection::getLatestDOSBoxDownloadFile(const std::string & dosboxVersionName, DeviceInformationBridge::OperatingSystemType operatingSystem, std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> optionalProcessorArchitecture) const {
	if(dosboxVersionName.empty()) {
		return nullptr;
	}

	std::shared_ptr<DOSBoxDownload> dosboxDownload(getDownloadWithName(dosboxVersionName));

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

	notifyCollectionChanged();

	return true;
}

bool DOSBoxDownloadCollection::removeDownload(size_t index) {
	if(index >= m_downloads.size()) {
		return false;
	}

	m_downloads.erase(m_downloads.begin() + index);

	notifyCollectionChanged();

	return true;
}

bool DOSBoxDownloadCollection::removeDownload(const DOSBoxDownload & download) {
	for(std::vector<std::shared_ptr<DOSBoxDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), download.getName())) {
			m_downloads.erase(i);

			notifyCollectionChanged();

			return true;
		}
	}

	return false;
}

bool DOSBoxDownloadCollection::removeDownloadWithName(const std::string & name) {
	if(name.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<DOSBoxDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			m_downloads.erase(i);

			notifyCollectionChanged();

			return true;
		}
	}

	return false;
}

void DOSBoxDownloadCollection::clearDownloads() {
	m_downloads.clear();

	notifyCollectionChanged();
}

rapidjson::Document DOSBoxDownloadCollection::toJSON() const {
	rapidjson::Document dosboxDownloadCollectionValue(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = dosboxDownloadCollectionValue.GetAllocator();

	rapidjson::Value fileFormatVersionValue(FILE_FORMAT_VERSION.c_str(), allocator);
	dosboxDownloadCollectionValue.AddMember(rapidjson::StringRef(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME), fileFormatVersionValue, allocator);

	rapidjson::Value dosboxDownloadsValue(rapidjson::kArrayType);

	for(std::vector<std::shared_ptr<DOSBoxDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		dosboxDownloadsValue.PushBack((*i)->toJSON(allocator), allocator);
	}

	dosboxDownloadCollectionValue.AddMember(rapidjson::StringRef(JSON_DOSBOX_DOWNLOADS_PROPERTY_NAME), dosboxDownloadsValue, allocator);

	return dosboxDownloadCollectionValue;
}

std::unique_ptr<DOSBoxDownloadCollection> DOSBoxDownloadCollection::parseFrom(const rapidjson::Value & modCollectionValue) {
	if(!modCollectionValue.IsObject()) {
		spdlog::error("Invalid DOSBox download collection type: '{}', expected 'object'.", Utilities::typeToString(modCollectionValue.GetType()));
		return nullptr;
	}

	if(modCollectionValue.HasMember(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & fileFormatVersionValue = modCollectionValue[JSON_FILE_FORMAT_VERSION_PROPERTY_NAME];

		if(!fileFormatVersionValue.IsString()) {
			spdlog::error("Invalid DOSBox download collection file format version type: '{}', expected: 'string'.", Utilities::typeToString(fileFormatVersionValue.GetType()));
			return false;
		}

		std::optional<std::uint8_t> optionalVersionComparison(Utilities::compareVersions(fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION));

		if(!optionalVersionComparison.has_value()) {
			spdlog::error("Invalid DOSBox download collection file format version: '{}'.", fileFormatVersionValue.GetString());
			return false;
		}

		if(*optionalVersionComparison != 0) {
			spdlog::error("Unsupported DOSBox download collection file format version: '{}', only version '{}' is supported.", fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION);
			return false;
		}
	}
	else {
		spdlog::warn("DOSBox download collection JSON data is missing file format version, and may fail to load correctly!");
	}

	if(!modCollectionValue.HasMember(JSON_DOSBOX_DOWNLOADS_PROPERTY_NAME)) {
		spdlog::error("DOSBox download collection is missing '{}' property'.", JSON_DOSBOX_DOWNLOADS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & dosboxDownloadsValue = modCollectionValue[JSON_DOSBOX_DOWNLOADS_PROPERTY_NAME];

	if(!dosboxDownloadsValue.IsArray()) {
		spdlog::error("Invalid DOSBox download collection entry list type: '{}', expected 'array'.", Utilities::typeToString(dosboxDownloadsValue.GetType()));
		return nullptr;
	}

	std::unique_ptr<DOSBoxDownloadCollection> newDOSBoxDownloadCollection(std::make_unique<DOSBoxDownloadCollection>());

	if(dosboxDownloadsValue.Empty()) {
		return newDOSBoxDownloadCollection;
	}

	std::unique_ptr<DOSBoxDownload> newDownload;

	for(rapidjson::Value::ConstValueIterator i = dosboxDownloadsValue.Begin(); i != dosboxDownloadsValue.End(); ++i) {
		newDownload = DOSBoxDownload::parseFrom(*i);

		if(!DOSBoxDownload::isValid(newDownload.get())) {
			spdlog::error("Failed to parse dosbox download #{}{}!", newDOSBoxDownloadCollection->m_downloads.size() + 1, newDOSBoxDownloadCollection->numberOfDownloads() == 0 ? "" : fmt::format(" (after dosbox download with ID '{}')", newDOSBoxDownloadCollection->getDownload(newDOSBoxDownloadCollection->numberOfDownloads() - 1)->getName()));
			return nullptr;
		}

		if(newDOSBoxDownloadCollection->hasDownload(*newDownload.get())) {
			spdlog::warn("Encountered duplicate dosbox download #{}{}.", newDOSBoxDownloadCollection->m_downloads.size() + 1, newDOSBoxDownloadCollection->numberOfDownloads() == 0 ? "" : fmt::format(" (after dosbox download with ID '{}')", newDOSBoxDownloadCollection->getDownload(newDOSBoxDownloadCollection->numberOfDownloads() - 1)->getName()));
		}

		newDOSBoxDownloadCollection->m_downloads.push_back(std::shared_ptr<DOSBoxDownload>(newDownload.release()));
	}

	newDOSBoxDownloadCollection->notifyCollectionChanged();

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

	notifyCollectionChanged();

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

DOSBoxDownloadCollection::Listener::~Listener() { }

size_t DOSBoxDownloadCollection::numberOfListeners() const {
	return m_listeners.size();
}

bool DOSBoxDownloadCollection::hasListener(const Listener & listener) const {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			return true;
		}
	}

	return false;
}

size_t DOSBoxDownloadCollection::indexOfListener(const Listener & listener) const {
	for(size_t i = 0; i < m_listeners.size(); i++) {
		if(m_listeners[i] == &listener) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

DOSBoxDownloadCollection::Listener * DOSBoxDownloadCollection::getListener(size_t index) const {
	if(index >= m_listeners.size()) {
		return nullptr;
	}

	return m_listeners[index];
}

bool DOSBoxDownloadCollection::addListener(Listener & listener) {
	if(!hasListener(listener)) {
		m_listeners.push_back(&listener);

		return true;
	}

	return false;
}

bool DOSBoxDownloadCollection::removeListener(size_t index) {
	if(index >= m_listeners.size()) {
		return false;
	}

	m_listeners.erase(m_listeners.cbegin() + index);

	return true;
}

bool DOSBoxDownloadCollection::removeListener(const Listener & listener) {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			m_listeners.erase(i);

			return true;
		}
	}

	return false;
}

void DOSBoxDownloadCollection::clearListeners() {
	m_listeners.clear();
}

void DOSBoxDownloadCollection::notifyCollectionChanged() {
	for(Listener * listener : m_listeners) {
		listener->dosboxDownloadCollectionUpdated(*this);
	}
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
	if(m_downloads.size() != c.m_downloads.size()) {
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