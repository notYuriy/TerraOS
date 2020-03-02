#include <kslub.h>

void kslub_init(kslub_t* stub, size_t size){
    stub->object_size = size;
    stub->head = NULL;
    stub->spinlock = 0;
}

void* kslub_new(kslub_t* stub){
    kheap_object_header_t* expected;
    do {
        expected = atomic_load(&(stub->head));
        if(expected == NULL){
            return kheap_malloc(stub->object_size);
        }
    } while(!atomic_compare_exchange_strong(&(stub->head), &expected, expected->next));
    void* result = kheap_get_data(expected);
    return result;
}

void kslub_delete(kslub_t* stub, void* data){
    kheap_object_header_t* obj = kheap_get_header(data);
    kheap_object_header_t* expected_next;
    do {
       expected_next = atomic_load(&(stub->head));
       obj->next = expected_next;
    } while (!atomic_compare_exchange_strong(&(stub->head), &expected_next, obj));
}

void kslub_flush(kslub_t* stub){
    while(stub->head != NULL){
        kheap_object_header_t* next = stub->head->next;
        kheap_free(kheap_get_data(stub->head));
        stub->head = next;
    }
}

void kaslub_init(kaslub_t* stub, size_t size, size_t align){
    stub->object_size = size;
    stub->object_align = align;
    stub->head = NULL;
    stub->spinlock = 0;
}

void* kaslub_new(kaslub_t* stub){
    kheap_object_header_t* expected;
    do {
        expected = atomic_load(&(stub->head));
        if(expected == NULL){
            return kheap_malloc_aligned(stub->object_size, stub->object_align);
        }
    } while(!atomic_compare_exchange_strong(&(stub->head), &expected, expected->next));
    void* result = kheap_get_data(expected);
    return result;
}

void kaslub_delete(kaslub_t* stub, void* data){
    kheap_object_header_t* obj = kheap_get_header(data);
    kheap_object_header_t* expected_next;
    do {
       expected_next = atomic_load(&(stub->head));
       obj->next = expected_next;
    } while (!atomic_compare_exchange_strong(&(stub->head), &expected_next, obj));
}

void kaslub_flush(kaslub_t* stub){
    while(stub->head != NULL){
        kheap_object_header_t* next = stub->head->next;
        kheap_free(kheap_get_data(stub->head));
        stub->head = next;
    }
}