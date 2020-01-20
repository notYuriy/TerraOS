#include "ksbrk.h"
#include <vmcore.h>
#include <phmmngr.h>
#include <video.h>

//beginning of memory, which is not used by kernel
vaddr_t ksbrk_kernel_end;
//beginning of unmapped memory
vaddr_t ksbrk_actual_end;

void ksbrk_init(void){
    ksbrk_kernel_end = KERNEL_INITIAL_MAPPING_END;
    ksbrk_actual_end = ksbrk_kernel_end;
}

void* ksbrk(int64_t delta){
    if(delta < 0){
        panic("Negative ksbrk is not supported yet =(\n");
    }
    vaddr_t new_kernel_end = ksbrk_kernel_end + delta;
    vaddr_t new_actual_end = up_align(new_kernel_end, 4 KB);
    void* result = (void*)ksbrk_kernel_end;
    for(vaddr_t cur = ksbrk_actual_end; cur < new_actual_end; cur += 4096){
        vmcore_map_new_at(cur);
        asmutils_invalidate_cache(cur);
    }
    ksbrk_actual_end = new_actual_end;
    ksbrk_kernel_end = new_kernel_end;
    return result;
}