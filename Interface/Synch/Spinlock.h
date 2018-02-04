/**
 * File: Spinlock.h
 * Module: ModuleFramework (kernel.silcos.mdfrwk)
 * Module: ExecutionManager (kernel.silcos.exemgr)
 *
 * Summary:
 * Basic locking mechanism given here. Is implemented in multiple module for better
 * memory caching.
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef MDFRWK_EXEMGR_SYNCH_SPIN_LOCK_H__
#define MDFRWK_EXEMGR_SYNCH_SPIN_LOCK_H__

typedef volatile unsigned int Spinlock;

extern "C" void SpinLock(volatile Spinlock *);
extern "C" void SpinUnlock(volatile Spinlock *);

static inline bool TestLock(volatile Spinlock *sl)
{
	return __sync_bool_compare_and_swap(sl, 0 ,1);
}

class Lockable
{
public:
	inline void lock()
	{
		SpinLock(&__lockstatus);
	}

	inline void unlock()
	{
		SpinUnlock(&__lockstatus);
	}
private:
	Spinlock __lockstatus;
};

#endif/* Synch/Spinlock.h */
