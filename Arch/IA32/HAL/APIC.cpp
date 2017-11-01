/*=++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: APIC.c
 *
 * Summary: This file contains the code to setup scheduler ticks, trigger IPIs and much more!
 *
 * Functions:
 * SetupTick() - This function writes to the APIC timer registers and enables kernel timing.
 * TriggerIPI() - This function contains the code to trigger a IPI
 * TriggerDelay() - This function is used to wait (no-scheduling) for some time.
 *
 * Copyright (C) 2017 - Shukant Pal
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=*/

#define NAMESPACE_IA32_IDT

#include <IA32/APIC.h>
#include <IA32/IDT.h>

UINT VAPICBase;
extern IDT defaultIDT[256];
import_asm void TimerUpdate();

/**
 * Function: KiClockRespond
 *
 * Summary:
 * Handles the tick related to clock on ticked kernel configurations.
 */
import_asm void KiClockRespond(VOID);

VOID SetupAPICTimer(){
	MapHandler(0x20, (UINT) &TimerUpdate, defaultIDT);
	WtApicRegister(APIC_REGISTER_SIVR, 0xFE | (1 << 8));
	WtApicRegister(APIC_REGISTER_TIMER_DCR, 128);
	WtApicRegister(APIC_REGISTER_LVT_TR, (1 << 17) | 0x20);
	WtApicRegister(APIC_REGISTER_TIMER_ICR, 1 << 20);
}

/**
 * Function: SetupTick()
 *
 * Summary: This function is used for initializing the APIC timer to give the scheduler
 * ticks in microsecond precision. It maps the KiClockRespond handler to the IDT, if the
 * BSP is calling, or else only APIC timer is initialized.
 *
 * @Version 1
 * @Since Circuit 2.03
 */
VOID SetupTick(VOID){
	extern U32 BSP_ID;
	if(RdApicRegister(APIC_REGISTER_ID) == BSP_ID)
		MapHandler(0x20, (UINT) &KiClockRespond, (IDT *) &defaultIDT);
	WtApicRegister(APIC_REGISTER_SIVR, 0xFE | (1 << 8));
	WtApicRegister(APIC_REGISTER_TIMER_DCR, 32);
	WtApicRegister(APIC_REGISTER_LVT_TR, (1 << 17) | 0x20);
	WtApicRegister(APIC_REGISTER_TIMER_ICR, 1 << 24);
}

ULONG TriggerIPI(U32 destination, U32 controlSet){
	WtApicRegister(APIC_REGISTER_ICR_HIGH, (destination << 24));
	WtApicRegister(APIC_REGISTER_ICR_LOW, controlSet);
}

VOID TriggerManualIPI(U32 lowerICR, U32 higherICR){
	WtApicRegister(APIC_REGISTER_ICR_HIGH, higherICR | RdApicRegister(APIC_REGISTER_ICR_HIGH));
	WtApicRegister(APIC_REGISTER_ICR_LOW, lowerICR | RdApicRegister(APIC_REGISTER_ICR_LOW));
}

VOID SendIPI(APIC_ID apicId, APIC_VECTOR vectorIndex){
	WtApicRegister(APIC_REGISTER_ICR_HIGH, ((U8) apicId << 24) | RdApicRegister(APIC_REGISTER_ICR_HIGH));
	WtApicRegister(APIC_REGISTER_ICR_LOW, vectorIndex | LAPIC_DM_FIXED | RdApicRegister(APIC_REGISTER_ICR_LOW));
}
