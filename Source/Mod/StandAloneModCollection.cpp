#include "StandAloneModCollection.h"

#include "Mod.h"
#include "ModGameVersion.h"
#include "ModVersion.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

static constexpr const char * JSON_FILE_FORMAT_VERSION_PROPERTY_NAME = "fileFormatVersion";
static constexpr const char * JSON_STANDALONE_MODS_PROPERTY_NAME = "standAloneMods";

const std::string StandAloneModCollection::FILE_FORMAT_VERSION = "1.0.0";

StandAloneModCollection::StandAloneModCollection() { }

StandAloneModCollection::StandAloneModCollection(StandAloneModCollection && standAloneMods) noexcept
	: m_standAloneMods(std::move(standAloneMods.m_standAloneMods)) { }

StandAloneModCollection::StandAloneModCollection(const StandAloneModCollection & standAloneMods) {
	for(const std::shared_ptr<StandAloneMod> standAloneMod : standAloneMods.m_standAloneMods) {
		m_standAloneMods.push_back(std::make_shared<StandAloneMod>(*standAloneMod));
	}
}

StandAloneModCollection & StandAloneModCollection::operator = (StandAloneModCollection && standAloneMods) noexcept {
	if(this != &standAloneMods) {
		m_standAloneMods = std::move(standAloneMods.m_standAloneMods);
	}

	return *this;
}

StandAloneModCollection & StandAloneModCollection::operator = (const StandAloneModCollection & standAloneMods) {
	for(const std::shared_ptr<StandAloneMod> standAloneMod : standAloneMods.m_standAloneMods) {
		m_standAloneMods.push_back(std::make_shared<StandAloneMod>(*standAloneMod));
	}

	return *this;
}

StandAloneModCollection::~StandAloneModCollection() = default;

size_t StandAloneModCollection::numberOfStandAloneMods() const {
	return m_standAloneMods.size();
}

bool StandAloneModCollection::hasStandAloneMod(const std::string & modID, const std::string & modVersion) const {
	return indexOfStandAloneMod(modID, modVersion) != std::numeric_limits<size_t>::max();
}

bool StandAloneModCollection::hasStandAloneMod(const StandAloneMod & standAloneMod) const {
	return indexOfStandAloneMod(standAloneMod) != std::numeric_limits<size_t>::max();
}

bool StandAloneModCollection::hasStandAloneMod(const ModGameVersion & modGameVersion) const {
	return indexOfStandAloneMod(modGameVersion) != std::numeric_limits<size_t>::max();
}

