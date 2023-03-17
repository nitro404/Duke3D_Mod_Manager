#ifndef _FAVOURITE_MOD_COLLECTION_H_
#define _FAVOURITE_MOD_COLLECTION_H_

#include <boost/signals2.hpp>
#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class ModIdentifier;
class ModCollection;

class FavouriteModCollection final {
public:
	FavouriteModCollection();
	FavouriteModCollection(FavouriteModCollection && m) noexcept;
	FavouriteModCollection(const FavouriteModCollection & m);
	FavouriteModCollection & operator = (FavouriteModCollection && m) noexcept;
	FavouriteModCollection & operator = (const FavouriteModCollection & m);
	~FavouriteModCollection();

	size_t numberOfFavourites();
	bool hasFavourite(const ModIdentifier & favourite) const;
	bool hasFavourite(const std::string & name, const std::string & version = std::string(), const std::string & versionType = std::string()) const;
	size_t indexOfFavourite(const ModIdentifier & favourite) const;
	size_t indexOfFavourite(const std::string & name, const std::string & version = std::string(), const std::string & versionType = std::string()) const;
	std::shared_ptr<ModIdentifier> getFavourite(size_t index) const;
	std::shared_ptr<ModIdentifier> getFavourite(const std::string & name, const std::string & version = std::string(), const std::string & versionType = std::string()) const;
	bool addFavourite(const ModIdentifier & favourite);
	bool removeFavourite(size_t index);
	bool removeFavourite(const ModIdentifier & favourite);
	bool removeFavourite(const std::string & name, const std::string & version = std::string(), const std::string & versionType = std::string());
	void clearFavourites();

	rapidjson::Document toJSON() const;
	bool parseFrom(const rapidjson::Value & favourites);

	bool loadFrom(const std::string & filePath);
	bool loadFromJSON(const std::string & filePath);
	bool saveTo(const std::string & filePath, bool overwrite = true);
	bool saveToJSON(const std::string & filePath, bool overwrite = true);

	size_t checkForMissingFavouriteMods(const ModCollection & mods) const;

	bool operator == (const FavouriteModCollection & m) const;
	bool operator != (const FavouriteModCollection & m) const;

	boost::signals2::signal<void (FavouriteModCollection & /* favouriteMods */)> updated;

private:
	std::vector<std::shared_ptr<ModIdentifier>> m_favourites;
};

#endif // _FAVOURITE_MOD_COLLECTION_H_
