#include <kthread.h>
#include <timer.h>
#include <idt.h>
#include <spinlock.h>
#include <kslub.h>
#include <vmcore.h>
#include <stdatomic.h>
#include <portio.h>

bool kthread_initialized_subsystem = false;

//defined in asmutils
void kthread_call_stub(kthread_entry_point_t entry);
extern uint64_t kthread_yield_end;
uint64_t asmutils_get_cr0(void);
uint64_t asmutils_get_cr2(void);
uint64_t asmutils_get_cr4(void);
uint64_t asmutils_get_p4_table(void);
uint64_t asmutils_get_rflags(void);
_Atomic uint64_t process_count;
spinlock_t yield_spinlock;

kthread_t* kthread_current;

kaslub_t tasks_stub;
kslub_t msg_stub;
kthread_t stub;

void kthread_copy_frame_to_state(idt_stack_frame_t* frame){
    memcpy(&(kthread_current->frame), frame, sizeof(*frame));
    //asmutils_store_ext_regs(kthread_current->regs);
}

void kthread_copy_state_to_frame(idt_stack_frame_t* frame){
    memcpy(frame, &(kthread_current->frame), sizeof(*frame));
    //asmutils_load_ext_regs(kthread_current->regs);
}

kthread_t* kthread_summon(kthread_entry_point_t main, size_t stack_size){
    kthread_t* new_task = kaslub_new(&tasks_stub);
    size_t stack_pages_size = ((stack_size + 4095)/4096) + 1;
    char* stack_base = (char*)kheap_malloc_aligned(4096 * stack_pages_size, 4096);
    char* stack_top = stack_base + (stack_pages_size * 4096);
    new_task->id = process_count;
    memset(new_task->regs, sizeof(new_task->regs), 0);
    memset((void*)&(new_task->frame), sizeof(new_task->frame), 0);
    new_task->stack_base = stack_base;
    new_task->frame.rsp = (uint64_t)stack_top;
    new_task->frame.rflags = asmutils_get_rflags();
    new_task->frame.cr3 = asmutils_get_p4_table();
    new_task->frame.cr0 = asmutils_get_cr0();
    new_task->frame.cr2 = asmutils_get_cr2();
    new_task->frame.cr4 = asmutils_get_cr4();
    new_task->frame.rip = (uint64_t)kthread_call_stub;
    new_task->frame.rdi = (uint64_t)main;
    new_task->frame.cs = 0x08;
    new_task->frame.es = new_task->frame.ds =
    new_task->frame.ss = new_task->frame.fs =
    new_task->frame.gs = 0x10;
    spinlock_lock(&yield_spinlock);
    new_task->next = kthread_current;
    kthread_current->prev->next = new_task;
    new_task->prev = kthread_current->prev;
    new_task->next->prev = new_task;
    spinlock_unlock(&yield_spinlock);
    kthread_yield();
    return new_task;
}

void kthread_kill_thread(kthread_t* thread){
    spinlock_lock(&yield_spinlock);
    kthread_t* next = thread->next;
    kthread_t* prev = thread->prev;
    prev->next = next;
    next->prev = prev;
    thread->id = 0;
    kheap_free(thread->stack_base);
    kaslub_delete(&tasks_stub, thread);
    spinlock_unlock(&yield_spinlock);
    kthread_yield();
}

void kthread_exit(void){
    kthread_kill_thread(kthread_current);
}

bool rip_in_yield_method(uint64_t rip){
    return rip >= (uint64_t)(kthread_yield) && rip < (uint64_t)&kthread_yield_end;
}

uint64_t kthread_trylock(){
    return spinlock_trylock(&yield_spinlock);
}

void kthread_lock(){
    spinlock_unlock(&yield_spinlock);
}

void kthread_yield_using_frame(idt_stack_frame_t* frame){
    if(rip_in_yield_method(frame->rip)){
        return;
    }
    //if(frame->rip >= (uint64_t)kthread_yield_using_frame &&
    //   frame->rip < (uint64_t)){
    //
    //}
    if(spinlock_trylock(&yield_spinlock)){
        kthread_t* next = kthread_current->next;
        kthread_copy_frame_to_state(frame);
        kthread_current = next;
        kthread_copy_state_to_frame(frame);
        spinlock_unlock(&yield_spinlock);
    }
}

void kthread_scheduler_isr(
    uint64_t __attribute__((unused)) ticks_count, 
    idt_stack_frame_t* frame){
    kthread_yield_using_frame(frame);
}

void kthread_init_subsystem(void){
    yield_spinlock = 1;
    process_count = 1; //0 will stand for deallocated
    //alignment for fxsave operation is 64
    kaslub_init(&tasks_stub, sizeof(kthread_t), 64);
    kslub_init(&msg_stub, sizeof(msg_stub));
    timer_set_callback(kthread_scheduler_isr);
    kthread_current = kaslub_new(&tasks_stub);
    memset(kthread_current->regs, sizeof(kthread_current->regs), 0);
    memset((void*)&(kthread_current->frame), sizeof(kthread_current->frame), 0);
    kthread_current->next = kthread_current;
    kthread_current->prev = kthread_current;
    kthread_initialized_subsystem = true;
    yield_spinlock = 0;
}

bool kthread_is_initialized(){
    return kthread_initialized_subsystem;
}

uint64_t kthread_get_id(){
    return kthread_current->id;
}