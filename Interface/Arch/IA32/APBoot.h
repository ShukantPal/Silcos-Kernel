/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: APBoot.h
 *
 * Summary: This file contains the ADM-plugin for AP booting process. The 
 * APBootSequence and APBoot symbols are also exported, to setup the processors
 * during the secondary boot processor. It also defines a few other parameters.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/
#ifndef IA32_APBOOT_H
#define IA32_APBOOT_H

#include <TYPE.h>

#define AP_TRAMPOLINE	(9*64*1024)

#define ALLOCATE_TRAMPOLINE() AP_TRAMPOLINE

#define AP_STATUS_INIT	0x2
#define AP_STATUS_BOOTING 0xBB
#define AP_STATUS_ERR 	0xFF
#define AP_STATUS_DONE	0x4

extern VOID APBoot
();/* Only a exported symbol */

extern VOID APBootSequenceStart
();/* Only a exported symbol */

extern ULONG APBootSequenceEnd
();/* Only a exported symbol */

extern VOID apSetupInfo
();/* Only a exported symbol */

extern VOID APInvokeMain32
();/* Only a exported symbol */

#ifdef CONFIG_APBOOT
	#define ADM_APBOOT
#endif


#endif/* IA32/APBoot.h */
