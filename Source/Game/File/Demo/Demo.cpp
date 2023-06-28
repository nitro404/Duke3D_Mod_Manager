#include "Demo.h"

#include <spdlog/spdlog.h>

Demo::Demo(const std::string & filePath)
	: GameFile(filePath) { }

Demo::Demo(Demo && demo) noexcept
	: GameFile(demo) { }

Demo::Demo(const Demo & demo)
	: GameFile(demo) { }

Demo & Demo::operator = (Demo && demo) noexcept {
	if(this != &demo) {
		GameFile::operator = (demo);
	}

	return *this;
}

Demo & Demo::operator = (const Demo & demo) {
	GameFile::operator = (demo);

	return *this;
}

Demo::~Demo() { }

std::unique_ptr<Demo> Demo::readFrom(const ByteBuffer & byteBuffer) {
	byteBuffer.setEndianness(ENDIANNESS);

	bool error = false;

uint32_t version = byteBuffer.readUnsignedInteger(&error);

//kdfread(void *buffer, size_t dasizeof, size_t count, long fil)
//kdfread(&bv,4,1,fil);

spdlog::info("version: {}", version);

// 1.3D:
// 1 - Dark Side (E2L8)
// 2 - Flood Zone (E3L3)
//*
	uint32_t totalNumberOfGameTicks = byteBuffer.readUnsignedInteger(&error);
	uint8_t volume = byteBuffer.readUnsignedByte(&error) + 1;
	uint8_t level = byteBuffer.readUnsignedByte(&error) + 1;
	uint8_t skill = byteBuffer.readUnsignedByte(&error);
	uint8_t multiplayerMode = byteBuffer.readUnsignedByte(&error);
	uint16_t playerNumber = byteBuffer.readUnsignedShort(&error);
	uint16_t noMonsters = byteBuffer.readUnsignedShort(&error);
	uint32_t respawnMonsters = byteBuffer.readUnsignedInteger(&error);
	uint32_t respawnItems = byteBuffer.readUnsignedInteger(&error);
	uint32_t respawnInventory = byteBuffer.readUnsignedInteger(&error);

spdlog::info("E{}L{} Skill: {} MP: {} Tics: {}", volume, level, skill, multiplayerMode, totalNumberOfGameTicks);
//*/

// ATOMIC:
// 1 - Freeway (E1L11)
// 2 - Area 51 (E4L11)
// 2 - Duke Burger (E4L2)

// TODO: eDuke32 demo is different!

/*
address type contents
0x0000 long number of game tics times number of players
0x0004 byte volume - 1 (/v parameter - 1)
0x0005 byte level - 1 (/l paramerer - 1)
0x0006 byte skill (0 .. 4) (/s parameter)
0x0007 byte MP mode (/c 1 = DukeMatch(spawn), 2 = Coop, 3 = Dukematch(no spawn))
0x0008 short player number (1..8)
0x000A short 0x01 with /m (nomonsters), 0x00 else
0x000C long 0x01 with /t1 (respawn monsters), 0x00 else
0x0010 long 0x01 with /t2 (respawn items), 0x00 else
0x0014 long 0x01 with /t3 (respawn inventory), 0x00 else 
*/

// TODO:
return nullptr;
}

std::unique_ptr<Demo> Demo::loadFrom(const std::string & filePath) {
// TODO:
return nullptr;
}

bool Demo::writeTo(ByteBuffer & byteBuffer) const {
	byteBuffer.setEndianness(ENDIANNESS);

// TODO:
return false;
}

void Demo::addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const {
	if(!isValid()) {
		return;
	}

// TODO:
}

Endianness Demo::getEndianness() const {
	return ENDIANNESS;
}

size_t Demo::getSizeInBytes() const {
// TODO:
return 0;
}

bool Demo::isValid(bool verifyParent) const {
// TODO:
return true;
}

bool Demo::isValid(const Demo * demo, bool verifyParent) {
	return demo != nullptr &&
		   demo->isValid(verifyParent);
}

bool Demo::operator == (const Demo & demo) const {
// TODO:
return this == &demo;
}

bool Demo::operator != (const Demo & demo) const {
	return !operator == (demo);
}
