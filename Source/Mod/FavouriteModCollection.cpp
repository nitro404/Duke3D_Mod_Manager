#include "FavouriteModCollection.h"

#include "ModIdentifier.h"
#include "Mod.h"
#include "ModCollection.h"
#include "ModVersion.h"
#include "ModVersionType.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/Utilities.h>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <filesystem>
#include <fstream>

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

bool FavouriteModCollection::hasFavourite(const std::string & name, const std::string & version, const std::string & versionType) const {
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

size_t FavouriteModCollection::indexOfFavourite(const std::string & name, const std::string & version, const std::string & versionType) const {
	return indexOfFavourite(ModIdentifier(name, version, versionType));
}

std::shared_ptr<ModIdentifier> FavouriteModCollection::getFavourite(size_t index) const {
	if(index >= m_favourites.size()) {
		return nullptr;
	}

	return m_favourites[index];
}

std::shared_ptr<ModIdentifier> FavouriteModCollection::getFavourite(const std::string & name, const std::string & version, const std::string & versionType) const {
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

bool FavouriteModCollection::removeFavourite(const std::string & name, const std::string & version, const std::string & versionType) {
	return removeFavourite(indexOfFavourite(name, version, versionType));
}

void FavouriteModCollection::clearFavourites() {
	m_favourites.clear();

	updated(*this);
}

rapidjson::Document FavouriteModCollection::toJSON() const {
	rapidjson::Document favourites(rapidjson::kArrayType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = favourites.GetAllocator();

	for(std::vector<std::shared_ptr<ModIdentifier>>::const_iterator i = m_favourites.begin(); i != m_favourites.end(); ++i) {
		favourites.PushBack((*i)->toJSON(allocator), allocator);
	}

	return favourites;
}

bool FavouriteModCollection::parseFrom(const rapidjson::Value & favourites) {
	if(!favourites.IsArray()) {
		spdlog::error("Invalid favourites value, expected array.");
		return false;
	}

	std::unique_ptr<ModIdentifier> newModIdentifier;
	std::vector<std::shared_ptr<ModIdentifier>> newFavourites;

	for(rapidjson::Value::ConstValueIterator i = favourites.Begin(); i != favourites.End(); ++i) {
		newModIdentifier = ModIdentifier::parseFrom(*i);

		if(!ModIdentifier::isValid(newModIdentifier.get())) {
			spdlog::error("Failed to parse favourite mod #{}.", newFavourites.size() + 1);
			return false;
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

	for(std::vector<std::shared_ptr<ModIdentifier>>::const_iterator i = m_favourites.begin(); i != m_favourites.end(); ++i) {
		mod = mods.getModWithName((*i)->getName());

		if(mod == nullptr) {
			numberOfMissingFavouriteMods++;

			spdlog::warn("Missing favourite mod #{}: '{}'.", numberOfMissingFavouriteMods, (*i)->getName());

			continue;
		}

		modVersion = mod->getVersion((*i)->getVersion());

		if(modVersion == nullptr) {
			numberOfMissingFavouriteMods++;

			spdlog::warn("Missing favourite mod #{}: '{}' with version: '{}'.", numberOfMissingFavouriteMods, (*i)->getName(), (*i)->getVersion());

			continue;
		}

		modVersionType = modVersion->getType((*i)->getVersionType());

		if(modVersion == nullptr) {
			numberOfMissingFavouriteMods++;

			spdlog::warn("Missing favourite mod #{}: '{}' with version: '{}' and type: '{}'.", numberOfMissingFavouriteMods, (*i)->getName(), (*i)->getVersion(), (*i)->getVersionType());

			continue;
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
