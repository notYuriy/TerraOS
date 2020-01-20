#ifndef __SPINLOCK_H_INCLUDED__
#define __SPINLOCK_H_INCLUDED__

#include <utils.h>

typedef uint64_t spinlock_t;

extern void spinlock_lock(spinlock_t* lock);
extern void spinlock_unlock(spinlock_t* lock);

#endif