#ifndef __KSTUB_H_INCLUDED__
#define __KSTUB_H_INCLUDED__

#include <utils.h>
#include <kheap.h>
#include <spinlock.h>

typedef struct kslub_struct {
    size_t object_size;
    kheap_object_header_t* head;
    spinlock_t spinlock;
} kslub_t;

typedef struct kaslub_struct {
    size_t object_size;
    size_t object_align;
    kheap_object_header_t* head;
    spinlock_t spinlock;
} kaslub_t;

void kslub_init(kslub_t* stub, size_t size);
void* kslub_new(kslub_t* stub);
void kslub_delete(kslub_t* stub, void* data);
void kslub_flush(kslub_t* stub);

void kaslub_init(kaslub_t* stub, size_t size, size_t align);
void* kaslub_new(kaslub_t* stub);
void kaslub_delete(kaslub_t* stub, void* data);
void kaslub_flush(kaslub_t* stub);

#endif

