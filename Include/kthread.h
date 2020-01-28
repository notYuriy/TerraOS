#ifndef __THREAD_H_INCLUDED__
#define __THREAD_H_INCLUDED__

#include <utils.h>

typedef void (*kthread_entry_point_t)();

//add one task continuing normal execution flow
void kthread_init_subsystem(void);
void kthread_sleep(size_t ms);
void kthread_yield(void);
void kthread_summon(kthread_entry_point_t main, size_t stack_size);
void kthread_exit(void);

#endif