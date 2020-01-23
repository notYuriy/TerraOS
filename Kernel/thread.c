#include <thread.h>
#include <timer.h>
#include <idt.h>
#include <fstate.h>
#include <spinlock.h>
#include <kstub.h>
#include <vmcore.h>
#include <stdatomic.h>

uint64_t asmutils_get_p4_table(void);
uint64_t asmutils_get_rflags(void);
_Atomic uint64_t  process_count;

typedef struct task_state_struct task_state_t;

typedef struct task_state_struct {
    ext_regs_t regs;
    idt_stack_frame_t frame;
    task_state_t *next;
    bool deleted;
    char* stack_base;
} __attribute__((packed)) task_state_t;

task_state_t* current;
spinlock_t yield_lock;
kastub_t tasks_stub;

void thread_copy_frame_to_state(idt_stack_frame_t* frame){
    memcpy(&(current->frame), frame, sizeof(*frame));
    asmutils_store_ext_regs(current->regs);
}

void thread_copy_state_to_frame(idt_stack_frame_t* frame){
    memcpy(frame, &(current->frame), sizeof(*frame));
    asmutils_load_ext_regs(current->regs);
}


void thread_summon(thread_entry_point_t main, size_t stack_size){
    process_count++;
    printf("process count: %llu\n", process_count);
    task_state_t* new_task = kastub_new(&tasks_stub);
    memset(new_task->regs, sizeof(new_task->regs), 0);
    memset((void*)&(new_task->frame), sizeof(new_task->frame), 0);
    size_t stack_pages_size = ((stack_size + 4095)/4096) + 1;
    char* stack_base = (char*)vmcore_kmmap(0, 0, stack_pages_size);
    char* stack_top = stack_base + (stack_pages_size * 4096);
    new_task->stack_base = stack_base;
    new_task->frame.rsp = (uint64_t)stack_top;
    new_task->frame.rflags = asmutils_get_rflags();
    new_task->frame.cr3 = asmutils_get_p4_table();
    new_task->next = current->next;
    new_task->frame.rip = (uint64_t)main;
    new_task->frame.cs = 8;
    new_task->frame.ss = 16;
    //we assume, that this op is atomic here
    current->next = new_task;
}

void thread_yield_using_frame(idt_stack_frame_t* frame){
    uint64_t result = spinlock_trylock(&yield_lock);
    if(result != 0){
        //printf("current: %p\n", current);
        task_state_t* next = current->next;
        thread_copy_frame_to_state(frame);
        current = next;
        thread_copy_state_to_frame(frame);
        spinlock_unlock(&yield_lock);
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
    yield_lock = 0;
    current = kastub_new(&tasks_stub);
    memset(current->regs, sizeof(current->regs), 0);
    memset((void*)&(current->frame), sizeof(current->frame), 0);
    current->next = current;
}
