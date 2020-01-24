#ifndef __VMCORE_H_INCLUDED__
#define __VMCORE_H_INCLUDED__

#include <utils.h>
#include <phmmngr.h>

#define VMCORE_FLAGS_MASK (uint64_t)((1<<12)-1)
#define VMCORE_PRESENT_FLAG (uint64_t)(1ULL<<0ULL)
#define VMCORE_WRITABLE_FLAG (uint64_t)(1ULL<<1ULL)
#define VMCORE_USER_ACCESSIBLE_FLAG (uint64_t)(1ULL<<2ULL)
#define VMCORE_WRITE_THROUGH_CACHING_FLAG (uint64_t)(1ULL<<3ULL)
#define VMCORE_DISABLE_CACHE_FLAG (uint64_t)(1ULL<<4ULL)
#define VMCORE_ACCESSED_FLAG (uint64_t)(1ULL<<5ULL)
#define VMCORE_DIRTY_FLAG (uint64_t)(1ULL<<6ULL)
#define VMCORE_HUGE_PAGE_FLAG (uint64_t)(1ULL<<7ULL)
#define VMCORE_GLOBAL_FLAG (uint64_t)(1ULL<<8ULL)
#define VMCORE_NO_EXECUTE_FLAG (uint64_t)(1ULL<<63ULL)
#define VMCORE_KERNEL_FLAGS VMCORE_PRESENT_FLAG | \
                                VMCORE_WRITABLE_FLAG
#define VMCORE_ALLOCATED_FLAG (uint64_t)(1ULL<<9ULL)

typedef uint64_t vaddr_t;
typedef uint64_t ventry_t;
typedef uint64_t vflags_t;
typedef size_t vindex_t;

extern void asmutils_load_p4_table(physaddr_t pageAddr);
extern void asmutils_invalidate_cache(vaddr_t addr);
ventry_t* vmcore_get_p3_ref(vindex_t p4_index);
ventry_t* vmcore_get_p2_ref(vindex_t p4_index,
                            vindex_t p3_index);
ventry_t* vmcore_get_p1_ref(vindex_t p4_index,
                            vindex_t p3_index,
                            vindex_t p2_index);
ventry_t* vmcore_get_p0_ref(vindex_t p4_index,
                            vindex_t p3_index,
                            vindex_t p2_index,
                            vindex_t p1_index);

ventry_t* vmcore_get_addr_ref(vaddr_t addr);

inline vindex_t vmcore_p4_index(vaddr_t addr){
    return (addr >> 39ULL) & 0777ULL;
}

inline vindex_t vmcore_p3_index(vaddr_t addr){
    return (addr >> 30ULL) & 0777ULL;
}

inline vindex_t vmcore_p2_index(vaddr_t addr){
    return (addr >> 21ULL) & 0777ULL;
}

inline vindex_t vmcore_p1_index(vaddr_t addr){
    return (addr >> 12ULL) & 0777ULL;
}

void vmcore_map_at(vaddr_t vaddr, physaddr_t physaddr);
void vmcore_map_new_at(vaddr_t vaddr);
vaddr_t vmcore_kmmap(vaddr_t vaddr, physaddr_t physaddr, uint64_t count);
#endif