#include "StandAloneMod.h"

#include "Mod.h"
#include "ModGameVersion.h"
#include "ModVersion.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

static constexpr const char * JSON_VERSION_PROPERTY_NAME = "version";

StandAloneMod::StandAloneMod(const ModGameVersion & standAloneModGameVersion, const std::string & standAloneModDirectoryPath)
	: GameVersion(standAloneModGameVersion.isStandAlone() ? *standAloneModGameVersion.getStandAloneGameVersion() : GameVersion())
	, m_version(standAloneModGameVersion.isValid(true) ? standAloneModGameVersion.getParentModVersion()->getVersion() : "") {
	setStandAlone(true);
	setGamePath(standAloneModDirectoryPath);
}

StandAloneMod::StandAloneMod(GameVersion && gameVersion, const std::string & version) noexcept
	: GameVersion(gameVersion)
	, m_version(version) {
	setStandAlone(true);
}

StandAloneMod::StandAloneMod(StandAloneMod && standAloneMod) noexcept
	: GameVersion(standAloneMod) { }

StandAloneMod::StandAloneMod(const StandAloneMod & standAloneMod)
	: GameVersion(standAloneMod) { }

StandAloneMod & StandAloneMod::operator = (StandAloneMod && standAloneMod) noexcept {
	if(this != &standAloneMod) {
		GameVersion::operator = (standAloneMod);
	}

	return *this;
}

StandAloneMod & StandAloneMod::operator = (const StandAloneMod & standAloneMod) {
	GameVersion::operator = (standAloneMod);

	return *this;
}

StandAloneMod::~StandAloneMod() { }

std::string StandAloneMod::getVersion() const {
	return m_version;
}

rapidjson::Value StandAloneMod::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value standAloneModValue(GameVersion::toJSON(allocator));

	rapidjson::Value versionValue(m_version.c_str(), allocator);
	standAloneModValue.AddMember(rapidjson::StringRef(JSON_VERSION_PROPERTY_NAME), versionValue, allocator);

	return std::move(standAloneModValue);
}

std::unique_ptr<StandAloneMod> StandAloneMod::parseFrom(const rapidjson::Value & standAloneModValue) {
	std::unique_ptr<GameVersion> gameVersion(GameVersion::parseFrom(standAloneModValue));

	if(gameVersion == nullptr) {
		return nullptr;
	}

	// parse stand-alone mod version
	if(!standAloneModValue.HasMember(JSON_VERSION_PROPERTY_NAME)) {
		spdlog::error("Stand-alone mod is missing '{}' property.", JSON_VERSION_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & versionValue = standAloneModValue[JSON_VERSION_PROPERTY_NAME];

	if(!versionValue.IsString()) {
		spdlog::error("Stand-alone mod has an invalid '{}' property type: '{}', expected 'string'.", JSON_VERSION_PROPERTY_NAME, Utilities::typeToString(versionValue.GetType()));
		return nullptr;
	}

	return std::unique_ptr<StandAloneMod>(new StandAloneMod(std::move(*gameVersion), Utilities::trimString(versionValue.GetString())));
}

bool StandAloneMod::isValid() const {
	return GameVersion::isValid();
}

bool StandAloneMod::isValid(const StandAloneMod * standAloneMod) {
	return standAloneMod != nullptr && standAloneMod->isValid();
}

bool StandAloneMod::operator == (const StandAloneMod & standAloneMod) const {
	return GameVersion::operator == (standAloneMod) &&
		   Utilities::areStringsEqualIgnoreCase(m_version, standAloneMod.m_version);
}

bool StandAloneMod::operator != (const StandAloneMod & standAloneMod) const {
	return !operator == (standAloneMod);
}
