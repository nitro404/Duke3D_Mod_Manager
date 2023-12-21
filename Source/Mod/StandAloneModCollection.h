#ifndef _STAND_ALONE_MOD_COLLECTION_H_
#define _STAND_ALONE_MOD_COLLECTION_H_

#include "StandAloneMod.h"

#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class ModGameVersion;
class ModVersion;

class StandAloneModCollection final {
public:
	StandAloneModCollection();
	StandAloneModCollection(StandAloneModCollection && standAloneMods) noexcept;
	StandAloneModCollection(const StandAloneModCollection & standAloneMods);
	StandAloneModCollection & operator = (StandAloneModCollection && standAloneMods) noexcept;
	StandAloneModCollection & operator = (const StandAloneModCollection & standAloneMods);
	~StandAloneModCollection();

	size_t numberOfStandAloneMods() const;
	bool hasStandAloneMod(const std::string & modID, const std::string & modVersion) const;
	bool hasStandAloneMod(const StandAloneMod & standAloneMod) const;
	bool hasStandAloneMod(const ModVersion & modVersion) const;
	bool hasStandAloneMod(const ModGameVersion & modGameVersion) const;
	bool isStandAloneModInstalled(const std::string & modID, const std::string & modVersion) const;
	bool isStandAloneModInstalled(const ModVersion & modVersion) const;
	bool isStandAloneModInstalled(const ModGameVersion & modGameVersion) const;
	size_t indexOfStandAloneMod(const std::string & modID, const std::string & modVersion) const;
	size_t indexOfStandAloneMod(const StandAloneMod & standAloneMod) const;
	size_t indexOfStandAloneMod(const ModVersion & modVersion) const;
	size_t indexOfStandAloneMod(const ModGameVersion & modGameVersion) const;
	std::shared_ptr<StandAloneMod> getStandAloneMod(size_t index) const;
	std::shared_ptr<StandAloneMod> getStandAloneMod(const std::string & modID, const std::string & modVersion) const;
	std::shared_ptr<StandAloneMod> getStandAloneMod(const ModVersion & modVersion) const;
	std::shared_ptr<StandAloneMod> getStandAloneMod(const ModGameVersion & modGameVersion) const;
	const std::vector<std::shared_ptr<StandAloneMod>> & getStandAloneMods() const;
	bool addStandAloneMod(const StandAloneMod & standAloneMod);
	bool addStandAloneMod(std::shared_ptr<StandAloneMod> standAloneMod);
	bool removeStandAloneMod(size_t index);
	bool removeStandAloneMod(const std::string & modID, const std::string & modVersion);
	bool removeStandAloneMod(const StandAloneMod & standAloneMod);
	bool removeStandAloneMod(const ModVersion & modVersion);
	bool removeStandAloneMod(const ModGameVersion & modGameVersion);
	void clearStandAloneMods();

	size_t checkForMissingExecutables() const;

	rapidjson::Document toJSON() const;
	static std::unique_ptr<StandAloneModCollection> parseFrom(const rapidjson::Value & standAloneModCollectionValue);

	bool loadFrom(const std::string & filePath, bool autoCreate = true);
	bool loadFromJSON(const std::string & filePath, bool autoCreate = true);
	bool saveTo(const std::string & filePath, bool overwrite = true) const;
	bool saveToJSON(const std::string & filePath, bool overwrite = true) const;

	bool isValid() const;
	static bool isValid(const StandAloneModCollection * standAloneMods);

	bool operator == (const StandAloneModCollection & standAloneMods) const;
	bool operator != (const StandAloneModCollection & standAloneMods) const;

	static const std::string FILE_TYPE;
	static const uint32_t FILE_FORMAT_VERSION;

private:
	std::vector<std::shared_ptr<StandAloneMod>> m_standAloneMods;
};

#endif // _STAND_ALONE_MOD_COLLECTION_H_
