#ifndef __IDT_H_INCLDUED__
#define __IDT_H_INCLUDED__
#include <utils.h>

void idt_init(void);

typedef struct idt_stack_frame_struct {
    uint64_t intno;
    uint64_t cr4, cr3, cr2, cr0;
    uint64_t r15, r14;
    uint64_t r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rsi, rdi, rdx, rcx, rbx, rax;
    uint64_t errcode;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed)) idt_stack_frame_t;

typedef void (*idt_handler_t)(idt_stack_frame_t* frame);
typedef uint8_t idt_index_t;
void idt_set_handler(idt_index_t index, idt_handler_t handler);

#endif