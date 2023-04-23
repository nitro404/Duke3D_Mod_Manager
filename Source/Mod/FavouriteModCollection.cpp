#include "FavouriteModCollection.h"

#include "Manager/ModMatch.h"
#include "ModIdentifier.h"
#include "Mod.h"
#include "ModCollection.h"
#include "ModVersion.h"
#include "ModVersionType.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/Utilities.h>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <filesystem>
#include <fstream>

static constexpr const char * JSON_FILE_TYPE_PROPERTY_NAME = "fileType";
static constexpr const char * JSON_FILE_FORMAT_VERSION_PROPERTY_NAME = "fileFormatVersion";
static constexpr const char * JSON_FAVOURITE_MODS_PROPERTY_NAME = "favouriteMods";

const std::string FavouriteModCollection::FILE_TYPE = "Favourite Mods";
const std::string FavouriteModCollection::FILE_FORMAT_VERSION = "1.0.0";

FavouriteModCollection::FavouriteModCollection() { }

FavouriteModCollection::FavouriteModCollection(FavouriteModCollection && m) noexcept
	: m_favourites(std::move(m.m_favourites)) { }

FavouriteModCollection::FavouriteModCollection(const FavouriteModCollection & m) {
	for(std::vector<std::shared_ptr<ModIdentifier>>::const_iterator i = m.m_favourites.begin(); i != m.m_favourites.end(); ++i) {
		m_favourites.push_back(std::make_shared<ModIdentifier>(**i));
	}
}

FavouriteModCollection & FavouriteModCollection::operator = (FavouriteModCollection && m) noexcept {
	if(this != &m) {
		m_favourites = std::move(m.m_favourites);
	}

	updated(*this);

	return *this;
}

FavouriteModCollection & FavouriteModCollection::operator = (const FavouriteModCollection & m) {
	m_favourites.clear();

	for(std::vector<std::shared_ptr<ModIdentifier>>::const_iterator i = m.m_favourites.begin(); i != m.m_favourites.end(); ++i) {
		m_favourites.push_back(std::make_shared<ModIdentifier>(**i));
	}

	updated(*this);

	return *this;
}

FavouriteModCollection::~FavouriteModCollection() = default;

size_t FavouriteModCollection::numberOfFavourites() {
	return m_favourites.size();
}

bool FavouriteModCollection::hasFavourite(const ModIdentifier & favourite) const {
	return indexOfFavourite(favourite) != std::numeric_limits<size_t>::max();
}

bool FavouriteModCollection::hasFavourite(const ModMatch & favourite) const {
	return indexOfFavourite(favourite) != std::numeric_limits<size_t>::max();
}

bool FavouriteModCollection::hasFavourite(const std::string & name, const std::optional<std::string> & version, const std::optional<std::string> & versionType) const {
	return indexOfFavourite(name, version, versionType) != std::numeric_limits<size_t>::max();
}

