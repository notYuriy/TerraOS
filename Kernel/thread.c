#include <thread.h>
#include <timer.h>
#include <idt.h>
#include <fstate.h>
#include <spinlock.h>
#include <kslub.h>
#include <vmcore.h>
#include <stdatomic.h>

void thread_call_stub(thread_entry_point_t entry);

uint64_t asmutils_get_p4_table(void);
uint64_t asmutils_get_rflags(void);
_Atomic uint64_t process_count;
_Atomic uint64_t thread_preemptable;
spinlock_t yield_spinlock;

typedef struct task_state_struct task_state_t;

typedef struct task_state_struct {
    ext_regs_t regs;
    idt_stack_frame_t frame;
    task_state_t *next;
    uint64_t id;
    bool deleted;
    char* stack_base;
} __attribute__((packed)) task_state_t;

task_state_t* thread_current;
task_state_t* thread_prev;
kastub_t tasks_stub;

void thread_copy_frame_to_state(idt_stack_frame_t* frame){
    memcpy(&(thread_current->frame), frame, sizeof(*frame));
    asmutils_store_ext_regs(thread_current->regs);
}

void thread_copy_state_to_frame(idt_stack_frame_t* frame){
    memcpy(frame, &(thread_current->frame), sizeof(*frame));
    asmutils_load_ext_regs(thread_current->regs);
}

void thread_summon(thread_entry_point_t main, size_t stack_size){
    task_state_t* new_task = kastub_new(&tasks_stub);
    thread_preemptable = 0;
    new_task->id = process_count;
    memset(new_task->regs, sizeof(new_task->regs), 0);
    memset((void*)&(new_task->frame), sizeof(new_task->frame), 0);
    size_t stack_pages_size = ((stack_size + 4095)/4096) + 1;
    char* stack_base = (char*)kheap_malloc_aligned(4096 * stack_pages_size, 4096);
    char* stack_top = stack_base + (stack_pages_size * 4096);
    new_task->stack_base = stack_base;
    new_task->frame.rsp = (uint64_t)stack_top;
    new_task->frame.rflags = asmutils_get_rflags();
    new_task->frame.cr3 = asmutils_get_p4_table();
    new_task->next = thread_current->next;
    new_task->frame.rip = (uint64_t)thread_call_stub;
    new_task->frame.rdi = (uint64_t)main;
    new_task->frame.cs = 8;
    new_task->frame.ss = 16;
    new_task->deleted = false;
    //we assume, that this op is atomic here
    thread_current->next = new_task;
    thread_preemptable = 1;
}

void thread_exit(void){
    thread_current->deleted = true;
    //after this thread can not do any useful work
    while(1) asm("pause":::);
}

void thread_yield_using_frame(idt_stack_frame_t* frame){
    if(spinlock_trylock(&yield_spinlock) && (thread_preemptable == 1)){
        task_state_t* next = thread_current->next;
        if(thread_current->deleted){
            kheap_free(thread_current->stack_base);
            kastub_delete(&tasks_stub, thread_current);
            thread_prev->next = next;
            thread_current = next;
            thread_copy_state_to_frame(frame);
        }else{
            thread_copy_frame_to_state(frame);
            thread_prev = thread_current;
            thread_current = next;
            thread_copy_state_to_frame(frame);
        }
        spinlock_unlock(&yield_spinlock);
    }
}

void thread_scheduler_isr(
    uint64_t __attribute__((unused)) ticks_count, 
    idt_stack_frame_t* frame){
    thread_yield_using_frame(frame);
}

void thread_init_subsystem(void){
    //alignment requirement for extended save
    kastub_init(&tasks_stub, sizeof(task_state_t), 64);
    timer_set_callback(thread_scheduler_isr);
    thread_current = kastub_new(&tasks_stub);
    memset(thread_current->regs, sizeof(thread_current->regs), 0);
    memset((void*)&(thread_current->frame), sizeof(thread_current->frame), 0);
    thread_current->next = thread_current;
    thread_current->deleted = false;
    thread_prev = thread_current;
    yield_spinlock = 0;
    thread_preemptable = 1;
}
