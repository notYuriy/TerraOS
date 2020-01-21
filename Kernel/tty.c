#include <tty.h>
#include <video.h>
#include <kybrd.h>

size_t getline(char* buffer, size_t buffer_size){
    size_t pos = 0;
    kybrd_event_t event;
    while(true){
        asm("pause":::);
        event = kybrd_poll_event();
        if(!(event.mask & KYBRD_FLAG_PRESENT)){
            continue;
        }
        if(event.mask & KYBRD_FLAG_RELEASED){
            continue;
        }
        if(event.code == '\t'){
            continue;
        }
        if(event.code == '\n'){
            video_putc('\n');
            break;
        }
        if(event.code == '\b'){
            if(pos != 0){
                video_putc('\b');
                video_putc(' ');
                video_putc('\b');
                buffer[pos] = '\n';
                pos--;
            }
            continue;
        }
        if(pos == buffer_size - 1){
            continue;
        }
        buffer[pos++] = event.code;
        video_putc(event.code);
    }
    buffer[pos++] = '\0';
    return pos;
}
