#ifndef _GROUP_UTILITIES_H_
#define _GROUP_UTILITIES_H_

#include "Group.h"
#include "GRP/GroupGRP.h"
#include "SSI/GroupSSI.h"

#include <Utilities/FileUtilities.h>

#include <memory>

namespace GroupUtilities {

	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	std::unique_ptr<Group> combineGroups(Arguments &&... arguments);
	template <typename ...Arguments, typename = typename std::enable_if<sizeof...(Arguments) >= 2>::type>
	std::unique_ptr<Group> combineGroupsFromPaths(Arguments &&... arguments);

}

template <typename ...Arguments, typename>
std::unique_ptr<Group> GroupUtilities::combineGroups(Arguments &&... arguments) {
	const Group * groups[sizeof...(arguments)] = {arguments...};

	if(groups[0] == nullptr) {
		return nullptr;
	}

	std::unique_ptr<Group> combinedGroup(std::make_unique<Group>(*groups[0]));

	for(size_t i = 1; i < sizeof...(arguments); i++) {
		const Group * currentGroup = groups[i];

		if(currentGroup == nullptr) {
			return nullptr;
		}

		for(size_t j = 0; j < currentGroup->numberOfFiles(); j++) {
			combinedGroup->addFile(*currentGroup->getFile(j), true);
		}
	}

	return combinedGroup;
}

template <typename ...Arguments, typename>
std::unique_ptr<Group> GroupUtilities::combineGroupsFromPaths(Arguments &&... arguments) {
	std::string_view groupPaths[sizeof...(arguments)] = {arguments...};

	std::unique_ptr<Group> combinedGroup;
	std::unique_ptr<Group> currentGroup;

	for(size_t i = 0; i < sizeof...(arguments); i++) {
		std::string_view groupPath(groupPaths[i]);
		std::string_view fileExtension(Utilities::getFileExtension(groupPath));

		if(Utilities::areStringsEqualIgnoreCase(fileExtension, "SSI")) {
			currentGroup = GroupSSI::loadFrom(groupPath);
		}
		else {
			currentGroup = GroupGRP::loadFrom(groupPath);
		}

		if(combinedGroup == nullptr) {
			combinedGroup = std::move(currentGroup);
			combinedGroup->clearFilePath();
			continue;
		}

		for(size_t j = 0; j < currentGroup->numberOfFiles(); j++) {
			combinedGroup->addFile(*currentGroup->getFile(j), true);
		}

		currentGroup.reset();
	}

	return combinedGroup;
}

#endif // _GROUP_UTILITIES_H_
