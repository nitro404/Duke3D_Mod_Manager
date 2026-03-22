#ifndef _ADDON_GROUP_FILE_GENERATOR_H_
#define _ADDON_GROUP_FILE_GENERATOR_H_

#include "Game/File/Group/Group.h"

#include <Singleton/Singleton.h>

#include <memory>

class AddonGroupFileGenerator final : public Singleton<AddonGroupFileGenerator> {
public:

private:
	AddonGroupFileGenerator();

	static std::unique_ptr<Group> generateDukeCaribbeanLifesABeachGroup();
	static std::unique_ptr<Group> generateDukeItOutInDCGroup();
	static std::unique_ptr<Group> generateNuclearWinterGroup();

	AddonGroupFileGenerator(const AddonGroupFileGenerator &) = delete;
	const AddonGroupFileGenerator & operator = (const AddonGroupFileGenerator &) = delete;
};

#endif // _ADDON_GROUP_FILE_GENERATOR_H_
