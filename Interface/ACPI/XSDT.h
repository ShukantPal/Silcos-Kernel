/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef CONFIG_XSDT_H
#define CONFIG_XSDT_H

#include "SDTHeader.h"

typedef
struct _XSDT {
	SDT_HEADER XRootHeader;
	U64 ConfigurationTables[];
} XSDT;

#endif /* Config/XSDT.h */
