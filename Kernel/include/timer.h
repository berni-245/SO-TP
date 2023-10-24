#ifndef TIMER_H
#define TIMER_H

void timer_tick();
unsigned long get_ticks();
unsigned long get_ms();
void sleep(unsigned long ms);

#endif
