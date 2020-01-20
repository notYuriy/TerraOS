#ifndef __KEYBOARD_H_INCLUDED__
#define __KEYBOARD_H_INCLUDED__

#include <utils.h>

#define KYBRD_FLAG_RELEASED (1<<1)
#define KYBRD_FLAG_ALT (1<<2)
#define KYBRD_FLAG_CONTROL (1<<3)
#define KYBRD_FLAG_SHIFT (1<<4)
#define KYBRD_FLAG_PRESENT (1<<6)

typedef struct kybrd_event_struct {
    char code;
    uint8_t mask;
} kybrd_event_t;    

typedef uint8_t kybrd_code_t;
void kybrd_init(void);
kybrd_event_t kybrd_poll_event(void);
void kybrd_enable(void);
void kybrd_disable(void);

#endif