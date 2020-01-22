#ifndef __SPINLOCK_H_INCLUDED__
#define __SPINLOCK_H_INCLUDED__

#include <utils.h>

typedef uint64_t spinlock_t;


void spinlock_lock(spinlock_t* lock);
void spinlock_unlock(spinlock_t* lock);

#endif