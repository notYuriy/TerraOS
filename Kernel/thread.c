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
_Atomic uint64_t preempt;

typedef struct task_state_struct task_state_t;

typedef struct task_state_struct {
    ext_regs_t regs;
    idt_stack_frame_t frame;
    task_state_t *next;
    uint64_t id;
    bool deleted;
    char* stack_base;
} __attribute__((packed)) task_state_t;

task_state_t* current;
task_state_t* prev;
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
    task_state_t* new_task = kastub_new(&tasks_stub);
    preempt = 0;
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
    new_task->next = current->next;
    new_task->frame.rip = (uint64_t)thread_call_stub;
    new_task->frame.rdi = (uint64_t)main;
    new_task->frame.cs = 8;
    new_task->frame.ss = 16;
    new_task->deleted = false;
    //we assume, that this op is atomic here
    current->next = new_task;
    preempt = 1;
}

void thread_exit(void){
    current->deleted = true;
    //after this thread can not do any useful work
    while(1) asm("pause":::); //waiting for the next schedule
}

void thread_yield_using_frame(idt_stack_frame_t* frame){
    task_state_t* next = current->next;
    if(current->deleted){
        kheap_free(current->stack_base);
        kastub_delete(&tasks_stub, current);
        prev->next = next;
        current = next;
        thread_copy_state_to_frame(frame);
    }else{
        thread_copy_frame_to_state(frame);
        prev = current;
        current = next;
        thread_copy_state_to_frame(frame);
    }    
}

void thread_scheduler_isr(
    uint64_t __attribute__((unused)) ticks_count, 
    idt_stack_frame_t* frame){
    if(preempt != 0) thread_yield_using_frame(frame);
}

void thread_init_subsystem(void){
    //alignment requirement for extended save
    preempt = 1;
    kastub_init(&tasks_stub, sizeof(task_state_t), 64);
    timer_set_callback(thread_scheduler_isr);
    current = kastub_new(&tasks_stub);
    memset(current->regs, sizeof(current->regs), 0);
    memset((void*)&(current->frame), sizeof(current->frame), 0);
    current->next = current;
    current->deleted = false;
    prev = current;
    //preallocate 10000 threads descriptors
    for(size_t i = 0; i < 10000; ++i){
        kastub_delete(&tasks_stub, kastub_new(&tasks_stub));
    }
}
