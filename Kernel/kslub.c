#include <kslub.h>

void kslub_init(kslub_t* stub, size_t size){
    stub->object_size = size;
    stub->head = NULL;
}

void* kslub_new(kslub_t* stub){
    //spinlock_lock(&stub->spinlock);
    if(stub->head == NULL){
        //spinlock_unlock(&stub->spinlock);
        return kheap_malloc(stub->object_size);
    }
    void* result = kheap_get_data(stub->head);
    stub->head = stub->head->next;
    //spinlock_unlock(&stub->spinlock);
    return result;
}

void kslub_delete(kslub_t* stub, void* data){
    kheap_object_header_t* newobj = kheap_get_header(data);
    newobj->next = stub->head;
    stub->head = newobj;
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
}

void* kaslub_new(kaslub_t* stub){
    //spinlock_lock(&stub->spinlock);
    if(stub->head == NULL){
        //spinlock_unlock(&stub->spinlock);
        return kheap_malloc_aligned(stub->object_size, stub->object_align);
    }
    void* result = kheap_get_data(stub->head);
    stub->head = stub->head->next;
    //spinlock_unlock(&stub->spinlock);
    return result;
}

void kaslub_delete(kaslub_t* stub, void* data){
    kheap_object_header_t* newobj = kheap_get_header(data);
    newobj->next = stub->head;
    //This operation is atomic
    //because stubs are 8 byte aligned
    stub->head = newobj;
}

void kaslub_flush(kaslub_t* stub){
    while(stub->head != NULL){
        kheap_object_header_t* next = stub->head->next;
        kheap_free(kheap_get_data(stub->head));
        stub->head = next;
    }
}