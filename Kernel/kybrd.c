#include <kybrd.h>
#include <idt.h>
#include <pic.h>
#include <portio.h>
#include <video.h>
#include <spinlock.h>
#include <timer.h>

#define KYBRD_BUFFER_SIZE 65536

kybrd_event_t kybrd_buffer[KYBRD_BUFFER_SIZE];
size_t kybrd_buffer_head, kybrd_buffer_tail;
spinlock_t kybrd_spinlock;
bool kybrd_alt_pressed;
bool kybrd_left_shift_pressed;
bool kybrd_right_shift_pressed;
bool kybrd_ctrl_pressed;
bool kybrd_caps_pressed;

char kybrd_scancodes[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

inline bool kybrd_buffer_is_full(void){
    return (kybrd_buffer_head - kybrd_buffer_tail) == 1;
}

inline bool kybrd_buffer_is_empty(void){
    return kybrd_buffer_tail == kybrd_buffer_head;
}

inline size_t kybrd_increment_pos(size_t pos){
    size_t result = pos + 1;
    if(result == KYBRD_BUFFER_SIZE){
        result = 0;
    }
    return result;
}

inline void kybrd_buffer_push_back(kybrd_event_t c){
    kybrd_buffer[kybrd_buffer_tail] = c;
    //discard the latest one in the case of overflow
    if(kybrd_buffer_is_full())
        kybrd_buffer_head = kybrd_increment_pos(kybrd_buffer_head);
    kybrd_buffer_tail = kybrd_increment_pos(kybrd_buffer_tail);   
}

inline kybrd_event_t kybrd_buffer_pop_back(){
    if(kybrd_buffer_is_empty()){
        kybrd_event_t empty;
        empty.code = '\0';
        empty.mask = 0;
        return empty;
    }
    kybrd_event_t result = kybrd_buffer[kybrd_buffer_head];
    kybrd_buffer_head = kybrd_increment_pos(kybrd_buffer_head);
    return result;
}

#define KYBRD_ALT_PRESSED_SCANCODE 0x38
#define KYBRD_ALT_RELEASED_SCANCODE 0xb8
#define KYBRD_LEFT_SHIFT_PRESSED_SCANCODE 0x2a
#define KYBRD_LEFT_SHIFT_RELEASED_SCANCODE 0xaa
#define KYBRD_RIGHT_SHIFT_PRESSED_SCANCODE 0x36
#define KYBRD_RIGHT_SHIFT_RELEASED_SCANCODE 0xb6
#define KYBRD_CTRL_PRESSED_SCANCODE 0x1d
#define KYBRD_CTRL_RELEASED_SCANCODE 0x9d
#define KYBRD_CAPS_PRESSED_SCANCODE 0x3a
#define KYBRD_CAPS_RELEASED_SCANCODE 0xba

char kybrd_get_char(char c, bool capitalize){
    if(c & 0x80){
        c = c & (~(char)(0x80));
    }
    c = kybrd_scancodes[c];
    if(!capitalize){
        return c;
    }
    if(c >= 'A' && c <= 'Z'){
        return c - 'A' + 'a';
    }
    if(c >= 'a' && c <= 'z'){
        return c - 'a' + 'A';
    }
    return c;
}

void kybrd_irq_handler(){
    kybrd_code_t code = inb(0x60);
    switch (code){
        case KYBRD_ALT_PRESSED_SCANCODE:
            kybrd_alt_pressed = true;
            break;
        case KYBRD_ALT_RELEASED_SCANCODE:
            kybrd_alt_pressed = false;
            break;
        case KYBRD_LEFT_SHIFT_PRESSED_SCANCODE:
            kybrd_left_shift_pressed = true;
            break;
        case KYBRD_LEFT_SHIFT_RELEASED_SCANCODE:
            kybrd_left_shift_pressed = false;
            break;
        case KYBRD_RIGHT_SHIFT_PRESSED_SCANCODE:
            kybrd_right_shift_pressed = true;
            break;
        case KYBRD_RIGHT_SHIFT_RELEASED_SCANCODE:
            kybrd_right_shift_pressed = false;
            break;
        case KYBRD_CTRL_PRESSED_SCANCODE:
            kybrd_ctrl_pressed = true;
            break;
        case KYBRD_CTRL_RELEASED_SCANCODE:
            kybrd_ctrl_pressed = false;
            break;
        case KYBRD_CAPS_PRESSED_SCANCODE:
            kybrd_caps_pressed = !kybrd_caps_pressed;
            break;
        case KYBRD_CAPS_RELEASED_SCANCODE:
            break;
        default:{
            bool kybrd_shift = 
                kybrd_left_shift_pressed || kybrd_right_shift_pressed;
            if(kybrd_caps_pressed){
                kybrd_shift = !kybrd_shift;
            }
            kybrd_event_t event;
            event.code = kybrd_get_char(code, kybrd_shift);
            if(event.code == 57){
                printf("Caps code: 0x%x\n", (int)code);
            }
            event.mask = KYBRD_FLAG_PRESENT;
            event.mask |= (kybrd_shift)?(KYBRD_FLAG_SHIFT):(0);
            event.mask |= (kybrd_alt_pressed)?(KYBRD_FLAG_ALT):(0);
            event.mask |= (kybrd_ctrl_pressed)?(KYBRD_FLAG_CONTROL):(0);
            event.mask |= (code & 0x80)?(KYBRD_FLAG_RELEASED):(0);
            kybrd_buffer_push_back(event);
        }
    }
}

kybrd_event_t kybrd_poll_event(void){
    kybrd_event_t result = kybrd_buffer_pop_back();
    return result;
}

void kybrd_init(void){
    idt_set_handler(IRQ1, kybrd_irq_handler);
    kybrd_buffer_head = 0;
    kybrd_buffer_tail = 0;
    kybrd_spinlock = 0;
    kybrd_alt_pressed = false;
    kybrd_ctrl_pressed = false;
    kybrd_left_shift_pressed = false;
    kybrd_right_shift_pressed = false;
    kybrd_caps_pressed = false;
}

void kybrd_enable(void){
    pic_enable_irq(1);
}

void kybrd_disable(void){
    pic_disable_irq(1);
}