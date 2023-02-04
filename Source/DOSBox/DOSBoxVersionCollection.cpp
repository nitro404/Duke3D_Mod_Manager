#include "DOSBoxVersionCollection.h"

#include "DOSBoxVersion.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/Utilities.h>

#include <magic_enum.hpp>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

static constexpr const char * JSON_FILE_FORMAT_VERSION_PROPERTY_NAME = "fileFormatVersion";
static constexpr const char * JSON_DOSBOX_VERSIONS_PROPERTY_NAME = "dosboxVersions";

const std::string DOSBoxVersionCollection::FILE_FORMAT_VERSION = "1.0.0";

DOSBoxVersionCollection::DOSBoxVersionCollection() = default;

DOSBoxVersionCollection::DOSBoxVersionCollection(const std::vector<DOSBoxVersion> & dosboxVersions) {
	addDOSBoxVersions(dosboxVersions);
}

DOSBoxVersionCollection::DOSBoxVersionCollection(const std::vector<const DOSBoxVersion *> & dosboxVersions) {
	addDOSBoxVersions(dosboxVersions);
}

DOSBoxVersionCollection::DOSBoxVersionCollection(const std::vector<std::shared_ptr<DOSBoxVersion>> & dosboxVersions) {
	addDOSBoxVersions(dosboxVersions);
}

DOSBoxVersionCollection::DOSBoxVersionCollection(DOSBoxVersionCollection && g) noexcept
	: m_dosboxVersions(std::move(g.m_dosboxVersions)) {
	for(std::shared_ptr<DOSBoxVersion> & dosboxVersion : m_dosboxVersions) {
		dosboxVersion->clearListeners();
		dosboxVersion->addListener(*this);
	}
}

DOSBoxVersionCollection::DOSBoxVersionCollection(const DOSBoxVersionCollection & g) {
	for(std::vector<std::shared_ptr<DOSBoxVersion>>::const_iterator i = g.m_dosboxVersions.cbegin(); i != g.m_dosboxVersions.cend(); ++i) {
		m_dosboxVersions.push_back(std::make_shared<DOSBoxVersion>(**i));
		m_dosboxVersions.back()->addListener(*this);
	}
}

DOSBoxVersionCollection & DOSBoxVersionCollection::operator = (DOSBoxVersionCollection && g) noexcept {
	if(this != &g) {
		m_dosboxVersions = std::move(g.m_dosboxVersions);

		for(std::shared_ptr<DOSBoxVersion> & dosboxVersion : m_dosboxVersions) {
			dosboxVersion->clearListeners();
			dosboxVersion->addListener(*this);
		}
	}

	return *this;
}

DOSBoxVersionCollection & DOSBoxVersionCollection::operator = (const DOSBoxVersionCollection & g) {
	m_dosboxVersions.clear();

	for(std::vector<std::shared_ptr<DOSBoxVersion>>::const_iterator i = g.m_dosboxVersions.cbegin(); i != g.m_dosboxVersions.cend(); ++i) {
		m_dosboxVersions.push_back(std::make_shared<DOSBoxVersion>(**i));
		m_dosboxVersions.back()->addListener(*this);
	}

	return *this;
}

DOSBoxVersionCollection::~DOSBoxVersionCollection() {
	for(std::shared_ptr<DOSBoxVersion> & dosboxVersion : m_dosboxVersions) {
		dosboxVersion->removeListener(*this);
	}
}

size_t DOSBoxVersionCollection::numberOfDOSBoxVersions() const {
	return m_dosboxVersions.size();
}

bool DOSBoxVersionCollection::hasDOSBoxVersion(const DOSBoxVersion & dosboxVersion) const {
	return indexOfDOSBoxVersion(dosboxVersion) != std::numeric_limits<size_t>::max();
}

bool DOSBoxVersionCollection::hasDOSBoxVersionWithName(const std::string & name) const {
	return indexOfDOSBoxVersionWithName(name) != std::numeric_limits<size_t>::max();
}

