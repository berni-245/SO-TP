#include <pcSpeaker.h>

void playSoundForCertainMs(uint32_t nFrequence, int ms){
    // playSound(nFrequence);
    // sleep(ms);
    // noSound();
    // sleep(56); // Si no hago esto, o pongo un valor menor, no puedo llamar a la función n (n >= 2) veces seguidas, 
			   // me parece que es porque le tengo que dar un enfriamiento al hardware para que
			   // se pause completamente el primer sonido, no creo que sea casualidad que es aprox 1 tick
}
