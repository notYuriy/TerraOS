#include <kheap.h>
#include <video.h>
#include <spinlock.h>
#include <ksbrk.h>

kheap_object_header_t literal_head;
kheap_object_header_t* head;
spinlock_t kheap_spinlock;

void kheap_init(phmeminfo_t info){
    head = &literal_head;
    head->next = NULL;
    head->size = 0;
    kheap_object_header_t* prev = head;
    uint64_t preserved_area_base = info.preserved_area_physical_base;
    uint64_t preserved_area_limit = info.preserved_area_physical_limit;
    uint64_t sep_highest = 
       preserved_area_base - sizeof(kheap_object_header_t);
    uint64_t post_preserved_header_addr = preserved_area_limit;
    uint64_t post_preserved_header_limit = 
        post_preserved_header_addr + sizeof(kheap_object_header_t);
    for(size_t i = 0; i < info.mmap_entries_count; ++i){
        boot_memory_map_entry_t* entry = info.mmap_entries + i;
        //memory is used by hardware should not be used for kernel heap
        if(entry->type != BOOT_MEMORY_MAP_AVAILABLE){
            continue;
        }
        //this would not be useful for kernel heap
        //because even placing small object_header
        //will cause page fault
        if(entry->base_addr >= KERNEL_INITIAL_MAPPING_SIZE - sizeof(kheap_object_header_t)){
            continue;
        }

        //memory beyond 1GB mark is not mapped => not useful
        uint64_t physical_upper_bound = entry->base_addr + entry->length;
        
        if(physical_upper_bound > KERNEL_INITIAL_MAPPING_SIZE){
            physical_upper_bound = KERNEL_INITIAL_MAPPING_SIZE;
        }
        //if there is not enough memory to place even one object header, it is not
        //useful too
        if(physical_upper_bound - entry->base_addr < sizeof(kheap_object_header_t)){
            continue;
        }
        //if this memory is fully avalible
        if(
            preserved_area_limit < entry->base_addr ||
            preserved_area_base > physical_upper_bound
        ){
            kheap_object_header_t* mem = 
            (kheap_object_header_t*)(entry->base_addr + KERNEL_MAPPING_BASE);
            mem->size = (physical_upper_bound + KERNEL_MAPPING_BASE)
             - (uint64_t)kheap_get_data(mem);
            mem->next = NULL;
            prev->next = mem;
            prev = mem;
            continue;
        }
        //case when kernel and boot info is located fully in this area
        //and there is enough memory to separate it
        //from both sides
        if(
            entry->base_addr <= sep_highest &&
            physical_upper_bound >= post_preserved_header_limit
        ){
            kheap_object_header_t* before_preserved;
            kheap_object_header_t* after_preserved;
            before_preserved = (kheap_object_header_t*)
            (entry->base_addr + KERNEL_MAPPING_BASE);
            after_preserved = (kheap_object_header_t*)
            (post_preserved_header_addr + KERNEL_MAPPING_BASE);
            prev->next = before_preserved;
            after_preserved->next = NULL;
            before_preserved->next = after_preserved;
            prev = after_preserved;
            before_preserved->size = preserved_area_base
                + KERNEL_MAPPING_BASE
                - (uint64_t)kheap_get_data(before_preserved);
            after_preserved->size = physical_upper_bound 
                + KERNEL_MAPPING_BASE
                - (uint64_t)kheap_get_data(after_preserved);
            continue;
        }
        //case when kernel and boot info limit is in this area
        //but there is not enough memory to separate preserved area base
        //or preserved area base is located before 
        //(which is very unlikely, because this area can not be mapped to
        //memory devices)
        if(physical_upper_bound >= post_preserved_header_limit){
            kheap_object_header_t* after_preserved;
            after_preserved = (kheap_object_header_t*)
            (post_preserved_header_addr + KERNEL_MAPPING_BASE);
            prev->next = after_preserved;
            after_preserved->next = NULL;
            prev = after_preserved;
            after_preserved->size = physical_upper_bound 
                + KERNEL_MAPPING_BASE
                - (uint64_t)kheap_get_data(after_preserved);
            continue;
        }
        //similar case, but with kernel base this time
        if(entry->base_addr <= sep_highest){
            
            kheap_object_header_t* before_preserved;
            before_preserved = (kheap_object_header_t*)
            (entry->base_addr + KERNEL_MAPPING_BASE);
            prev->next = before_preserved;
            before_preserved->next = NULL;
            prev = before_preserved;
            before_preserved->size = preserved_area_base
                + KERNEL_MAPPING_BASE
                - (uint64_t)kheap_get_data(before_preserved);
            continue;
        }
        //if we are here, this area is not useful for as    
        continue;
    }
    //at this moment, heap is fully initialized by sequence of avalible blocks
    kheap_spinlock = 0;
}

void* kheap_sbrk_malloc(uint64_t size){
    //calculating allocation size
    uint64_t real_size = size + sizeof(kheap_object_header_t);
    //requesting ksbrk
    void* allocated = ksbrk(real_size);
    kheap_object_header_t* allocated_header = allocated;
    allocated_header->size = size;
    return kheap_get_data(allocated_header);
}

