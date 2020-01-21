#ifndef __KSTUB_H_INCLUDED__
#define __KSTUB_H_INCLUDED__

#include <utils.h>
#include <kheap.h>
#include <spinlock.h>

typedef struct kstub_struct{
    size_t object_size;
    kheap_object_header_t* head;
    spinlock_t spinlock;
} kstub_t;

void kstub_init(kstub_t* stub, size_t size);
void* kstub_new(kstub_t* stub);
void kstub_delete(kstub_t* stub, void* data);
void kstub_flush(kstub_t* stub);

#endif

