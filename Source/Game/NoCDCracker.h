#ifndef _NO_CD_CRACKER_H_
#define _NO_CD_CRACKER_H_

#include <BitmaskOperators.h>
#include <ByteBuffer.h>

#include <string>

class NoCDCracker final {
public:
	enum class GameExecutableStatus
	{
		Missing = 0,
		Exists = 1,
		Invalid = 1 << 1,
		RegularVersion = 1 << 2,
		AtomicEdition = 1 << 3,
		Cracked = 1 << 4
	};

	static GameExecutableStatus getGameExecutableStatus(const std::string & gameExecutablePath);
	static GameExecutableStatus getGameExecutableStatus(const ByteBuffer * gameExecutableBuffer);
	static bool isRegularVersionGameExecutable(const std::string & gameExecutablePath);
	static bool isAtomicEditionGameExecutable(const std::string & gameExecutablePath);
	static bool isGameExecutableCrackable(const std::string & gameExecutablePath);
	static bool isGameExecutableCracked(const std::string & gameExecutablePath);
	static bool crackGameExecutable(const std::string & gameExecutablePath);
	static bool crackGameExecutable(const std::string & inputGameExecutablePath, const std::string & outputGameExecutablePath);

};

template<>
struct BitmaskOperators<NoCDCracker::GameExecutableStatus> {
	static const bool enabled = true;
};

#endif // _NO_CD_CRACKER_H_