size_t DOSBoxVersionCollection::indexOfDOSBoxVersion(const DOSBoxVersion & dosboxVersion) const {
	auto dosboxVersionIterator = std::find_if(m_dosboxVersions.cbegin(), m_dosboxVersions.cend(), [&dosboxVersion](const std::shared_ptr<DOSBoxVersion> & currentDOSBoxVersion) {
		return &dosboxVersion == currentDOSBoxVersion.get();
	});

	if(dosboxVersionIterator == m_dosboxVersions.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return dosboxVersionIterator - m_dosboxVersions.cbegin();
}

size_t DOSBoxVersionCollection::indexOfDOSBoxVersionWithName(const std::string & name) const {
	if(name.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	auto dosboxVersionIterator = std::find_if(m_dosboxVersions.cbegin(), m_dosboxVersions.cend(), [&name](const std::shared_ptr<DOSBoxVersion> & currentDOSBoxVersion) {
		return Utilities::areStringsEqualIgnoreCase(name, currentDOSBoxVersion->getName());
	});

	if(dosboxVersionIterator == m_dosboxVersions.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return dosboxVersionIterator - m_dosboxVersions.cbegin();
}

std::shared_ptr<DOSBoxVersion> DOSBoxVersionCollection::getDOSBoxVersion(size_t index) const {
	if(index >= m_dosboxVersions.size()) {
		return nullptr;
	}

	return m_dosboxVersions[index];
}

std::shared_ptr<DOSBoxVersion> DOSBoxVersionCollection::getDOSBoxVersionWithName(const std::string & name) const {
	return getDOSBoxVersion(indexOfDOSBoxVersionWithName(name));
}

const std::vector<std::shared_ptr<DOSBoxVersion>> & DOSBoxVersionCollection::getDOSBoxVersions() const {
	return m_dosboxVersions;
}

std::vector<std::shared_ptr<DOSBoxVersion>> DOSBoxVersionCollection::getConfiguredDOSBoxVersions() const {
	std::vector<std::shared_ptr<DOSBoxVersion>> configuredDOSBoxVersions;

	for(std::vector<std::shared_ptr<DOSBoxVersion>>::const_iterator i = m_dosboxVersions.begin(); i != m_dosboxVersions.end(); ++i) {
		if(!(*i)->isConfigured()) {
			continue;
		}

		configuredDOSBoxVersions.push_back(*i);
	}

	return configuredDOSBoxVersions;
}

std::vector<std::shared_ptr<DOSBoxVersion>> DOSBoxVersionCollection::getUnconfiguredDOSBoxVersions() const {
	std::vector<std::shared_ptr<DOSBoxVersion>> unconfiguredDOSBoxVersions;

	for(std::vector<std::shared_ptr<DOSBoxVersion>>::const_iterator i = m_dosboxVersions.begin(); i != m_dosboxVersions.end(); ++i) {
		if((*i)->isConfigured()) {
			continue;
		}

		unconfiguredDOSBoxVersions.push_back(*i);
	}

	return unconfiguredDOSBoxVersions;
}

std::vector<std::string> DOSBoxVersionCollection::getDOSBoxVersionDisplayNames(bool prependItemNumber) const {
	return getDOSBoxVersionDisplayNamesFrom(m_dosboxVersions, prependItemNumber);
}

std::vector<std::string> DOSBoxVersionCollection::getDOSBoxVersionDisplayNamesFrom(const std::vector<std::shared_ptr<DOSBoxVersion>> & dosboxVersions, bool prependItemNumber) {
	std::vector<std::string> dosboxVersionDisplayNames;

	for(size_t i = 0; i < dosboxVersions.size(); i++) {
		std::stringstream dosboxVersionStream;

		if(prependItemNumber) {
			dosboxVersionStream << i + 1 << ": ";
		}

		dosboxVersionStream << dosboxVersions[i]->getName();

		dosboxVersionDisplayNames.push_back(dosboxVersionStream.str());
	}

	return dosboxVersionDisplayNames;
}

std::vector<std::string> DOSBoxVersionCollection::getDOSBoxVersionDisplayNamesFrom(const std::vector<const DOSBoxVersion *> & dosboxVersions, bool prependItemNumber) {
	std::vector<std::string> dosboxVersionDisplayNames;

	for(size_t i = 0; i < dosboxVersions.size(); i++) {
		std::stringstream dosboxVersionStream;

		if(prependItemNumber) {
			dosboxVersionStream << i + 1 << ": ";
		}

		dosboxVersionStream << dosboxVersions[i]->getName();

		dosboxVersionDisplayNames.push_back(dosboxVersionStream.str());
	}

	return dosboxVersionDisplayNames;
}

bool DOSBoxVersionCollection::addDOSBoxVersion(const DOSBoxVersion & dosboxVersion) {
	if(!dosboxVersion.isValid() || hasDOSBoxVersionWithName(dosboxVersion.getName())) {
		return false;
	}

	m_dosboxVersions.push_back(std::make_shared<DOSBoxVersion>(dosboxVersion));
	m_dosboxVersions.back()->addListener(*this);

	notifyCollectionSizeChanged();

	return true;
}

bool DOSBoxVersionCollection::addDOSBoxVersion(std::shared_ptr<DOSBoxVersion> dosboxVersion) {
	if(!DOSBoxVersion::isValid(dosboxVersion.get()) || hasDOSBoxVersionWithName(dosboxVersion->getName())) {
		return false;
	}

	m_dosboxVersions.push_back(dosboxVersion);
	m_dosboxVersions.back()->addListener(*this);

	notifyCollectionSizeChanged();

	return true;
}

size_t DOSBoxVersionCollection::addDOSBoxVersions(const std::vector<DOSBoxVersion> & dosboxVersions) {
	size_t numberOfDOSBoxVersionsAdded = 0;

	for(std::vector<DOSBoxVersion>::const_iterator i = dosboxVersions.begin(); i != dosboxVersions.end(); ++i) {
		if(addDOSBoxVersion(*i)) {
			numberOfDOSBoxVersionsAdded++;
		}
	}

	return numberOfDOSBoxVersionsAdded;
}

size_t DOSBoxVersionCollection::addDOSBoxVersions(const std::vector<const DOSBoxVersion *> & dosboxVersions) {
	size_t numberOfDOSBoxVersionsAdded = 0;

	for(std::vector<const DOSBoxVersion *>::const_iterator i = dosboxVersions.begin(); i != dosboxVersions.end(); ++i) {
		if(addDOSBoxVersion(**i)) {
			numberOfDOSBoxVersionsAdded++;
		}
	}

	return numberOfDOSBoxVersionsAdded;
}

size_t DOSBoxVersionCollection::addDOSBoxVersions(const std::vector<std::shared_ptr<DOSBoxVersion>> & dosboxVersions) {
	size_t numberOfDOSBoxVersionsAdded = 0;

	for(std::vector<std::shared_ptr<DOSBoxVersion>>::const_iterator i = dosboxVersions.begin(); i != dosboxVersions.end(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if(addDOSBoxVersion(*i)) {
			numberOfDOSBoxVersionsAdded++;
		}
	}

	return numberOfDOSBoxVersionsAdded;
}

bool DOSBoxVersionCollection::removeDOSBoxVersion(size_t index) {
	if(index >= m_dosboxVersions.size()) {
		return false;
	}

	m_dosboxVersions[index]->removeListener(*this);
	m_dosboxVersions.erase(m_dosboxVersions.begin() + index);

	notifyCollectionSizeChanged();

	return true;
}

bool DOSBoxVersionCollection::removeDOSBoxVersion(const DOSBoxVersion & dosboxVersion) {
	return removeDOSBoxVersion(indexOfDOSBoxVersion(dosboxVersion));
}

bool DOSBoxVersionCollection::removeDOSBoxVersionWithName(const std::string & name) {
	return removeDOSBoxVersion(indexOfDOSBoxVersionWithName(name));
}

size_t DOSBoxVersionCollection::addMissingDefaultDOSBoxVersions() {
	size_t numberOfDOSBoxVersionsAdded = 0;

	for(std::vector<const DOSBoxVersion *>::const_iterator i = DOSBoxVersion::DEFAULT_DOSBOX_VERSIONS.cbegin(); i != DOSBoxVersion::DEFAULT_DOSBOX_VERSIONS.cend(); ++i) {
		if(hasDOSBoxVersionWithName((*i)->getName())) {
			continue;
		}

		spdlog::info("Adding missing default DOSBox version '{}'.", (*i)->getName());

		addDOSBoxVersion(**i);

		numberOfDOSBoxVersionsAdded++;
	}

	return numberOfDOSBoxVersionsAdded;
}

void DOSBoxVersionCollection::setDefaultDOSBoxVersions() {
	clearDOSBoxVersions();

	addDOSBoxVersions(DOSBoxVersion::DEFAULT_DOSBOX_VERSIONS);
}

void DOSBoxVersionCollection::clearDOSBoxVersions() {
	for(std::shared_ptr<DOSBoxVersion> & dosboxVersion : m_dosboxVersions) {
		dosboxVersion->removeListener(*this);
	}

	m_dosboxVersions.clear();

	notifyCollectionSizeChanged();
}

size_t DOSBoxVersionCollection::checkForMissingExecutables() const {
	size_t numberOfMissingExecutables = 0;

	for(std::vector<std::shared_ptr<DOSBoxVersion>>::const_iterator i = m_dosboxVersions.begin(); i != m_dosboxVersions.end(); ++i) {
		if((*i)->checkForMissingExecutable()) {
			numberOfMissingExecutables++;
		}
	}

	return numberOfMissingExecutables++;
}

size_t DOSBoxVersionCollection::checkForMissingExecutables(const std::string & name) const {
	std::shared_ptr<DOSBoxVersion> dosboxVersion = getDOSBoxVersionWithName(name);

	if(dosboxVersion == nullptr) {
		return 0;
	}

	return dosboxVersion->checkForMissingExecutable();
}

rapidjson::Document DOSBoxVersionCollection::toJSON() const {
	rapidjson::Document dosboxVersionsDocument(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = dosboxVersionsDocument.GetAllocator();

	rapidjson::Value fileFormatVersionValue(FILE_FORMAT_VERSION.c_str(), allocator);
	dosboxVersionsDocument.AddMember(rapidjson::StringRef(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME), fileFormatVersionValue, allocator);

	rapidjson::Value dosboxVersionsValue(rapidjson::kArrayType);

	for(std::vector<std::shared_ptr<DOSBoxVersion>>::const_iterator i = m_dosboxVersions.begin(); i != m_dosboxVersions.end(); ++i) {
		dosboxVersionsValue.PushBack((*i)->toJSON(allocator), allocator);
	}

	dosboxVersionsDocument.AddMember(rapidjson::StringRef(JSON_DOSBOX_VERSIONS_PROPERTY_NAME), dosboxVersionsValue, allocator);

	return dosboxVersionsDocument;
}

std::unique_ptr<DOSBoxVersionCollection> DOSBoxVersionCollection::parseFrom(const rapidjson::Value & dosboxVersionCollectionValue) {
	if(!dosboxVersionCollectionValue.IsObject()) {
		spdlog::error("Invalid DOSBox version collection type: '{}', expected 'object'.", Utilities::typeToString(dosboxVersionCollectionValue.GetType()));
		return nullptr;
	}

	if(dosboxVersionCollectionValue.HasMember(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & fileFormatVersionValue = dosboxVersionCollectionValue[JSON_FILE_FORMAT_VERSION_PROPERTY_NAME];

		if(!fileFormatVersionValue.IsString()) {
			spdlog::error("Invalid DOSBox version collection file format version type: '{}', expected: 'string'.", Utilities::typeToString(fileFormatVersionValue.GetType()));
			return false;
		}

		std::optional<std::uint8_t> optionalVersionComparison(Utilities::compareVersions(fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION));

		if(!optionalVersionComparison.has_value()) {
			spdlog::error("Invalid DOSBox version collection file format version: '{}'.", fileFormatVersionValue.GetString());
			return false;
		}

		if(*optionalVersionComparison != 0) {
			spdlog::error("Unsupported DOSBox version collection file format version: '{}', only version '{}' is supported.", fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION);
			return false;
		}
	}
	else {
		spdlog::warn("DOSBox version collection JSON data is missing file format version, and may fail to load correctly!");
	}

	if(!dosboxVersionCollectionValue.HasMember(JSON_DOSBOX_VERSIONS_PROPERTY_NAME)) {
		spdlog::error("DOSBox version collection is missing '{}' property'.", JSON_DOSBOX_VERSIONS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & dosboxVersionsValue = dosboxVersionCollectionValue[JSON_DOSBOX_VERSIONS_PROPERTY_NAME];

	if(!dosboxVersionsValue.IsArray()) {
		spdlog::error("Invalid DOSBox version collection '{}' type: '{}', expected 'array'.", Utilities::typeToString(dosboxVersionsValue.GetType()));
		return nullptr;
	}

	std::unique_ptr<DOSBoxVersionCollection> newDOSBoxVersionCollection = std::make_unique<DOSBoxVersionCollection>();

	if(dosboxVersionsValue.Empty()) {
		return newDOSBoxVersionCollection;
	}

	std::unique_ptr<DOSBoxVersion> newDOSBoxVersion;

	for(rapidjson::Value::ConstValueIterator i = dosboxVersionsValue.Begin(); i != dosboxVersionsValue.End(); ++i) {
		newDOSBoxVersion = DOSBoxVersion::parseFrom(*i);

		if(!DOSBoxVersion::isValid(newDOSBoxVersion.get())) {
			spdlog::error("Failed to parse DOSBox version #{}{}!", newDOSBoxVersionCollection->m_dosboxVersions.size() + 1, newDOSBoxVersionCollection->numberOfDOSBoxVersions() == 0 ? "" : fmt::format(" (after DOSBox version '{}')", newDOSBoxVersionCollection->getDOSBoxVersion(newDOSBoxVersionCollection->numberOfDOSBoxVersions() - 1)->getName()));
			return nullptr;
		}

		if(newDOSBoxVersionCollection->hasDOSBoxVersion(*newDOSBoxVersion.get())) {
			spdlog::error("Encountered duplicate DOSBox version #{}{}.", newDOSBoxVersionCollection->m_dosboxVersions.size() + 1, newDOSBoxVersionCollection->numberOfDOSBoxVersions() == 0 ? "" : fmt::format(" (after DOSBox version '{}')", newDOSBoxVersionCollection->getDOSBoxVersion(newDOSBoxVersionCollection->numberOfDOSBoxVersions() - 1)->getName()));
			return nullptr;
		}

		newDOSBoxVersionCollection->m_dosboxVersions.push_back(std::shared_ptr<DOSBoxVersion>(newDOSBoxVersion.release()));
	}

	return newDOSBoxVersionCollection;
}

bool DOSBoxVersionCollection::loadFrom(const std::string & filePath, bool autoCreate) {
	if(filePath.empty()) {
		return false;
	}

	std::string fileExtension(Utilities::getFileExtension(filePath));

	if(fileExtension.empty()) {
		return false;
	}
	else if(Utilities::areStringsEqualIgnoreCase(fileExtension, "json")) {
		return loadFromJSON(filePath, autoCreate);
	}

	return false;
}

bool DOSBoxVersionCollection::loadFromJSON(const std::string & filePath, bool autoCreate) {
	if(filePath.empty()) {
		return false;
	}

	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		if(autoCreate) {
			saveToJSON(filePath);
		}

		return false;
	}

	std::ifstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::Document dosboxVersionsValue;
	rapidjson::IStreamWrapper fileStreamWrapper(fileStream);
	dosboxVersionsValue.ParseStream(fileStreamWrapper);

	fileStream.close();

	std::unique_ptr<DOSBoxVersionCollection> dosboxVersionCollection = parseFrom(dosboxVersionsValue);

	if(!DOSBoxVersionCollection::isValid(dosboxVersionCollection.get())) {
		spdlog::error("Failed to parse dosboxVersion collection from JSON file '{}'.", filePath);
		return false;
	}

	m_dosboxVersions = dosboxVersionCollection->m_dosboxVersions;

	notifyCollectionSizeChanged();

	return true;
}

bool DOSBoxVersionCollection::saveTo(const std::string & filePath, bool overwrite) const {
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

bool DOSBoxVersionCollection::saveToJSON(const std::string & filePath, bool overwrite) const {
	if (!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		spdlog::warn("File '{}' already exists, use overwrite to force write.", filePath);
		return false;
	}

	std::ofstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::Document dosboxVersions(toJSON());

	rapidjson::OStreamWrapper fileStreamWrapper(fileStream);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> fileStreamWriter(fileStreamWrapper);
	fileStreamWriter.SetIndent('\t', 1);
	dosboxVersions.Accept(fileStreamWriter);
	fileStream.close();

	return true;
}

size_t DOSBoxVersionCollection::numberOfListeners() const {
	return m_listeners.size();
}

bool DOSBoxVersionCollection::hasListener(const Listener & listener) const {
	for(std::vector<Listener *>::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i) {
		if(*i == &listener) {
			return true;
		}
	}

	return false;
}

size_t DOSBoxVersionCollection::indexOfListener(const Listener & listener) const {
	for(size_t i = 0; i < m_listeners.size(); i++) {
		if(m_listeners[i] == &listener) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

DOSBoxVersionCollection::Listener * DOSBoxVersionCollection::getListener(size_t index) const {
	if(index >= m_listeners.size()) {
		return nullptr;
	}

	return m_listeners[index];
}

bool DOSBoxVersionCollection::addListener(Listener & listener) {
	if(!hasListener(listener)) {
		m_listeners.push_back(&listener);

		return true;
	}

	return false;
}

bool DOSBoxVersionCollection::removeListener(size_t index) {
	if(index >= m_listeners.size()) {
		return false;
	}

	m_listeners.erase(m_listeners.begin() + index);

	return true;
}

bool DOSBoxVersionCollection::removeListener(const Listener & listener) {
	for(std::vector<Listener *>::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i) {
		if(*i == &listener) {
			m_listeners.erase(i);

			return true;
		}
	}

	return false;
}

void DOSBoxVersionCollection::clearListeners() {
	m_listeners.clear();
}

void DOSBoxVersionCollection::notifyCollectionSizeChanged() {
	for(size_t i = 0; i < numberOfListeners(); i++) {
		getListener(i)->dosboxVersionCollectionSizeChanged(*this);
	}
}

bool DOSBoxVersionCollection::isValid() const {
	for(std::vector<std::shared_ptr<DOSBoxVersion>>::const_iterator i = m_dosboxVersions.begin(); i != m_dosboxVersions.end(); ++i) {
		if(!(*i)->isValid()) {
			return false;
		}

		for(std::vector<std::shared_ptr<DOSBoxVersion>>::const_iterator j = i + 1; j != m_dosboxVersions.end(); ++j) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), (*j)->getName())) {
				return false;
			}
		}
	}

	return true;
}

bool DOSBoxVersionCollection::isValid(const DOSBoxVersionCollection * g) {
	return g != nullptr && g->isValid();
}

bool DOSBoxVersionCollection::operator == (const DOSBoxVersionCollection & g) const {
	if(m_dosboxVersions.size() != g.m_dosboxVersions.size()) {
		return false;
	}

	for(size_t i = 0; i < g.m_dosboxVersions.size(); i++) {
		if(*m_dosboxVersions[i] != *g.m_dosboxVersions[i]) {
			return false;
		}
	}

	return true;
}

bool DOSBoxVersionCollection::operator != (const DOSBoxVersionCollection & g) const {
	return !operator == (g);
}

void DOSBoxVersionCollection::dosboxVersionModified(DOSBoxVersion & dosboxVersion) {
	notifyDOSBoxVersionModified(dosboxVersion);
}

void DOSBoxVersionCollection::notifyDOSBoxVersionModified(DOSBoxVersion & dosboxVersion) {
	for(size_t i = 0; i < numberOfListeners(); i++) {
		getListener(i)->dosboxVersionCollectionItemModified(*this, dosboxVersion);
	}
}

DOSBoxVersionCollection::Listener::~Listener() { }
