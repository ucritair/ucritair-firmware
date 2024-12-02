#ifndef _DG_SOUND_H_
#define _DG_SOUND_H_

#include <stdbool.h>
#include <stdint.h>

enum SoundPlayMode {
	SoundReplaceCurrent,
	SoundWaitForCurrent,
	SoundDoNothingIfPlaying,
};


void soundPower(bool on);

void soundStopAll(void);
bool soundIsPlaying(void);
void soundPlay(const uint8_t *data, uint32_t len, enum SoundPlayMode playbackMode);

void test_speaker();

#endif
