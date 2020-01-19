#include <phmmngr.h>
#include <kheap.h>
#include <video.h>
#include <spinlock.h>

phmeminfo_t info;
uint64_t phmmngr_upper_bound;
uint64_t phmmngr_search_bound;
uint64_t phmmngr_bitmap_size;
uint64_t* phmmngr_bitmap;
spinlock_t phmmngr_spinlock;


void phmmngr_claim_bit(physaddr_t addr){
    phmmngr_bitmap[addr / (4096ULL * 64ULL)] |= (1ULL << ((addr / 4096ULL) % 64ULL));
}

void phmmngr_free_bit(physaddr_t addr){
    uint64_t mask = ~(1ULL << ((addr / 4096ULL) % 64ULL));
    phmmngr_bitmap[addr / (4096ULL * 64ULL)] &= mask;
}

bool phmmngr_is_free(physaddr_t addr){
    return 
    (
        (phmmngr_bitmap[addr / (4096ULL * 64ULL)] 
        & (1 << ((addr / 4096ULL) % 64ULL))) == 0ULL
    );
}

void phmmngr_claim_bits(physaddr_t begin, physaddr_t end){
    begin = down_align(begin, 4096ULL);
    end = up_align(end, 4096ULL);
    for(physaddr_t i = begin; i < end; i += 4096ULL)
    {
        phmmngr_claim_bit(i);
    }
}

void phmmngr_free_bits(physaddr_t begin, physaddr_t end){
    begin = down_align(begin, 4096ULL);
    end = up_align(end, 4096ULL);
    for(physaddr_t i = begin; i < end; i += 4096ULL)
    {
        phmmngr_free_bit(i);
    }
}

void phmmngr_init(phmeminfo_t _info){
    memcpy(&info, &_info, sizeof(phmeminfo_t));
    phmmngr_upper_bound = 0ULL;
    for(uint32_t i = 0ULL; i < info.mmap_entries_count; ++i)
    {
        if(info.mmap_entries[i].type != boot_memory_map_avalible_map_id)
        {
            continue;
        }
        uint32_t end = info.mmap_entries[i].base_addr +
         info.mmap_entries[i].length;
        if(end > phmmngr_upper_bound)
        {
            phmmngr_upper_bound = end;
        }
    }
    phmmngr_search_bound = up_align(phmmngr_upper_bound, 4096 * 64);
    phmmngr_bitmap_size = phmmngr_search_bound/(4096 * 64);
    phmmngr_bitmap = kheap_malloc(phmmngr_bitmap_size * 8);
    if(phmmngr_bitmap == NULL){
        panic("Physical memory manager: can't allocate bitmap");
    }
    longmemset(phmmngr_bitmap, phmmngr_bitmap_size, 0);
    phmmngr_claim_bits(0, KERNEL_INITIAL_MAPPING_SIZE);
    phmmngr_claim_bits(phmmngr_upper_bound, phmmngr_search_bound);
    for(uint32_t i = 0ULL; i < info.mmap_entries_count; ++i)
    {
        if(info.mmap_entries[i].type == boot_memory_map_avalible_map_id)
        {
            continue;
        }
        phmmngr_claim_bits
        (
            info.mmap_entries[i].base_addr, 
            info.mmap_entries[i].base_addr + info.mmap_entries[i].length
        );
    }
    phmmngr_spinlock = 0;
}

physaddr_t phmmngr_alloc(size_t count){
    spinlock_lock(&phmmngr_spinlock);
    physaddr_t begin = 0;
    size_t seqcount = 0;
    for(physaddr_t i = 0; i < phmmngr_search_bound; i += 4096)
    {
        if(phmmngr_is_free(i))
        {
            if(begin == 0)
            {
                seqcount = 0;
                begin = i;
            }
            else
            {
                seqcount++;
                if(seqcount == count){
                    phmmngr_claim_bits(begin, i);
                    spinlock_unlock(&phmmngr_spinlock);
                    return begin;
                }
            }
        }
        else
        {
            if(begin != 0)
            {
                begin = 0;
                seqcount = 0;
            }
        }
    }
    spinlock_unlock(&phmmngr_spinlock);
    return 0;
}

physaddr_t phmmngr_new_frame(void){
    spinlock_lock(&phmmngr_spinlock);
    for(size_t i = 0; i < phmmngr_bitmap_size; ++i)
    {
        if(phmmngr_bitmap[i] != (unsigned long long int)(-1LL))
        {
            physaddr_t base = 64 * 4096 * i;
            int bitpos = bit_scan_forward(~phmmngr_bitmap[i]);
            physaddr_t addr = base + bitpos * 4096;
            phmmngr_claim_bit(addr);
            spinlock_unlock(&phmmngr_spinlock);
            return addr;
        }
    }
    spinlock_unlock(&phmmngr_spinlock);
    return 0;
}

void phmmngr_dealloc(physaddr_t addr, size_t count){
    spinlock_lock(&phmmngr_spinlock);
    phmmngr_free_bits(addr, addr + 4096 * count);
    spinlock_unlock(&phmmngr_spinlock);
}

void phmmngr_free_frame(physaddr_t addr){
    phmmngr_dealloc(addr, 1);
}