#ifndef PC_SPEAKER_H
#define PC_SPEAKER_H

#include <stdint.h>
#include <timer.h>

void initializeSpeaker();
void playSoundForCertainMs(uint32_t nFrequence, uint32_t ms);

#endif
