#include <timer.h>
#include <video.h>
#include <idt.h>
#include <portio.h>
#include <pic.h>
#include <spinlock.h>
#include <kslub.h>

uint64_t ticks_count;
uint32_t timer_frequency;
timer_callback_t timer_callback;
kaslub_t tasks_stub;

void timer_inc(idt_stack_frame_t* frame){
    ticks_count++;
    if(timer_callback != NULL){
        timer_callback(ticks_count, frame);
    }
}

uint32_t timer_get_frequency(void){
    return timer_frequency;
}

void timer_init(uint32_t frequency){
    timer_frequency = frequency;
    ticks_count = 0;
    idt_set_handler(IRQ0, timer_inc);
    uint32_t divisor = 1193180 / frequency;
    if(divisor > 65536){
        panic("Timer: can't handle so small frequency");
        return;
    }
    outb(0x43, 0x36);
    uint16_t divlow = (uint16_t)divisor;
    outb(0x40, divlow & 0xff);
    outb(0x40, divlow >> 8);
    timer_callback = 0;
}

uint64_t timer_ticks_count(){
    return ticks_count;
}

void timer_enable(){
    pic_enable_irq(0);
}

void timer_disable(){
    pic_disable_irq(0);
}

void timer_set_callback(timer_callback_t callback){
    timer_callback = callback;
}