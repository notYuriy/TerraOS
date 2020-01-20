#ifndef __KSBRK_H_INCLUDED__
#define __KSBRK_H_INCLUDED__

#include <utils.h>

void ksbrk_init();
void* ksbrk(int64_t delta);

#endif