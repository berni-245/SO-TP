#ifndef TIMER_H
#define TIMER_H

void incTicks();
uint64_t getTicks();
uint64_t getMs();
void sleep(uint64_t ms);

#endif
