#include <idt.h>
#include <video.h>
#include <portio.h>
#include <pic.h>

typedef struct idt_entry_struct {
    uint16_t addr_low;
    uint16_t selector;
    uint8_t ist : 3;
    uint8_t zeroed1 : 5;
    uint8_t type : 1;
    uint8_t ones1 : 3;
    uint8_t zeroed3 : 1;
    uint8_t dpl : 2;
    uint8_t present : 1;
    uint16_t addr_middle;
    uint32_t addr_high;
    uint32_t reserved;
} __attribute__((packed)) idt_entry_t;

typedef struct idt_pointer_struct {
    //according to the intel programming manual
    //size of base is 48 bits
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idt_pointer_t;

typedef struct idt_struct {
    idt_entry_t vectors[256];
} __attribute__((packed)) idt_t;

idt_t idt;
idt_handler_t handlers[256];

void load_idt(void){
    idt_pointer_t pointer;
    pointer.base = (uint64_t)&(idt.vectors);
    pointer.limit = sizeof(idt.vectors);
    extern void asmutils_load_idt(idt_pointer_t* idtr);
    asmutils_load_idt(&pointer);
}

void install_idt_vector(int index, uint64_t addr){
    idt.vectors[index].addr_low = (uint16_t)(addr);
    idt.vectors[index].addr_middle = (uint16_t)(addr >> 16);
    idt.vectors[index].addr_high = (uint32_t)(addr >> 32);
    idt.vectors[index].zeroed1 = 0;
    idt.vectors[index].ones1 = 0b111;
    idt.vectors[index].zeroed3 = 0;
    idt.vectors[index].selector = 0x8;
    idt.vectors[index].type = 0;
    idt.vectors[index].present = 1;
    idt.vectors[index].ist = 0;
    idt.vectors[index].dpl = 0;
}

#define INSTALL_HANDLER(n) extern void int##n##_handler();\
install_idt_vector(n, (uint64_t)int##n##_handler);

void idt_default_handler(idt_stack_frame_t* frame){
    char* exceptions_names[] ={
        "Divide-by-zero Error",
        "Debug",
        "Non-maskable Interrupt",
        "Breakpoint",
        "Overflow",
        "Bound Range Exceeded",
        "Invalid Opcode",
        "Device Not Available",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Invalid TSS",
        "Segment Not Present",
        "Stack-Segment Fault",
        "General Protection Fault",
        "Page Fault",
        "Reserved",
        "x87 Floating-Point exception",
        "Alignment Check",
        "Machine Check",
        "SIMD Floating-Point Exception",
        "Virtualization Exception",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Security Exception",
        "Reserved"
    };
    video_packed_color_t color = video_get_packed_color();
    video_set_foreground(red);
    if(frame->intno > 31){
        printf("Unhandled interrupt with no %lld\n", frame->intno);
    }
    else{
        printf("Unhandled exception with id %llu: %s\n", frame->intno, exceptions_names[frame->intno]);
        printf("Error code %llu\n", frame->errcode);
        printf("Addr %p\n", frame->cr2);
        while(true);
    }
    
    video_set_packed_color(color);
}

void idt_init(void){
    load_idt();
    //this code was autogenerated
    //and i have no idea
    //how to write it differently
    {
        INSTALL_HANDLER(0)
        INSTALL_HANDLER(1)
        INSTALL_HANDLER(2)
        INSTALL_HANDLER(3)
        INSTALL_HANDLER(4)
        INSTALL_HANDLER(5)
        INSTALL_HANDLER(6)
        INSTALL_HANDLER(7)
        INSTALL_HANDLER(8)
        INSTALL_HANDLER(9)
        INSTALL_HANDLER(10)
        INSTALL_HANDLER(11)
        INSTALL_HANDLER(12)
        INSTALL_HANDLER(13)
        INSTALL_HANDLER(14)
        INSTALL_HANDLER(15)
        INSTALL_HANDLER(16)
        INSTALL_HANDLER(17)
        INSTALL_HANDLER(18)
        INSTALL_HANDLER(19)
        INSTALL_HANDLER(20)
        INSTALL_HANDLER(21)
        INSTALL_HANDLER(22)
        INSTALL_HANDLER(23)
        INSTALL_HANDLER(24)
        INSTALL_HANDLER(25)
        INSTALL_HANDLER(26)
        INSTALL_HANDLER(27)
        INSTALL_HANDLER(28)
        INSTALL_HANDLER(29)
        INSTALL_HANDLER(30)
        INSTALL_HANDLER(31)
        INSTALL_HANDLER(32)
        INSTALL_HANDLER(33)
        INSTALL_HANDLER(34)
        INSTALL_HANDLER(35)
        INSTALL_HANDLER(36)
        INSTALL_HANDLER(37)
        INSTALL_HANDLER(38)
        INSTALL_HANDLER(39)
        INSTALL_HANDLER(40)
        INSTALL_HANDLER(41)
        INSTALL_HANDLER(42)
        INSTALL_HANDLER(43)
        INSTALL_HANDLER(44)
        INSTALL_HANDLER(45)
        INSTALL_HANDLER(46)
        INSTALL_HANDLER(47)
        INSTALL_HANDLER(48)
        INSTALL_HANDLER(49)
        INSTALL_HANDLER(50)
        INSTALL_HANDLER(51)
        INSTALL_HANDLER(52)
        INSTALL_HANDLER(53)
        INSTALL_HANDLER(54)
        INSTALL_HANDLER(55)
        INSTALL_HANDLER(56)
        INSTALL_HANDLER(57)
        INSTALL_HANDLER(58)
        INSTALL_HANDLER(59)
        INSTALL_HANDLER(60)
        INSTALL_HANDLER(61)
        INSTALL_HANDLER(62)
        INSTALL_HANDLER(63)
        INSTALL_HANDLER(64)
        INSTALL_HANDLER(65)
        INSTALL_HANDLER(66)
        INSTALL_HANDLER(67)
        INSTALL_HANDLER(68)
        INSTALL_HANDLER(69)
        INSTALL_HANDLER(70)
        INSTALL_HANDLER(71)
        INSTALL_HANDLER(72)
        INSTALL_HANDLER(73)
        INSTALL_HANDLER(74)
        INSTALL_HANDLER(75)
        INSTALL_HANDLER(76)
        INSTALL_HANDLER(77)
        INSTALL_HANDLER(78)
        INSTALL_HANDLER(79)
        INSTALL_HANDLER(80)
        INSTALL_HANDLER(81)
        INSTALL_HANDLER(82)
        INSTALL_HANDLER(83)
        INSTALL_HANDLER(84)
        INSTALL_HANDLER(85)
        INSTALL_HANDLER(86)
        INSTALL_HANDLER(87)
        INSTALL_HANDLER(88)
        INSTALL_HANDLER(89)
        INSTALL_HANDLER(90)
        INSTALL_HANDLER(91)
        INSTALL_HANDLER(92)
        INSTALL_HANDLER(93)
        INSTALL_HANDLER(94)
        INSTALL_HANDLER(95)
        INSTALL_HANDLER(96)
        INSTALL_HANDLER(97)
        INSTALL_HANDLER(98)
        INSTALL_HANDLER(99)
        INSTALL_HANDLER(100)
        INSTALL_HANDLER(101)
        INSTALL_HANDLER(102)
        INSTALL_HANDLER(103)
        INSTALL_HANDLER(104)
        INSTALL_HANDLER(105)
        INSTALL_HANDLER(106)
        INSTALL_HANDLER(107)
        INSTALL_HANDLER(108)
        INSTALL_HANDLER(109)
        INSTALL_HANDLER(110)
        INSTALL_HANDLER(111)
        INSTALL_HANDLER(112)
        INSTALL_HANDLER(113)
        INSTALL_HANDLER(114)
        INSTALL_HANDLER(115)
        INSTALL_HANDLER(116)
        INSTALL_HANDLER(117)
        INSTALL_HANDLER(118)
        INSTALL_HANDLER(119)
        INSTALL_HANDLER(120)
        INSTALL_HANDLER(121)
        INSTALL_HANDLER(122)
        INSTALL_HANDLER(123)
        INSTALL_HANDLER(124)
        INSTALL_HANDLER(125)
        INSTALL_HANDLER(126)
        INSTALL_HANDLER(127)
        INSTALL_HANDLER(128)
        INSTALL_HANDLER(129)
        INSTALL_HANDLER(130)
        INSTALL_HANDLER(131)
        INSTALL_HANDLER(132)
        INSTALL_HANDLER(133)
        INSTALL_HANDLER(134)
        INSTALL_HANDLER(135)
        INSTALL_HANDLER(136)
        INSTALL_HANDLER(137)
        INSTALL_HANDLER(138)
        INSTALL_HANDLER(139)
        INSTALL_HANDLER(140)
        INSTALL_HANDLER(141)
        INSTALL_HANDLER(142)
        INSTALL_HANDLER(143)
        INSTALL_HANDLER(144)
        INSTALL_HANDLER(145)
        INSTALL_HANDLER(146)
        INSTALL_HANDLER(147)
        INSTALL_HANDLER(148)
        INSTALL_HANDLER(149)
        INSTALL_HANDLER(150)
        INSTALL_HANDLER(151)
        INSTALL_HANDLER(152)
        INSTALL_HANDLER(153)
        INSTALL_HANDLER(154)
        INSTALL_HANDLER(155)
        INSTALL_HANDLER(156)
        INSTALL_HANDLER(157)
        INSTALL_HANDLER(158)
        INSTALL_HANDLER(159)
        INSTALL_HANDLER(160)
        INSTALL_HANDLER(161)
        INSTALL_HANDLER(162)
        INSTALL_HANDLER(163)
        INSTALL_HANDLER(164)
        INSTALL_HANDLER(165)
        INSTALL_HANDLER(166)
        INSTALL_HANDLER(167)
        INSTALL_HANDLER(168)
        INSTALL_HANDLER(169)
        INSTALL_HANDLER(170)
        INSTALL_HANDLER(171)
        INSTALL_HANDLER(172)
        INSTALL_HANDLER(173)
        INSTALL_HANDLER(174)
        INSTALL_HANDLER(175)
        INSTALL_HANDLER(176)
        INSTALL_HANDLER(177)
        INSTALL_HANDLER(178)
        INSTALL_HANDLER(179)
        INSTALL_HANDLER(180)
        INSTALL_HANDLER(181)
        INSTALL_HANDLER(182)
        INSTALL_HANDLER(183)
        INSTALL_HANDLER(184)
        INSTALL_HANDLER(185)
        INSTALL_HANDLER(186)
        INSTALL_HANDLER(187)
        INSTALL_HANDLER(188)
        INSTALL_HANDLER(189)
        INSTALL_HANDLER(190)
        INSTALL_HANDLER(191)
        INSTALL_HANDLER(192)
        INSTALL_HANDLER(193)
        INSTALL_HANDLER(194)
        INSTALL_HANDLER(195)
        INSTALL_HANDLER(196)
        INSTALL_HANDLER(197)
        INSTALL_HANDLER(198)
        INSTALL_HANDLER(199)
        INSTALL_HANDLER(200)
        INSTALL_HANDLER(201)
        INSTALL_HANDLER(202)
        INSTALL_HANDLER(203)
        INSTALL_HANDLER(204)
        INSTALL_HANDLER(205)
        INSTALL_HANDLER(206)
        INSTALL_HANDLER(207)
        INSTALL_HANDLER(208)
        INSTALL_HANDLER(209)
        INSTALL_HANDLER(210)
        INSTALL_HANDLER(211)
        INSTALL_HANDLER(212)
        INSTALL_HANDLER(213)
        INSTALL_HANDLER(214)
        INSTALL_HANDLER(215)
        INSTALL_HANDLER(216)
        INSTALL_HANDLER(217)
        INSTALL_HANDLER(218)
        INSTALL_HANDLER(219)
        INSTALL_HANDLER(220)
        INSTALL_HANDLER(221)
        INSTALL_HANDLER(222)
        INSTALL_HANDLER(223)
        INSTALL_HANDLER(224)
        INSTALL_HANDLER(225)
        INSTALL_HANDLER(226)
        INSTALL_HANDLER(227)
        INSTALL_HANDLER(228)
        INSTALL_HANDLER(229)
        INSTALL_HANDLER(230)
        INSTALL_HANDLER(231)
        INSTALL_HANDLER(232)
        INSTALL_HANDLER(233)
        INSTALL_HANDLER(234)
        INSTALL_HANDLER(235)
        INSTALL_HANDLER(236)
        INSTALL_HANDLER(237)
        INSTALL_HANDLER(238)
        INSTALL_HANDLER(239)
        INSTALL_HANDLER(240)
        INSTALL_HANDLER(241)
        INSTALL_HANDLER(242)
        INSTALL_HANDLER(243)
        INSTALL_HANDLER(244)
        INSTALL_HANDLER(245)
        INSTALL_HANDLER(246)
        INSTALL_HANDLER(247)
        INSTALL_HANDLER(248)
        INSTALL_HANDLER(249)
        INSTALL_HANDLER(250)
        INSTALL_HANDLER(251)
        INSTALL_HANDLER(252)
        INSTALL_HANDLER(253)
        INSTALL_HANDLER(254)
        INSTALL_HANDLER(255)
    }
    for(size_t i = 0; i < 255; ++i){
        idt_set_handler(i, idt_default_handler);
    }
}

void idt_dispatcher(idt_stack_frame_t* frame){
    handlers[frame->intno](frame);
    if(frame->intno > 40){
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);
}

void idt_set_handler(idt_index_t index, idt_handler_t handler){
    handlers[index] = handler;
}