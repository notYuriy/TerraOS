#ifndef __THREAD_H_INCLUDED__
#define __THREAD_H_INCLUDED__

#include <utils.h>
#include <fstate.h>
#include <idt.h>

typedef struct kthread_struct kthread_t;

typedef struct kthread_struct {
    ext_regs_t regs;
    idt_stack_frame_t frame;
    kthread_t *next;
    kthread_t *prev;
    uint64_t id;
    char* stack_base;
} kthread_t;

typedef void (*kthread_entry_point_t)();

//All functions assume safe use
//So thread shouldn't be deallocated

void kthread_init_subsystem(void);
void kthread_sleep(size_t ms);
void kthread_yield(void);
kthread_t* kthread_summon(kthread_entry_point_t main, size_t stack_size);
kthread_t* kthread_summon_preallocated(kthread_t* thread);
void kthread_exit(void);
bool kthread_is_initialized();
uint64_t kthread_get_id();
void kthread_kill_thread(kthread_t* thread);

#endif