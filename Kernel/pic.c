#include <pic.h>
#include <portio.h>


#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)
#define ICW1_ICW4	0x01
#define ICW1_SINGLE	0x02
#define ICW1_INTERVAL4	0x04
#define ICW1_LEVEL	0x08
#define ICW1_INIT	0x10
#define ICW4_8086	0x01
#define ICW4_AUTO	0x02
#define ICW4_BUF_SLAVE	0x08
#define ICW4_BUF_MASTER	0x0C
#define ICW4_SFNM	0x10

_Atomic size_t interrupt_cli_requests;

uint8_t pic_slave_mask;
uint8_t pic_master_mask;

void pic_delay(void){
    for(volatile int a = 0; a < 300; ++a);
}

void pic_init(void){
    interrupt_cli_requests = 0;
    uint8_t a1, a2;
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    pic_delay();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    pic_delay();
    outb(PIC1_DATA, 32);
    pic_delay();
    outb(PIC2_DATA, 40);
    pic_delay();
    outb(PIC1_DATA, 4);
    pic_delay();
    outb(PIC2_DATA, 2);
    pic_delay();	
    outb(PIC1_DATA, ICW4_8086);
    pic_delay();
    outb(PIC2_DATA, ICW4_8086);
    pic_delay();	
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
    asm("sti");
}

void pic_update(){
    outb(PIC1_DATA, pic_master_mask);
    outb(PIC2_DATA, pic_slave_mask);
}

void pic_enable_irq(uint8_t ind){
    if(ind > 8){
        pic_slave_mask &= ~(1 << (ind - 8));
    }else{
        pic_master_mask &= ~(1 << ind);
    }
    pic_update();
}

void pic_disable_irq(uint8_t ind){
if(ind > 8){
        pic_slave_mask |= (1 << (ind - 8));
    }else{
        pic_master_mask |= (1 << ind);
    }
    pic_update();
}
