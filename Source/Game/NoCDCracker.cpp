#include "NoCDCracker.h"

#include <Utilities/StringUtilities.h>

#include <fmt/core.h>

#include <filesystem>

static const std::string REGULAR_VERSION_GAME_EXECUTABLE_SHA1("a64cc5b61cba728427cfcc537aa2f74438ea4c65");
static const std::string ATOMIC_EDITION_GAME_EXECTUABLE_UNCRACKED_SHA1("f0dc7f1ca810aa517fcad544a3bf5af623a3e44e");
static const std::string ATOMIC_EDITION_GAME_EXECTUABLE_CRACKED_SHA1("a849e1e00ac58c0271498dd302d5c5f2819ab2e9");
static constexpr uint32_t ATOMIC_EDITION_EXECUTABLE_SIZE = 1246231;
static constexpr uint32_t NO_CD_CRACK_BYTE_INDEX = 556947;
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

	if(Utilities::areStringsEqual(gameExecutableSHA1, ATOMIC_EDITION_GAME_EXECTUABLE_UNCRACKED_SHA1)) {
		gameExecutableStatus |= GameExecutableStatus::AtomicEdition;
	}
	else if(Utilities::areStringsEqual(gameExecutableSHA1, ATOMIC_EDITION_GAME_EXECTUABLE_CRACKED_SHA1)) {
		gameExecutableStatus |= GameExecutableStatus::AtomicEdition | GameExecutableStatus::Cracked;
	}
	else if(Utilities::areStringsEqual(gameExecutableSHA1, REGULAR_VERSION_GAME_EXECUTABLE_SHA1)) {
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

bool NoCDCracker::isAtomicEditionGameExecutable(const std::string & gameExecutablePath) {
	return Any(getGameExecutableStatus(gameExecutablePath) & GameExecutableStatus::AtomicEdition);
}

bool NoCDCracker::isGameExecutableCrackable(const std::string & gameExecutablePath) {
	GameExecutableStatus gameExecutableStatus = getGameExecutableStatus(gameExecutablePath);

	return Any(gameExecutableStatus & GameExecutableStatus::AtomicEdition) && None(gameExecutableStatus & GameExecutableStatus::Cracked);
}

bool NoCDCracker::isGameExecutableCracked(const std::string & gameExecutablePath) {
	GameExecutableStatus gameExecutableStatus = getGameExecutableStatus(gameExecutablePath);

	return Any(gameExecutableStatus & GameExecutableStatus::AtomicEdition) && Any(gameExecutableStatus & GameExecutableStatus::Cracked);
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

	if(None(gameExecutableStatus & GameExecutableStatus::AtomicEdition) || Any(gameExecutableStatus & GameExecutableStatus::Cracked) ) {
		return false;
	}

	if(gameExecutableBuffer->getSize() != ATOMIC_EDITION_EXECUTABLE_SIZE) {
		return false;
	}

	// crack the game executable data
	gameExecutableBuffer->putByte(NO_CD_CRACK_BYTE_VALUE, NO_CD_CRACK_BYTE_INDEX);

	return gameExecutableBuffer->writeTo(outputGameExecutablePath, true);
}
