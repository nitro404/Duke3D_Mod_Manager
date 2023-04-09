#ifndef _STAND_ALONE_MOD_H_
#define _STAND_ALONE_MOD_H_

#include "Game/GameVersion.h"

#include <memory>
#include <string>

class Mod;
class ModGameVersion;
class ModVersion;

class StandAloneMod final : public GameVersion {
public:
	StandAloneMod(const ModGameVersion & standAloneModGameVersion, const std::string & standAloneModDirectoryPath);
	StandAloneMod(StandAloneMod && standAloneMod) noexcept;
	StandAloneMod(const StandAloneMod & standAloneMod);
	StandAloneMod & operator = (StandAloneMod && standAloneMod) noexcept;
	StandAloneMod & operator = (const StandAloneMod & standAloneMod);
	virtual ~StandAloneMod();

	std::string getVersion() const;

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<StandAloneMod> parseFrom(const rapidjson::Value & standAloneModValue);

	virtual bool isValid() const override;
	static bool isValid(const StandAloneMod * standAloneMod);

	bool operator == (const StandAloneMod & standAloneMod) const;
	bool operator != (const StandAloneMod & standAloneMod) const;

private:
	StandAloneMod(GameVersion && gameVersion, const std::string & version) noexcept;

	std::string m_version;
};

#endif // _STAND_ALONE_MOD_H_
