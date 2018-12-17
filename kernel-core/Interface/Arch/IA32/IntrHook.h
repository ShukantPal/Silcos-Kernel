/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef IA32_INTR_HOOK_H
#define IA32_INTR_HOOK_H

#include <TYPE.h>

import_asm void DoubleFault();
import_asm void InvalidTSS();
import_asm void SegmentNotPresent();
import_asm void GeneralProtectionFault();
import_asm void PageFault();

#endif /* IA32/IntrHook.h */
