#ifndef _BUILD_CONSTANTS_H_
#define _BUILD_CONSTANTS_H_

namespace BuildConstants {

	constexpr int16_t MAX_NUMBER_OF_SECTORS = 1024;
	constexpr int16_t MAX_NUMBER_OF_WALLS = 8192;
	constexpr int16_t MAX_NUMBER_OF_SPRITES = 4096;
	constexpr int16_t MAX_NUMBER_OF_SOUNDS = 450;
	constexpr int16_t MIN_ANGLE = 0;
	constexpr int16_t MAX_ANGLE = 2047;
	constexpr uint16_t ATOMIC_EDITION_START_TILE_NUMBER = 4096;
	constexpr uint16_t MIRROR_TILE_NUMBER = 560;
	constexpr uint16_t ECHO_EFFECT_OFFSET = 1000;
	constexpr uint16_t MAX_ECHO_EFFECT = 255;
	constexpr uint16_t ONE_TIME_SECTOR_SOUND_LOW_TAG = 10000;

}

#endif // _BUILD_CONSTANTS_H_
