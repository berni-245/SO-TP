#include <pcSpeaker.h>

#define baseFrequence 1193180

// este código está sacado de wiki, luego lo mejoraría
// por ejemplo una función "playSound for x secs"

void playSound(uint32_t nFrequence){
	uint32_t div;
	uint8_t tmp;

	div = baseFrequence / nFrequence;
	outb(0x43, 0xb6);
	outb(0x42, (uint8_t) (div));
	outb(0x42, (uint8_t) (div >> 8));

	tmp = inb(0x61);
	if (tmp != (tmp | 3)){
		outb(0x61, tmp | 3);
	}
}

void noSound(){
	uint8_t tmp = inb(0x61) & 0xFC;

	outb(0x61, tmp);
}

void playSoundForCertainTime(uint32_t nFrequence, Time time){
    playSound(nFrequence);
    sleepWithTime(time);
    noSound();
}

void playSoundForCertainSeconds(uint32_t nFrequence, int seconds){
    playSound(nFrequence);
    sleep(seconds);
    noSound();
}