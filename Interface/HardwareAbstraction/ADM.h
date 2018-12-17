/**
 * Copyright (C) 2017 - Shukant Pal
 *
 * ADM (or Automatic Data Manager) is a platform-dependent data manager
 * for boot-time and permanent data. It keeps the locations of data at statically
 * allocated positions, and is manually maintained by the developers. You cannot
 * dynamically allocated memory from this, but you must define the symbol
 * for your data.
 */
#ifndef HAL_ADM_H
#define HAL_ADM_H

#include <Multiboot2.h>

extern MULTIBOOT_TAG *tagTable;

#endif /* HAL/ADM.h */
