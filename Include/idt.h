#ifndef __IDT_H_INCLUDED__
#define __IDT_H_INCLUDED__
#include <utils.h>

#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

void idt_init(void);

typedef struct idt_stack_frame_struct {
    uint64_t fs, gs, ds, es;
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