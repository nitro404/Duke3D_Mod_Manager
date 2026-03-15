#ifndef _ADDON_GROUP_FILE_LOCATOR_H_
#define _ADDON_GROUP_FILE_LOCATOR_H_

#include <Singleton/Singleton.h>

class AddonGroupFileLocator final : public Singleton<AddonGroupFileLocator> {
public:

private:
	AddonGroupFileLocator();

	//static std::vector<std::string> getDukeCaribbeanLifesABeachGroupFilePaths() const;
	//static std::vector<std::string> getDukeItOutInDCGroupFilePaths() const;
	//static std::vector<std::string> getNuclearWinterGroupFilePaths() const;

	AddonGroupFileLocator(const AddonGroupFileLocator &) = delete;
	const AddonGroupFileLocator & operator = (const AddonGroupFileLocator &) = delete;
};

#endif // _ADDON_GROUP_FILE_LOCATOR_H_
