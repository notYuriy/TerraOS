#ifndef __KSTUB_H_INCLUDED__
#define __KSTUB_H_INCLUDED__

#include <utils.h>
#include <kheap.h>
#include <spinlock.h>

typedef struct kstub_struct {
    size_t object_size;
    kheap_object_header_t* head;
    spinlock_t spinlock;
} kstub_t;

typedef struct kastub_struct {
    size_t object_size;
    size_t object_align;
    kheap_object_header_t* head;
    spinlock_t spinlock;
} kastub_t;

void kstub_init(kstub_t* stub, size_t size);
void* kstub_new(kstub_t* stub);
void kstub_delete(kstub_t* stub, void* data);
void kstub_flush(kstub_t* stub);

void kastub_init(kastub_t* stub, size_t size, size_t align);
void* kastub_new(kastub_t* stub);
void kastub_delete(kastub_t* stub, void* data);
void kastub_flush(kastub_t* stub);

#endif

