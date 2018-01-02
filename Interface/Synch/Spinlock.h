/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef SYNCH_SPIN_LOCK_H
#define SYNCH_SPIN_LOCK_H

typedef volatile unsigned int Spinlock;
typedef volatile unsigned int SPIN_LOCK;

import_asm void SpinLock(volatile SPIN_LOCK *);
import_asm void SpinUnlock(volatile SPIN_LOCK *);

static inline bool TestLock(volatile Spinlock *sl)
{
	return __sync_bool_compare_and_swap(sl, 0 ,1);
}

#endif /* Synch/Spinlock.h */
