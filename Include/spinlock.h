#ifndef __SPINLOCK_H_INCLUDED__
#define __SPINLOCK_H_INCLUDED__

#include <utils.h>
#include <kthread.h>

typedef uint64_t spinlock_t;

void spinlock_lock_busy(spinlock_t* lock);
void spinlock_lock_yield(spinlock_t* lock);
inline void spinlock_lock(spinlock_t* lock){
    if(kthread_is_initialized()){
        spinlock_lock_yield(lock);
    }else{
        //no threads, so everything is atomic
        *lock = 1;
    }
}

void spinlock_unlock(spinlock_t* lock);
bool spinlock_trylock(spinlock_t* lock);

#endif