#include "AddonGroupFileGenerator.h"

#include "GameAddon.h"
#include "GameAddonDiscImageDownloader.h"
#include "Game/File/GameFileFactoryRegistry.h"

#include <ByteBuffer.h>
#include <Utilities/CDIOUtilities.h>
#include <Utilities/FileUtilities.h>

#include <spdlog/spdlog.h>

#include <string>

AddonGroupFileGenerator::AddonGroupFileGenerator() { }

std::unique_ptr<Group> AddonGroupFileGenerator::generateDukeCaribbeanLifesABeachGroup() {
	std::pair<std::unique_ptr<ISO9660::FS>, std::string> dukeCaribbeanLifesABeachDiscImage(GameAddonDiscImageDownloader::downloadGameAddonDiscImage(GameAddon::DukeCaribbeanLifesABeach));

	if(dukeCaribbeanLifesABeachDiscImage.first == nullptr) {
		// TODO: error
		return nullptr;
	}

	std::array<std::string, 3> groupFileNames({ "vaca13.ssi", "vaca15.ssi", "vacapp.ssi" });

	for(const std::string & groupFileName : groupFileNames) {
		std::unique_ptr<ByteBuffer> data(CDIOUtilities::readFile(*dukeCaribbeanLifesABeachDiscImage.first, "/vacation/" + groupFileName));

		if(data == nullptr) {
			// TODO: error
			return nullptr;
		}

		//data->writeTo(fileName);

		std::shared_ptr<Group> group(std::dynamic_pointer_cast<Group>(std::shared_ptr<GameFile>(GameFileFactoryRegistry::getInstance()->readGameFileFrom(*data, "SSI"))));

		if(group == nullptr) {
			// TODO: improve error:
			spdlog::error("Failed to read group.");
			return nullptr;
		}

		group->reverseFileExtensions({ "MNA", "TRA", "NOC", "TAD", "OMD", "PRG", "PAM", "DIM", "COV", "VAW" });
		group->renameFile("GAME.CON", "VACATION.CON");
		group->renameFile("DEFS.CON", "VACATION.DEF");
		group->renameFile("USER.CON", "VACATION.USR");

		std::shared_ptr<GroupFile> gameConGroupFile(group->getFileWithName("VACATION.CON"));
		ByteBuffer & gameConGroupFileData = gameConGroupFile->getData();
		gameConGroupFileData.replaceInstanceOfStringFromStart("USER.CON", "VACATION.USR", true, 1, 0);
		gameConGroupFileData.replaceInstanceOfStringFromStart("DEFS.CON", "VACATION.DEF", true, 1, 0);

		// TODO: except GRP extension:

		group->extractAllFiles(std::string(Utilities::getFileNameNoExtension(groupFileName)), true);

		std::shared_ptr<GroupFile> vacationGroupFile(group->getFileWithName("VACATION.GRP"));

		if(vacationGroupFile != nullptr) {
			std::shared_ptr<Group> vacationGroup(std::dynamic_pointer_cast<Group>(std::shared_ptr<GameFile>(GameFileFactoryRegistry::getInstance()->readGameFileFrom(vacationGroupFile->getData(), "GRP"))));

			// TODO: except: KGROUP.EXE

			vacationGroup->extractAllFiles(Utilities::joinPaths(Utilities::getFileNameNoExtension(groupFileName), "grp"), true);
		}
	}

	// TODO: fixme:
}

std::unique_ptr<Group> AddonGroupFileGenerator::generateDukeItOutInDCGroup() {
	std::pair<std::unique_ptr<ISO9660::FS>, std::string> dukeItOutInDCDiscImage(GameAddonDiscImageDownloader::downloadGameAddonDiscImage(GameAddon::DukeItOutInDC));

	if(dukeItOutInDCDiscImage.first == nullptr) {
		// TODO: error
		return nullptr;
	}

	std::array<std::string, 2> groupFileNames({ "dukedc13.ssi", "dukedcpp.ssi" });

	for(const std::string & groupFileName : groupFileNames) {
		std::unique_ptr<ByteBuffer> data(CDIOUtilities::readFile(*dukeItOutInDCDiscImage.first, "/dukedc/" + groupFileName));

		if(data == nullptr) {
			// TODO: error
			return nullptr;
		}

		std::shared_ptr<Group> group(std::dynamic_pointer_cast<Group>(std::shared_ptr<GameFile>(GameFileFactoryRegistry::getInstance()->readGameFileFrom(*data, "SSI"))));

		if(group == nullptr) {
			// TODO: improve error:
			spdlog::error("Failed to read group.");
			return nullptr;
		}

		group->renameFile("USER.CON", "DUKEDC.USR");

		// TODO: need GAME.CON file!:
		/*
		std::shared_ptr<GroupFile> gameConGroupFile(group->getFileWithName("DUKEDC.CON"));
		ByteBuffer & gameConGroupFileData = gameConGroupFile->getData();
		gameConGroupFileData.replaceInstanceOfStringFromStart("USER.CON", "DUKEDC.USR", true, 1, 0);
		*/

		group->extractAllFiles(std::string(Utilities::getFileNameNoExtension(groupFileName)), true);
	}

	// TODO: fixme:
	return nullptr;
}

std::unique_ptr<Group> AddonGroupFileGenerator::generateNuclearWinterGroup() {
	std::pair<std::unique_ptr<ISO9660::FS>, std::string> nuclearWinterDiscImage(GameAddonDiscImageDownloader::downloadGameAddonDiscImage(GameAddon::NuclearWinter));

	if(nuclearWinterDiscImage.first == nullptr) {
		// TODO: error
		return nullptr;
	}

	std::unique_ptr<ByteBuffer> data(CDIOUtilities::readFile(*nuclearWinterDiscImage.first, "/gamedata/nwinter.grp"));

	if(data == nullptr) {
		spdlog::error("Failed to read '{}' group file data from game addon disc image.", getGameAddonName(GameAddon::NuclearWinter));
		return nullptr;
	}

	std::unique_ptr<GameFile> gameFile(GameFileFactoryRegistry::getInstance()->readGameFileFrom(*data, "GRP"));

	if(gameFile == nullptr) {
		spdlog::error("Failed to parse '{}' game addon group file data.", getGameAddonName(GameAddon::NuclearWinter));
		return nullptr;
	}

	if(dynamic_cast<Group *>(gameFile.get()) == nullptr) {
		spdlog::error("Invalid '{}' game addon game file type, expected group.");
		return nullptr;
	}

	return std::unique_ptr<Group>(static_cast<Group *>(gameFile.release()));
}