void* kheap_search_free_blocks(size_t size){
    kheap_object_header_t* prev = head;
    size_t split_size = size + sizeof(kheap_object_header_t);
    while(prev->next != NULL){
        kheap_object_header_t* current = prev->next;
        if(current->size >= split_size){
            kheap_object_header_t* new_node = 
            (kheap_object_header_t*)(((uint64_t)kheap_get_data(current)) + size);
            new_node->next = current->next;
            new_node->size = current->size - split_size;
            current->next = NULL;
            current->size = size;
            prev->next = new_node;
            spinlock_unlock(&kheap_spinlock);
            return (void*)(current + 1);
        }
        if(current->size >= size){
            prev->next = current->next;
            current->next = NULL;
            spinlock_unlock(&kheap_spinlock);
            return (void*)(current + 1);
        }
        prev = prev->next;
    }
    return NULL;
}

void* kheap_malloc(size_t size){
    size = up_align(size, 16);
    spinlock_lock(&kheap_spinlock);
    void* result = kheap_search_free_blocks(size);
    if(result == NULL){
        printf("Search was not successful\n");
        result = kheap_sbrk_malloc(size);
        spinlock_unlock(&kheap_spinlock);
        return result;
    }
    spinlock_unlock(&kheap_spinlock);
    return result;
}

void kheap_free(void* addr){
    kheap_object_header_t* obj = (kheap_object_header_t*)(addr) - 1;
    obj->next = head->next;
    head->next = obj;
}

void* kheap_search_aligned_free_blocks(size_t size, size_t align){
    kheap_object_header_t* prev = head;
    size_t alloc_size = size;
    while(prev->next != NULL){
        kheap_object_header_t* cur = prev->next;
        uint64_t limit = (uint64_t)kheap_get_data(cur) + cur->size;
        uint64_t pages_base = up_align((uint64_t)kheap_get_data(cur), align);
        uint64_t space_before_align = 
            pages_base - (uint64_t)kheap_get_data(cur);
        uint64_t pages_end = pages_base + alloc_size;
        uint64_t likely_header_offset = pages_base - sizeof(kheap_object_header_t);
        uint64_t space_after_pages = limit - pages_end;
        if(pages_end > ((uint64_t)kheap_get_data(cur) + cur->size)){
            prev = prev->next;
            continue;
        }
        if(space_before_align == 0){
            cur->size = alloc_size;
            if(space_after_pages >= sizeof(kheap_object_header_t)){
                kheap_object_header_t* after = (kheap_object_header_t*)pages_end;
                after->next = cur->next;
                cur->next = after;
                after->size = space_after_pages - sizeof(kheap_object_header_t);
            }
            return kheap_get_data(cur);
        }
        if(space_before_align >= sizeof(kheap_object_header_t)){
            kheap_object_header_t* allocated;
            allocated = (kheap_object_header_t*)likely_header_offset;
            allocated->next = NULL;
            allocated->size = alloc_size;
            cur->size = (uint64_t)allocated - (uint64_t)kheap_get_data(cur);
            if(space_after_pages >= sizeof(kheap_object_header_t)){
                kheap_object_header_t* after = (kheap_object_header_t*)pages_end;
                after->next = cur->next;
                cur->next = after;
                after->size = space_after_pages - sizeof(kheap_object_header_t);
            }
            return kheap_get_data(allocated);
        }
        uint64_t new_pages_base = pages_base + align;
        uint64_t new_pages_end = pages_end + align;
        uint64_t new_header_base = new_pages_base - sizeof(kheap_object_header_t);
        uint64_t new_space_after_pages = limit - new_pages_end;
        if(new_pages_end <= limit){
            kheap_object_header_t* allocated;
            allocated = (kheap_object_header_t*)new_header_base;
            allocated->size = alloc_size;
            cur->size = (uint64_t)allocated - (uint64_t)kheap_get_data(cur);
            if(new_space_after_pages >= sizeof(kheap_object_header_t)){
                kheap_object_header_t* after = (kheap_object_header_t*)new_pages_end;
                after->next = cur->next;
                cur->next = after;
                after->size = new_space_after_pages - sizeof(kheap_object_header_t);
            }
            return kheap_get_data(allocated);
        }
        //leaking memory is not acceptable, so we move to next variant
        prev = prev->next;
    }
    return NULL;
}

void* kheap_malloc_aligned(size_t size, size_t align){
    spinlock_lock(&kheap_spinlock);
    void* result = kheap_search_aligned_free_blocks(size, align);
    if(result == NULL){
        uint64_t kernel_end = (uint64_t)ksbrk(0);
        uint64_t after_header = kernel_end + sizeof(kheap_object_header_t);
        uint64_t payload_start = up_align(after_header, align);
        while(payload_start - after_header < sizeof(kheap_object_header_t)){
            payload_start += align;
        }
        uint64_t first_header_addr = kernel_end;
        uint64_t second_header_addr = payload_start - sizeof(kheap_object_header_t);
        ksbrk(payload_start + size - kernel_end);
        kheap_object_header_t* first_header = (kheap_object_header_t*)first_header_addr;
        kheap_object_header_t* second_header = (kheap_object_header_t*)second_header_addr;
        first_header->size = (uint64_t)second_header - (uint64_t)kheap_get_data(first_header);
        first_header->next = head->next;
        head->next = first_header;
        spinlock_unlock(&kheap_spinlock);
        return kheap_get_data(second_header);
    }
    spinlock_unlock(&kheap_spinlock);
    return result;   
}

void kheap_traverse(void){
    spinlock_lock(&kheap_spinlock);
    kheap_object_header_t* prev = head;
    while(prev->next != NULL){
        kheap_object_header_t* cur = prev->next;
        printf("block at %p:\ncur->size = %lu, cur->next = %p\n", cur, cur->size, cur->next);
        prev = prev->next;
    }
    spinlock_unlock(&kheap_spinlock);
}