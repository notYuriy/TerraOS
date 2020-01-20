#ifndef __PHMMNGR_H_INCLUDED__
#define __PHMMNGR_H_INCLUDED__
#include <utils.h>
#include <multiboot2.h>

typedef uint64_t physaddr_t;

typedef struct phmeminfo_struct {
    boot_memory_map_entry_t* mmap_entries;
    uint32_t mmap_entries_count;
    uint64_t preserved_area_physical_base;
    uint64_t preserved_area_physical_limit;
} phmeminfo_t;

//Initialize memory manager
void phmmngr_init(phmeminfo_t info);
//Allocates consecutive frames for ISA DMA
physaddr_t phmmngr_alloc(size_t count);
//Deallocates consecutive frames
void phmmngr_dealloc(physaddr_t addr, size_t count);
//Optimised version of physicalMemoryAlloc for one frame
physaddr_t phmmngr_new_frame(void);
//Wrapper of physicalMemoryDealloc to deallocate one frame
void phmmngr_free_frame(physaddr_t addr);


#endif