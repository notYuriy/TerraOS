#ifndef __TIMER_H_INCLUDED__
#define __TIMER_H_INCLUDED__

#include <utils.h>

void timer_init(uint32_t frequency);
uint64_t timer_ticks_count();

#endif