bool StandAloneModCollection::isStandAloneModInstalled(const std::string & modID, const std::string & modVersion) const {
	size_t standAloneModIndex = indexOfStandAloneMod(modID, modVersion);

	if(standAloneModIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	return m_standAloneMods[standAloneModIndex]->isConfigured();
}

bool StandAloneModCollection::isStandAloneModInstalled(const ModVersion & modVersion) const {
	size_t standAloneModIndex = indexOfStandAloneMod(modVersion);

	if(standAloneModIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	return m_standAloneMods[standAloneModIndex]->isConfigured();
}

bool StandAloneModCollection::isStandAloneModInstalled(const ModGameVersion & modGameVersion) const {
	size_t standAloneModIndex = indexOfStandAloneMod(modGameVersion);

	if(standAloneModIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	return m_standAloneMods[standAloneModIndex]->isConfigured();
}

size_t StandAloneModCollection::indexOfStandAloneMod(const std::string & modID, const std::string & modVersion) const {
	if(modID.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	auto standAloneModIterator = std::find_if(m_standAloneMods.cbegin(), m_standAloneMods.cend(), [&modID, &modVersion](const std::shared_ptr<StandAloneMod> & currentStandAloneMod) {
		return Utilities::areStringsEqualIgnoreCase(currentStandAloneMod->getID(), modID) &&
			   Utilities::areStringsEqualIgnoreCase(currentStandAloneMod->getVersion(), modVersion);
	});

	if(standAloneModIterator == m_standAloneMods.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return standAloneModIterator - m_standAloneMods.cbegin();
}

size_t StandAloneModCollection::indexOfStandAloneMod(const StandAloneMod & standAloneMod) const {
	auto standAloneModIterator = std::find_if(m_standAloneMods.cbegin(), m_standAloneMods.cend(), [&standAloneMod](const std::shared_ptr<StandAloneMod> & currentStandAloneMod) {
		return &standAloneMod == currentStandAloneMod.get();
	});

	if(standAloneModIterator == m_standAloneMods.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return standAloneModIterator - m_standAloneMods.cbegin();
}

size_t StandAloneModCollection::indexOfStandAloneMod(const ModVersion & modVersion) const {
	if(!modVersion.isValid(true)) {
		return std::numeric_limits<size_t>::max();
	}

	return indexOfStandAloneMod(modVersion.getParentMod()->getID(), modVersion.getVersion());
}

size_t StandAloneModCollection::indexOfStandAloneMod(const ModGameVersion & modGameVersion) const {
	if(!modGameVersion.isValid(true)) {
		return std::numeric_limits<size_t>::max();
	}

	return indexOfStandAloneMod(modGameVersion.getParentMod()->getID(), modGameVersion.getParentModVersion()->getVersion());
}

std::shared_ptr<StandAloneMod> StandAloneModCollection::getStandAloneMod(size_t index) const {
	if(index >= m_standAloneMods.size()) {
		return nullptr;
	}

	return m_standAloneMods[index];
}

std::shared_ptr<StandAloneMod> StandAloneModCollection::getStandAloneMod(const std::string & modID, const std::string & modVersion) const {
	size_t standAloneModIndex = indexOfStandAloneMod(modID, modVersion);

	if(standAloneModIndex == std::numeric_limits<size_t>::max()) {
		return nullptr;
	}

	return m_standAloneMods[standAloneModIndex];
}

std::shared_ptr<StandAloneMod> StandAloneModCollection::getStandAloneMod(const ModVersion & modVersion) const {
	size_t standAloneModIndex = indexOfStandAloneMod(modVersion);

	if(standAloneModIndex == std::numeric_limits<size_t>::max()) {
		return nullptr;
	}

	return m_standAloneMods[standAloneModIndex];
}

std::shared_ptr<StandAloneMod> StandAloneModCollection::getStandAloneMod(const ModGameVersion & modGameVersion) const {
	size_t standAloneModIndex = indexOfStandAloneMod(modGameVersion);

	if(standAloneModIndex == std::numeric_limits<size_t>::max()) {
		return nullptr;
	}

	return m_standAloneMods[standAloneModIndex];
}

const std::vector<std::shared_ptr<StandAloneMod>> & StandAloneModCollection::getStandAloneMods() const {
	return m_standAloneMods;
}


bool StandAloneModCollection::addStandAloneMod(const StandAloneMod & standAloneMod) {
	if(!standAloneMod.isValid() || hasStandAloneMod(standAloneMod.getID(), standAloneMod.getVersion())) {
		return false;
	}

	m_standAloneMods.emplace_back(std::make_shared<StandAloneMod>(standAloneMod));

	return true;
}

bool StandAloneModCollection::addStandAloneMod(std::shared_ptr<StandAloneMod> standAloneMod) {
	if(!StandAloneMod::isValid(standAloneMod.get()) || hasStandAloneMod(standAloneMod->getID(), standAloneMod->getVersion())) {
		return false;
	}

	m_standAloneMods.push_back(standAloneMod);

	return true;
}

bool StandAloneModCollection::removeStandAloneMod(size_t index) {
	if(index >= m_standAloneMods.size()) {
		return false;
	}

	m_standAloneMods.erase(m_standAloneMods.begin() + index);

	return true;
}

bool StandAloneModCollection::removeStandAloneMod(const std::string & modID, const std::string & modVersion) {
	return removeStandAloneMod(indexOfStandAloneMod(modID, modVersion));
}

bool StandAloneModCollection::removeStandAloneMod(const StandAloneMod & standAloneMod) {
	return removeStandAloneMod(indexOfStandAloneMod(standAloneMod));
}

bool StandAloneModCollection::removeStandAloneMod(const ModVersion & modVersion) {
	return removeStandAloneMod(indexOfStandAloneMod(modVersion));
}

bool StandAloneModCollection::removeStandAloneMod(const ModGameVersion & modGameVersion) {
	return removeStandAloneMod(indexOfStandAloneMod(modGameVersion));
}

void StandAloneModCollection::clearStandAloneMods() {
	m_standAloneMods.clear();
}

size_t StandAloneModCollection::checkForMissingExecutables() const {
	size_t numberOfMissingExecutables = 0;

	for(const std::shared_ptr<StandAloneMod> & standAloneMod : m_standAloneMods) {
		numberOfMissingExecutables += standAloneMod->checkForMissingExecutables();
	}

	return numberOfMissingExecutables++;
}

rapidjson::Document StandAloneModCollection::toJSON() const {
	rapidjson::Document standAloneModsDocument(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = standAloneModsDocument.GetAllocator();

	rapidjson::Value fileFormatVersionValue(FILE_FORMAT_VERSION.c_str(), allocator);
	standAloneModsDocument.AddMember(rapidjson::StringRef(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME), fileFormatVersionValue, allocator);

	rapidjson::Value standAloneModsValue(rapidjson::kArrayType);

	for(const std::shared_ptr<StandAloneMod> & standAloneMod : m_standAloneMods) {
		standAloneModsValue.PushBack(standAloneMod->toJSON(allocator), allocator);
	}

	standAloneModsDocument.AddMember(rapidjson::StringRef(JSON_STANDALONE_MODS_PROPERTY_NAME), standAloneModsValue, allocator);

	return standAloneModsDocument;
}

std::unique_ptr<StandAloneModCollection> StandAloneModCollection::parseFrom(const rapidjson::Value & standAloneModCollectionValue) {
	if(!standAloneModCollectionValue.IsObject()) {
		spdlog::error("Invalid stand-alone mod collection type: '{}', expected 'object'.", Utilities::typeToString(standAloneModCollectionValue.GetType()));
		return nullptr;
	}

	if(standAloneModCollectionValue.HasMember(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & fileFormatVersionValue = standAloneModCollectionValue[JSON_FILE_FORMAT_VERSION_PROPERTY_NAME];

		if(!fileFormatVersionValue.IsString()) {
			spdlog::error("Invalid stand-alone mod collection file format version type: '{}', expected: 'string'.", Utilities::typeToString(fileFormatVersionValue.GetType()));
			return false;
		}

		std::optional<std::uint8_t> optionalVersionComparison(Utilities::compareVersions(fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION));

		if(!optionalVersionComparison.has_value()) {
			spdlog::error("Invalid stand-alone mod collection file format version: '{}'.", fileFormatVersionValue.GetString());
			return false;
		}

		if(*optionalVersionComparison != 0) {
			spdlog::error("Unsupported stand-alone mod collection file format version: '{}', only version '{}' is supported.", fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION);
			return false;
		}
	}
	else {
		spdlog::warn("Stand-alone mod collection JSON data is missing file format version, and may fail to load correctly!");
	}

	if(!standAloneModCollectionValue.HasMember(JSON_STANDALONE_MODS_PROPERTY_NAME)) {
		spdlog::error("Stand-alone mod collection is missing '{}' property'.", JSON_STANDALONE_MODS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & standAloneModsValue = standAloneModCollectionValue[JSON_STANDALONE_MODS_PROPERTY_NAME];

	if(!standAloneModsValue.IsArray()) {
		spdlog::error("Invalid stand-alone mod collection '{}' type: '{}', expected 'array'.", JSON_STANDALONE_MODS_PROPERTY_NAME, Utilities::typeToString(standAloneModsValue.GetType()));
		return nullptr;
	}

	std::unique_ptr<StandAloneModCollection> newStandAloneModCollection(std::make_unique<StandAloneModCollection>());

	if(standAloneModsValue.Empty()) {
		return newStandAloneModCollection;
	}

	std::unique_ptr<StandAloneMod> newStandAloneMod;

	for(rapidjson::Value::ConstValueIterator i = standAloneModsValue.Begin(); i != standAloneModsValue.End(); ++i) {
		newStandAloneMod = StandAloneMod::parseFrom(*i);

		if(!StandAloneMod::isValid(newStandAloneMod.get())) {
			spdlog::error("Failed to parse stand-alone mod #{}{}!", newStandAloneModCollection->m_standAloneMods.size() + 1, newStandAloneModCollection->numberOfStandAloneMods() == 0 ? "" : fmt::format(" (after stand-alone mod '{}')", newStandAloneModCollection->getStandAloneMod(newStandAloneModCollection->numberOfStandAloneMods() - 1)->getLongName()));
			return nullptr;
		}

		if(newStandAloneModCollection->hasStandAloneMod(newStandAloneMod->getID(), newStandAloneMod->getVersion())) {
			spdlog::error("Encountered duplicate stand-alone mod '{}' version '{}' #{}{}.", newStandAloneMod->getLongName(), newStandAloneMod->getVersion(), newStandAloneModCollection->m_standAloneMods.size() + 1, newStandAloneModCollection->numberOfStandAloneMods() == 0 ? "" : fmt::format(" (after stand-alone mod '{}')", newStandAloneModCollection->getStandAloneMod(newStandAloneModCollection->numberOfStandAloneMods() - 1)->getLongName()));
			return nullptr;
		}

		newStandAloneModCollection->m_standAloneMods.push_back(std::shared_ptr<StandAloneMod>(newStandAloneMod.release()));
	}

	return newStandAloneModCollection;
}

bool StandAloneModCollection::loadFrom(const std::string & filePath, bool autoCreate) {
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

bool StandAloneModCollection::loadFromJSON(const std::string & filePath, bool autoCreate) {
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

	rapidjson::Document standAloneModsValue;
	rapidjson::IStreamWrapper fileStreamWrapper(fileStream);
	standAloneModsValue.ParseStream(fileStreamWrapper);

	fileStream.close();

	std::unique_ptr<StandAloneModCollection> standAloneModCollection(parseFrom(standAloneModsValue));

	if(!StandAloneModCollection::isValid(standAloneModCollection.get())) {
		spdlog::error("Failed to parse stand-alone mod collection from JSON file '{}'.", filePath);
		return false;
	}

	m_standAloneMods = standAloneModCollection->m_standAloneMods;

	return true;
}

bool StandAloneModCollection::saveTo(const std::string & filePath, bool overwrite) const {
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

bool StandAloneModCollection::saveToJSON(const std::string & filePath, bool overwrite) const {
	if (!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		spdlog::warn("File '{}' already exists, use overwrite to force write.", filePath);
		return false;
	}

	std::ofstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::Document standAloneMods(toJSON());

	rapidjson::OStreamWrapper fileStreamWrapper(fileStream);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> fileStreamWriter(fileStreamWrapper);
	fileStreamWriter.SetIndent('\t', 1);
	standAloneMods.Accept(fileStreamWriter);
	fileStream.close();

	return true;
}

bool StandAloneModCollection::isValid() const {
	for(std::vector<std::shared_ptr<StandAloneMod>>::const_iterator i = m_standAloneMods.begin(); i != m_standAloneMods.end(); ++i) {
		if(!(*i)->isValid()) {
			return false;
		}

		for(std::vector<std::shared_ptr<StandAloneMod>>::const_iterator j = i + 1; j != m_standAloneMods.end(); ++j) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), (*j)->getID()) &&
			   Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), (*j)->getVersion())) {
				return false;
			}
		}
	}

	return true;
}

bool StandAloneModCollection::isValid(const StandAloneModCollection * standAloneMods) {
	return standAloneMods != nullptr && standAloneMods->isValid();
}

bool StandAloneModCollection::operator == (const StandAloneModCollection & standAloneMods) const {
	if(m_standAloneMods.size() != standAloneMods.m_standAloneMods.size()) {
		return false;
	}

	for(size_t i = 0; i < standAloneMods.m_standAloneMods.size(); i++) {
		if(*m_standAloneMods[i] != *standAloneMods.m_standAloneMods[i]) {
			return false;
		}
	}

	return true;
}

bool StandAloneModCollection::operator != (const StandAloneModCollection & standAloneMods) const {
	return !operator == (standAloneMods);
}
