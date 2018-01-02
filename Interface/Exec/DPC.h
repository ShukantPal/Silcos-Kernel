/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: DPC.h
 *
 * Summary: This file contains the interface for deferred procedure calls, or executing ISRs at a later
 * time.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
#ifndef EXEC_DPC_H
#define EXEC_DPC_H

#include <Exec/KTask.h>

typedef
struct {
	KRUNNABLE Executor;
} DPC;

#endif/* Exec/DPC.h */
