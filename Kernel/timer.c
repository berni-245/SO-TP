#include <timer.h>

static double freq = 65536 / 3600.0; // interruptions/second
static unsigned long ticks = 0;
void timer_tick() { ticks++; }
unsigned long get_ticks() { return ticks; }
unsigned long get_ms() { return ticks * 1000 / freq; }
void sleep(unsigned long ms){
    unsigned long end = ms + get_ms();
    while(get_ms() < end);
}
