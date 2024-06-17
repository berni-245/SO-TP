#include <array.h>
#include <pcSpeaker.h>
#include <scheduler.h>
#include <semaphores.h>
#include <timer.h>

typedef struct {
  uint32_t freq;
  uint32_t ms;
} Sound;

sem_t sem;
Array sounds;

extern void setSpeakerFreq(uint32_t nFrequence);
extern void speakerOff();
void speakerProcess();

void initializeSpeaker() {
  sem = semInit(0);
  sounds = Array_initialize(sizeof(Sound), 500, NULL);
  const char* argv[] = {"speaker"};
  createUserProcess(1, argv, speakerProcess);
}

void playSoundForCertainMs(uint32_t nFrequence, uint32_t ms) {
  if (ms <= 0 || nFrequence == 0) return;
  Sound sound = {.freq = nFrequence, .ms = ms};
  Array_push(sounds, &sound);
  postSemaphore(sem);
}

void speakerProcess() {
  int i = 0;
  while (1) {
    waitSemaphore(sem);
    Sound* sound = Array_get(sounds, i++);
    setSpeakerFreq(sound->freq);
    sleep(sound->ms);
    speakerOff();
    if (i == Array_getLen(sounds)) Array_clear(sounds);
  }
}
