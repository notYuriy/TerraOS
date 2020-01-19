#include <timer.h>
#include <video.h>
#include <idt.h>
#include <portio.h>
#include <pic.h>

uint64_t tick_count;

void timer_inc(idt_stack_frame_t* frame){
    tick_count++;
    printf("tick_count: %llu\n", tick_count);
}

void timer_init(uint32_t frequency){
    tick_count = 0;
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
    pic_enable_irq(0);
}

uint64_t timer_ticks_count(){
    return tick_count;
}