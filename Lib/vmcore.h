#ifndef __VMCORE_H_INCLUDED__
#define __VMCORE_H_INCLUDED__

#include <utils.h>
#include <phmmngr.h>

#define vmcore_flags_mask (uint64_t)((1<<12)-1)
#define vmcore_present_flag (uint64_t)(1ULL<<0ULL)
#define vmcore_writable_flag (uint64_t)(1ULL<<1ULL)
#define vmcore_user_accessible_flag (uint64_t)(1ULL<<2ULL)
#define vmcore_write_through_caching_flag (uint64_t)(1ULL<<3ULL)
#define vmcore_disable_cache_flag (uint64_t)(1ULL<<4ULL)
#define vmcore_accessed_flag (uint64_t)(1ULL<<5ULL)
#define vmcore_dirty_flag (uint64_t)(1ULL<<6ULL)
#define vmcore_huge_page_flag (uint64_t)(1ULL<<7ULL)
#define vmcore_global_flag (uint64_t)(1ULL<<8ULL)
#define vmcore_no_execute_flag (uint64_t)(1ULL<<63ULL)
#define vmcore_kernel_flags vmcore_present_flag | \
                                vmcore_writable_flag | \
                                vmcore_user_accessible_flag

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
#endif