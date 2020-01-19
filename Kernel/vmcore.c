#include <vmcore.h>
#include <phmmngr.h>

vaddr_t vmcore_walk_to_next_table(vaddr_t current, vindex_t index){
    return (current << 9ULL) | ((uint64_t)index << 12ULL);
}

vaddr_t vmcore_walk_to_next_table_alloc(vaddr_t current, vindex_t index){
    uint64_t addr = current + 8ULL * index;
    uint64_t* ref = (uint64_t*)addr;
    if(!(*ref & VMCORE_PRESENT_FLAG)){
        *ref = phmmngr_new_frame() | VMCORE_KERNEL_FLAGS;
    }
    return (current << 9ULL) | ((uint64_t)index << 12ULL);
}

const vaddr_t vmcore_p4_vaddr = (vaddr_t)01777777777777777770000;

ventry_t* vmcore_get_p3_ref(vindex_t p4_index){
    vaddr_t ref = vmcore_p4_vaddr;
    ref += 8 * p4_index;
    return (ventry_t*)ref;
}
ventry_t* vmcore_get_p2_ref(vindex_t p4_index,
                            vindex_t p3_index){
    vaddr_t ref = vmcore_p4_vaddr;
    ref = vmcore_walk_to_next_table(ref, p4_index);
    ref += 8 * p3_index;
    return (ventry_t*)ref;
}
ventry_t* vmcore_get_p1_ref(vindex_t p4_index,
                            vindex_t p3_index,
                            vindex_t p2_index){
    vaddr_t ref = vmcore_p4_vaddr;
    ref = vmcore_walk_to_next_table(ref, p4_index);
    ref = vmcore_walk_to_next_table(ref, p3_index);
    ref += 8 * p2_index;
    return (ventry_t*)ref;
}

ventry_t* vmcore_get_p0_ref(vindex_t p4_index,
                            vindex_t p3_index,
                            vindex_t p2_index,
                            vindex_t p1_index){
    vaddr_t ref = vmcore_p4_vaddr;
    ref = vmcore_walk_to_next_table(ref, p4_index);
    ref = vmcore_walk_to_next_table(ref, p3_index);
    ref = vmcore_walk_to_next_table(ref, p2_index);
    ref += 8 * p1_index;
    return (ventry_t*)ref;
}

ventry_t* vmcore_get_addr_ref(vaddr_t addr){
    return vmcore_get_p0_ref(vmcore_p4_index(addr),
                            vmcore_p3_index(addr),
                            vmcore_p2_index(addr),
                            vmcore_p1_index(addr));
}


void vmcore_map_at(vaddr_t addr, physaddr_t physaddr){
    uint64_t ref = 0;
    ref = vmcore_walk_to_next_table_alloc(vmcore_p4_vaddr, vmcore_p4_index(addr));
    ref = vmcore_walk_to_next_table_alloc(ref, vmcore_p3_index(addr));
    ref = vmcore_walk_to_next_table_alloc(ref, vmcore_p2_index(addr));
    ref += 8 * vmcore_p1_index(addr);
    *(uint64_t*)ref = physaddr | VMCORE_KERNEL_FLAGS;
    asmutils_invalidate_cache(addr);
}

void vmcore_map_new_at(vaddr_t addr){
    vmcore_map_at(addr, phmmngr_new_frame());
}