#ifndef __UTILS_H_INCLUDED__
#define __UTILS_H_INCLUDED__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>

#define KERNEL_MAPPING_BASE 0xffff800000000000ULL
#define KB * 1024
#define MB * 1024 * 1024
#define GB * 1024 * 1024 * 1024
#define KERNEL_INITIAL_MAPPING_SIZE 64 MB
#define KERNEL_INITIAL_MAPPING_END (KERNEL_INITIAL_MAPPING_SIZE + KERNEL_MAPPING_BASE)
#define ARRSIZE(arr) (sizeof(arr)/sizeof(*arr))
#define UNUSED __attribute__((unused))

typedef int64_t ssize_t;

size_t strlen(char* str);

inline uint64_t up_align(uint64_t value, uint64_t align){
    return ((value + align - 1) / align) * align;
}

inline uint64_t down_align(uint64_t value, uint64_t align){
    return ((value) / align) * align;
}

inline void longmemset(uint64_t* begin, uint64_t count, uint64_t val){
    for(uint64_t i = 0; i < count; ++i){
        begin[i] = val;
    }
}

void memset(char* begin, uint64_t count, char val);
void memcpy(void* dst, void* src, size_t count);
int strcmp(char* str1, char* str2);

//Used for fast bitmap scanning. Source is taken from
//https://www.chessprogramming.org/BitScan#Bsf.2FBsr_x86-64_Timings
inline int bit_scan_forward(uint64_t x) {
   asm ("bsfq %0, %0" : "=r" (x) : "0" (x));
   return (int) x;
}

inline uint64_t fetch_and_add(uint64_t* p, uint64_t val){
    __asm__ volatile("lock; xaddq %0, %1"
    : "+r" (val), "+m"(p)
    :
    : "memory");
}

void panic(char* error);

#endif