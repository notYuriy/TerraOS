#ifndef __TIMER_H_INCLUDED__
#define __TIMER_H_INCLUDED__

#include <utils.h>
#include <idt.h>

typedef void (*timer_callback_t)(uint64_t ticks_count);
void timer_init(uint32_t frequency);
void timer_enable(void);
void timer_disable(void);
uint64_t timer_ticks_count(void);
void timer_set_callback(timer_callback_t callback);
uint32_t timer_get_frequency(void);

#endif