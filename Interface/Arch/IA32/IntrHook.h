/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef IA32_INTR_HOOK_H
#define IA32_INTR_HOOK_H

#include <TYPE.h>

import_asm VOID DoubleFault();
import_asm VOID InvalidTSS();
import_asm VOID SegmentNotPresent();
import_asm VOID GeneralProtectionFault();
import_asm VOID PageFault();

#endif /* IA32/IntrHook.h */
