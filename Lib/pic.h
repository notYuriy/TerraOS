#ifndef __PIC_H_INCLUDED__
#define __PIC_H_INCLUDED__

#include <utils.h>

void pic_init(void);
void pic_enable_irq(uint8_t ind);
void pic_disable_irq(uint8_t ind);

#endif