size_t FavouriteModCollection::indexOfFavourite(const ModIdentifier & favourite) const {
	if(!favourite.isValid()) {
		return std::numeric_limits<size_t>::max();
	}

	std::vector<std::shared_ptr<ModIdentifier>>::const_iterator favouriteModIterator = std::find_if(m_favourites.cbegin(), m_favourites.cend(), [&favourite](const std::shared_ptr<ModIdentifier> & currentFavourite) {
		return *currentFavourite == favourite;
	});

	if(favouriteModIterator == m_favourites.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return favouriteModIterator - m_favourites.cbegin();
}

size_t FavouriteModCollection::indexOfFavourite(const ModMatch & favourite) const {
	return indexOfFavourite(favourite.getModName(), favourite.getModVersionName(), favourite.getModVersionTypeName());
}

size_t FavouriteModCollection::indexOfFavourite(const std::string & name, const std::optional<std::string> & version, const std::optional<std::string> & versionType) const {
	return indexOfFavourite(ModIdentifier(name, version, versionType));
}

std::shared_ptr<ModIdentifier> FavouriteModCollection::getFavourite(size_t index) const {
	if(index >= m_favourites.size()) {
		return nullptr;
	}

	return m_favourites[index];
}

std::shared_ptr<ModIdentifier> FavouriteModCollection::getFavourite(const ModMatch & favourite) const {
	return getFavourite(indexOfFavourite(favourite));
}

std::shared_ptr<ModIdentifier> FavouriteModCollection::getFavourite(const std::string & name, const std::optional<std::string> & version, const std::optional<std::string> & versionType) const {
	return getFavourite(indexOfFavourite(name, version, versionType));
}

bool FavouriteModCollection::addFavourite(const ModIdentifier & favourite) {
	if(!favourite.isValid() || hasFavourite(favourite)) {
		return false;
	}

	m_favourites.push_back(std::make_shared<ModIdentifier>(favourite));

	updated(*this);

	return true;
}

bool FavouriteModCollection::addFavourite(const ModMatch & modMatch) {
	if(!modMatch.isValid() || hasFavourite(modMatch)) {
		return false;
	}

	m_favourites.push_back(std::make_shared<ModIdentifier>(modMatch));

	updated(*this);

	return true;
}

bool FavouriteModCollection::removeFavourite(size_t index) {
	if(index >= m_favourites.size()) {
		return false;
	}

	m_favourites.erase(m_favourites.begin() + index);

	updated(*this);

	return true;
}

bool FavouriteModCollection::removeFavourite(const ModIdentifier & favourite) {
	return removeFavourite(indexOfFavourite(favourite));
}

bool FavouriteModCollection::removeFavourite(const ModMatch & favourite) {
	return removeFavourite(indexOfFavourite(favourite));
}

bool FavouriteModCollection::removeFavourite(const std::string & name, const std::optional<std::string> & version, const std::optional<std::string> & versionType) {
	return removeFavourite(indexOfFavourite(name, version, versionType));
}

void FavouriteModCollection::clearFavourites() {
	m_favourites.clear();

	updated(*this);
}

rapidjson::Document FavouriteModCollection::toJSON() const {
	rapidjson::Document favouriteModsDocument(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = favouriteModsDocument.GetAllocator();

	rapidjson::Value fileTypeValue(FILE_TYPE.c_str(), allocator);
	favouriteModsDocument.AddMember(rapidjson::StringRef(JSON_FILE_TYPE_PROPERTY_NAME), fileTypeValue, allocator);

	rapidjson::Value fileFormatVersionValue(FILE_FORMAT_VERSION.c_str(), allocator);
	favouriteModsDocument.AddMember(rapidjson::StringRef(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME), fileFormatVersionValue, allocator);

	rapidjson::Value favouriteModsListValue(rapidjson::kArrayType);

	for(std::vector<std::shared_ptr<ModIdentifier>>::const_iterator i = m_favourites.begin(); i != m_favourites.end(); ++i) {
		favouriteModsListValue.PushBack((*i)->toJSON(allocator), allocator);
	}

	favouriteModsDocument.AddMember(rapidjson::StringRef(JSON_FAVOURITE_MODS_PROPERTY_NAME), favouriteModsListValue, allocator);

	return favouriteModsDocument;
}

bool FavouriteModCollection::parseFrom(const rapidjson::Value & favouriteModsCollectionValue) {
	if(!favouriteModsCollectionValue.IsObject()) {
		spdlog::error("Invalid favourite mods collection type: '{}', expected 'object'.", Utilities::typeToString(favouriteModsCollectionValue.GetType()));
		return nullptr;
	}

	if(favouriteModsCollectionValue.HasMember(JSON_FILE_TYPE_PROPERTY_NAME)) {
		const rapidjson::Value & fileTypeValue = favouriteModsCollectionValue[JSON_FILE_TYPE_PROPERTY_NAME];

		if(!fileTypeValue.IsString()) {
			spdlog::error("Invalid favourite mods collection file type type: '{}', expected: 'string'.", Utilities::typeToString(fileTypeValue.GetType()));
			return false;
		}

		if(!Utilities::areStringsEqualIgnoreCase(fileTypeValue.GetString(), FILE_TYPE)) {
			spdlog::error("Incorrect favourite mods collection file type: '{}', expected: '{}'.", fileTypeValue.GetString(), FILE_TYPE);
			return false;
		}
	}
	else {
		spdlog::warn("Favourite mods collection JSON data is missing file type, and may fail to load correctly!");
	}

	if(favouriteModsCollectionValue.HasMember(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & fileFormatVersionValue = favouriteModsCollectionValue[JSON_FILE_FORMAT_VERSION_PROPERTY_NAME];

		if(!fileFormatVersionValue.IsString()) {
			spdlog::error("Invalid favourite mods collection version collection file format version type: '{}', expected: 'string'.", Utilities::typeToString(fileFormatVersionValue.GetType()));
			return false;
		}

		std::optional<std::uint8_t> optionalVersionComparison(Utilities::compareVersions(fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION));

		if(!optionalVersionComparison.has_value()) {
			spdlog::error("Invalid favourite mods collection file format version: '{}'.", fileFormatVersionValue.GetString());
			return false;
		}

		if(*optionalVersionComparison != 0) {
			spdlog::error("Unsupported favourite mods collection file format version: '{}', only version '{}' is supported.", fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION);
			return false;
		}
	}
	else {
		spdlog::warn("Favourite mods collection JSON data is missing file format version, and may fail to load correctly!");
	}

	if(!favouriteModsCollectionValue.HasMember(JSON_FAVOURITE_MODS_PROPERTY_NAME)) {
		spdlog::error("Favourite mods collection is missing '{}' property'.", JSON_FAVOURITE_MODS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & favouriteModsValue = favouriteModsCollectionValue[JSON_FAVOURITE_MODS_PROPERTY_NAME];

	if(!favouriteModsValue.IsArray()) {
		spdlog::error("Invalid favourite mods collection '{}' type: '{}', expected 'array'.", JSON_FAVOURITE_MODS_PROPERTY_NAME, Utilities::typeToString(favouriteModsValue.GetType()));
		return nullptr;
	}

	std::unique_ptr<ModIdentifier> newModIdentifier;
	std::vector<std::shared_ptr<ModIdentifier>> newFavourites;

	for(rapidjson::Value::ConstValueIterator i = favouriteModsValue.Begin(); i != favouriteModsValue.End(); ++i) {
		newModIdentifier = ModIdentifier::parseFrom(*i);

		if(!ModIdentifier::isValid(newModIdentifier.get())) {
			spdlog::error("Failed to parse favourite mod #{}.", newFavourites.size() + 1);
			continue;
		}

		newFavourites.push_back(std::shared_ptr<ModIdentifier>(newModIdentifier.release()));
	}

	m_favourites = newFavourites;

	updated(*this);

	return true;
}

bool FavouriteModCollection::loadFrom(const std::string & filePath) {
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

bool FavouriteModCollection::loadFromJSON(const std::string & filePath) {
	if(filePath.empty()) {
		return false;
	}

	std::ifstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::Document settings;
	rapidjson::IStreamWrapper fileStreamWrapper(fileStream);
	if(settings.ParseStream(fileStreamWrapper).HasParseError()) {
		return false;
	}

	fileStream.close();

	return parseFrom(settings);
}

bool FavouriteModCollection::saveTo(const std::string & filePath, bool overwrite) {
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

bool FavouriteModCollection::saveToJSON(const std::string & filePath, bool overwrite) {
	if (!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		spdlog::warn("File '{}' already exists, use overwrite to force write.", filePath);
		return false;
	}

	std::ofstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::Document settings(toJSON());

	rapidjson::OStreamWrapper fileStreamWrapper(fileStream);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> fileStreamWriter(fileStreamWrapper);
	fileStreamWriter.SetIndent('\t', 1);
	settings.Accept(fileStreamWriter);
	fileStream.close();

	return true;
}

size_t FavouriteModCollection::checkForMissingFavouriteMods(const ModCollection & mods) const {
	if(m_favourites.empty()) {
		return 0;
	}

	std::shared_ptr<Mod> mod;
	std::shared_ptr<ModVersion> modVersion;
	std::shared_ptr<ModVersionType> modVersionType;
	size_t numberOfMissingFavouriteMods = 0;

	for(const std::shared_ptr<ModIdentifier> & favouriteMod : m_favourites) {
		mod = mods.getModWithName(favouriteMod->getName());

		if(mod == nullptr) {
			numberOfMissingFavouriteMods++;

			spdlog::warn("Missing favourite mod #{}: '{}'.", numberOfMissingFavouriteMods, favouriteMod->getName());

			continue;
		}

		if(favouriteMod->hasVersion()) {
			modVersion = mod->getVersion(favouriteMod->getVersion().value());

			if(modVersion == nullptr) {
				numberOfMissingFavouriteMods++;

				spdlog::warn("Missing favourite mod #{}: '{}' with version: '{}'.", numberOfMissingFavouriteMods, favouriteMod->getName(), favouriteMod->getVersion().value());

				continue;
			}

			if(favouriteMod->hasVersionType()) {
				modVersionType = modVersion->getType(favouriteMod->getVersionType().value());

				if(modVersion == nullptr) {
					numberOfMissingFavouriteMods++;

					spdlog::warn("Missing favourite mod #{}: '{}' with version: '{}' and type: '{}'.", numberOfMissingFavouriteMods, favouriteMod->getName(), favouriteMod->getVersion().value(), favouriteMod->getVersionType().value());

					continue;
				}
			}
		}
	}

	if(numberOfMissingFavouriteMods > 0) {
		spdlog::warn("Missing {} favourite mod{}.", numberOfMissingFavouriteMods, numberOfMissingFavouriteMods == 1 ? "" : "s");
	}

	return numberOfMissingFavouriteMods;
}

bool FavouriteModCollection::operator == (const FavouriteModCollection & m) const {
	if(m_favourites.size() != m.m_favourites.size()) {
		return false;
	}

	for(size_t i = 0; i < m_favourites.size(); i++) {
		if(*m_favourites[i] != *m.m_favourites[i]) {
			return false;
		}
	}

	return true;
}

bool FavouriteModCollection::operator != (const FavouriteModCollection & m) const {
	return !operator == (m);
}
