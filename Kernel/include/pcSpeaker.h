#ifndef PC_SPEAKER_H
#define PC_SPEAKER_H

#include <stdint.h>
#include <clock.h>

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t code);
void playSound(uint32_t nFrequence);
void playSoundForCertainTime(uint32_t nFrequence, Time time);
void playSoundForCertainSeconds(uint32_t nFrequence, int seconds);
void noSound();

#endif