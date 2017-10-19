/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef IA32_INTR_HOOK_H
#define IA32_INTR_HOOK_H

#include <TYPE.h>

extern VOID DoubleFault();
extern VOID InvalidTSS();
extern VOID SegmentNotPresent();
extern VOID GeneralProtectionFault();
extern VOID PageFault();

#endif /* IA32/IntrHook.h */
