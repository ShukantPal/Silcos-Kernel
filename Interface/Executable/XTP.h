/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef EXEC_XTP_H
#define EXEC_XTP_H

#include <Types.h>

typedef
struct RuntimeInfo {
	U8 Privelege;
	U8 Priority;
	U16 Flags;
} __attribute__((packed)) RUNTIME_INFO;

#endif /* Exec/XTP.h */
