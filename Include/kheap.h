#ifndef __KHEAP_H_INCLUDED__
#define __KHEAP_H_INCLUDED__

#include <utils.h>
#include <phmmngr.h>

//header of every heap memory region
typedef struct kheap_object_header_struct kheap_object_header_t;

typedef struct kheap_object_header_struct {
        kheap_object_header_t* next;
        size_t size;
} kheap_object_header_t;

//get data location from header location
inline void* kheap_get_data(kheap_object_header_t* header){
        return (void*)(header + 1);
}

//get header location from data location
inline kheap_object_header_t* kheap_get_header(void* data){
        return ((kheap_object_header_t*)data) - 1;
}

//Initializes kernel heap
void kheap_init(phmeminfo_t info);
//Allocate memory in kernel heap
void* kheap_malloc(size_t size);
//Free memory in kernel heap
void kheap_free(void* addr);
//Allocate aligned memory
void* kheap_malloc_aligned(size_t size, size_t align);
//Traverse heap
void kheap_traverse(void);

#endif