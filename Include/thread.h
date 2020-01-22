#ifndef __THREAD_H_INCLUDED__
#define __THREAD_H_INCLUDED__

#include <utils.h>

typedef void (*thread_entry_point_t)();

void thread_init_subsystem();
void thread_sleep(size_t ms);
void thread_yield(void);
void thread_summon(thread_entry_point_t main, size_t stack_size);

#endif