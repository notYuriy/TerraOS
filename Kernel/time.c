#include <time.h>
#include <timer.h>
#include <video.h>

void time_sleep(uint64_t millis){
    uint64_t eta_ticks = (millis * timer_get_frequency()) / 1000;
    uint64_t current_ticks = timer_ticks_count();
    uint64_t end_ticks = current_ticks + eta_ticks;
    while(timer_ticks_count() < end_ticks) asm("pause":::);
}