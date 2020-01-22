#include <kstub.h>

void kstub_init(kstub_t* stub, size_t size){
    stub->object_size = size;
    stub->head = NULL;
}

void* kstub_new(kstub_t* stub){
    spinlock_lock(&stub->spinlock);
    if(stub->head == NULL){
        spinlock_unlock(&stub->spinlock);
        return kheap_malloc(stub->object_size);
    }
    void* result = kheap_get_data(stub->head);
    stub->head = stub->head->next;
    spinlock_unlock(&stub->spinlock);
    return result;
}

void kstub_delete(kstub_t* stub, void* data){
    spinlock_lock(&stub->spinlock);
    kheap_object_header_t* newobj = kheap_get_header(data);
    newobj->next = stub->head;
    stub->head = newobj;
    spinlock_unlock(&stub->spinlock);
}

void kstub_flush(kstub_t* stub){
    while(stub->head != NULL){
        kheap_object_header_t* next = stub->head->next;
        kheap_free(kheap_get_data(stub->head));
        stub->head = next;
    }
}

void kastub_init(kastub_t* stub, size_t size, size_t align){
    stub->object_size = size;
    stub->object_align = align;
    stub->head = NULL;
}

void* kastub_new(kastub_t* stub){
    spinlock_lock(&stub->spinlock);
    if(stub->head == NULL){
        spinlock_unlock(&stub->spinlock);
        return kheap_malloc_aligned(stub->object_size, stub->object_align);
    }
    void* result = kheap_get_data(stub->head);
    stub->head = stub->head->next;
    spinlock_unlock(&stub->spinlock);
    return result;
}

void kastub_delete(kastub_t* stub, void* data){
    spinlock_lock(&stub->spinlock);
    kheap_object_header_t* newobj = kheap_get_header(data);
    newobj->next = stub->head;
    stub->head = newobj;
    spinlock_unlock(&stub->spinlock);
}

void kastub_flush(kastub_t* stub){
    while(stub->head != NULL){
        kheap_object_header_t* next = stub->head->next;
        kheap_free(kheap_get_data(stub->head));
        stub->head = next;
    }
}