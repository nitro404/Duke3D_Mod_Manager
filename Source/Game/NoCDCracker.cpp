#include "NoCDCracker.h"

#include "Game/GameVersion.h"

#include <Utilities/StringUtilities.h>

#include <fmt/core.h>

#include <filesystem>

static constexpr uint32_t PLUTONIUM_PAK_EXECUTABLE_SIZE = 1240151;
static constexpr uint32_t PLUTONIUM_PAK_NO_CD_CRACK_BYTE_INDEX = 553795;
static constexpr uint32_t ATOMIC_EDITION_EXECUTABLE_SIZE = 1246231;
static constexpr uint32_t ATOMIC_EDITION_NO_CD_CRACK_BYTE_INDEX = 556947;
static constexpr uint8_t NO_CD_CRACK_BYTE_VALUE = 42;

NoCDCracker::GameExecutableStatus NoCDCracker::getGameExecutableStatus(const std::string & gameExecutablePath) {
	if(gameExecutablePath.empty() || !std::filesystem::is_regular_file(std::filesystem::path(gameExecutablePath))) {
		return GameExecutableStatus::Missing;
	}

	std::unique_ptr<ByteBuffer> gameExecutableBuffer(ByteBuffer::readFrom(gameExecutablePath, Endianness::LittleEndian));

	return getGameExecutableStatus(gameExecutableBuffer.get());
}

NoCDCracker::GameExecutableStatus NoCDCracker::getGameExecutableStatus(const ByteBuffer * gameExecutableBuffer) {
	GameExecutableStatus gameExecutableStatus = GameExecutableStatus::Missing;

	if(gameExecutableBuffer == nullptr) {
		return gameExecutableStatus;
	}

	gameExecutableStatus |= GameExecutableStatus::Exists;

	std::string gameExecutableSHA1(gameExecutableBuffer->getSHA1());

	if(Utilities::areStringsEqual(gameExecutableSHA1, GameVersion::PLUTONIUM_PAK_GAME_EXECTUABLE_UNCRACKED_SHA1)) {
		gameExecutableStatus |= GameExecutableStatus::PlutoniumPak;
	}
	else if(Utilities::areStringsEqual(gameExecutableSHA1, GameVersion::PLUTONIUM_PAK_GAME_EXECTUABLE_CRACKED_SHA1)) {
		gameExecutableStatus |= GameExecutableStatus::PlutoniumPak | GameExecutableStatus::Cracked;
	}
	else if(Utilities::areStringsEqual(gameExecutableSHA1, GameVersion::ATOMIC_EDITION_GAME_EXECTUABLE_UNCRACKED_SHA1)) {
		gameExecutableStatus |= GameExecutableStatus::AtomicEdition;
	}
	else if(Utilities::areStringsEqual(gameExecutableSHA1, GameVersion::ATOMIC_EDITION_GAME_EXECTUABLE_CRACKED_SHA1)) {
		gameExecutableStatus |= GameExecutableStatus::AtomicEdition | GameExecutableStatus::Cracked;
	}
	else if(Utilities::areStringsEqual(gameExecutableSHA1, GameVersion::REGULAR_VERSION_GAME_EXECUTABLE_SHA1)) {
		gameExecutableStatus |= GameExecutableStatus::RegularVersion;
	}
	else {
		gameExecutableStatus |= GameExecutableStatus::Invalid;
	}

	return gameExecutableStatus;
}

bool NoCDCracker::isRegularVersionGameExecutable(const std::string & gameExecutablePath) {
	return Any(getGameExecutableStatus(gameExecutablePath) & GameExecutableStatus::RegularVersion);
}

bool NoCDCracker::isPlutoniumPakGameExecutable(const std::string & gameExecutablePath) {
	return Any(getGameExecutableStatus(gameExecutablePath) & GameExecutableStatus::PlutoniumPak);
}
bool NoCDCracker::isAtomicEditionGameExecutable(const std::string & gameExecutablePath) {
	return Any(getGameExecutableStatus(gameExecutablePath) & GameExecutableStatus::AtomicEdition);
}

bool NoCDCracker::isGameExecutableCrackable(const std::string & gameExecutablePath) {
	GameExecutableStatus gameExecutableStatus = getGameExecutableStatus(gameExecutablePath);

	return Any(gameExecutableStatus & (GameExecutableStatus::AtomicEdition | GameExecutableStatus::PlutoniumPak)) && None(gameExecutableStatus & GameExecutableStatus::Cracked);
}

bool NoCDCracker::isGameExecutableCracked(const std::string & gameExecutablePath) {
	GameExecutableStatus gameExecutableStatus = getGameExecutableStatus(gameExecutablePath);

	return Any(gameExecutableStatus & (GameExecutableStatus::AtomicEdition | GameExecutableStatus::PlutoniumPak)) && Any(gameExecutableStatus & GameExecutableStatus::Cracked);
}

bool NoCDCracker::crackGameExecutable(const std::string & gameExecutablePath) {
	return crackGameExecutable(gameExecutablePath, gameExecutablePath);
}

bool NoCDCracker::crackGameExecutable(const std::string & inputGameExecutablePath, const std::string & outputGameExecutablePath) {
	if(inputGameExecutablePath.empty() || !std::filesystem::is_regular_file(std::filesystem::path(inputGameExecutablePath))) {
		return false;
	}

	std::unique_ptr<ByteBuffer> gameExecutableBuffer(ByteBuffer::readFrom(inputGameExecutablePath, Endianness::LittleEndian));

	GameExecutableStatus gameExecutableStatus = getGameExecutableStatus(gameExecutableBuffer.get());

	if(Any(gameExecutableStatus & GameExecutableStatus::Cracked)) {
		return false;
	}

	uint32_t noCDCrackByteIndex = 0;

	if(Any(gameExecutableStatus & GameExecutableStatus::AtomicEdition)) {
		if(gameExecutableBuffer->getSize() != ATOMIC_EDITION_EXECUTABLE_SIZE) {
			return false;
		}

		noCDCrackByteIndex = ATOMIC_EDITION_NO_CD_CRACK_BYTE_INDEX;
	}
	else if(Any(gameExecutableStatus & GameExecutableStatus::PlutoniumPak)) {
		if(gameExecutableBuffer->getSize() != PLUTONIUM_PAK_EXECUTABLE_SIZE) {
			return false;
		}

		noCDCrackByteIndex = PLUTONIUM_PAK_NO_CD_CRACK_BYTE_INDEX;
	}
	else {
		return false;
	}

	// crack the game executable data
	gameExecutableBuffer->putByte(NO_CD_CRACK_BYTE_VALUE, noCDCrackByteIndex);

	return gameExecutableBuffer->writeTo(outputGameExecutablePath, true);
}
