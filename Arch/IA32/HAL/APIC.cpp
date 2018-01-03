/**
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
 */

#define NAMESPACE_IA32_IDT

#include <IA32/APIC.h>
#include <IA32/IDT.h>
#include <KERNEL.h>

unsigned int VAPICBase;
extern IDTEntry defaultIDT[256];
import_asm void TimerUpdate();

import_asm void TimerWait(U32 t);

/**
 * Function: KiClockRespond
 *
 * Summary:
 * Handles the tick related to clock on ticked kernel configurations.
 */
import_asm void KiClockRespond(void);

void SetupAPICTimer(){
	MapHandler(0x20, (unsigned int) &TimerUpdate, defaultIDT);
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
void SetupTick(void)
{
	extern U32 BSP_ID;
	if(RdApicRegister(APIC_REGISTER_ID) == BSP_ID)
		MapHandler(0x20, (unsigned int) &KiClockRespond, (IDTEntry*) &defaultIDT);
	WtApicRegister(APIC_REGISTER_SIVR, 0xFE | (1 << 8));
	WtApicRegister(APIC_REGISTER_TIMER_DCR, 32);
	WtApicRegister(APIC_REGISTER_LVT_TR, (1 << 17) | 0x20);
	WtApicRegister(APIC_REGISTER_TIMER_ICR, 1 << 28);
}

/**
 * Function: APIC::triggerIPI
 *
 * Summary:
 * Triggers a inter-processor interrupt from the running local APIC using the
 * default values (fixed-delivery, physical-destination, level-deassert, and
 * edge-trigger with no shorthand).
 *
 * Args:
 * U32 apicId - apic-id for the destination processor
 * U8 vect - vector number for the interrupt handler
 *
 * Author: Shukant Pal
 */
void APIC::triggerIPI(U32 apicId, U8 vect)
{
	if(!x2APICModeEnabled)
	{
		while
		(
			((xAPICDriver::read((unsigned long) APIC_REGISTER_ICR_LOW) >> 12) & 1) == DeliveryStatus::SendPending
		);

		ICRHigh hreg = {
				.destField = apicId
		};

		ICRLow lreg;
		lreg.vectorNo = vect;

		xAPICDriver::write(APIC_REGISTER_ICR_HIGH, hreg.value);
		xAPICDriver::write(APIC_REGISTER_ICR_LOW, lreg.value);
	}
	else
		Dbg("x2apic not impl sorry , expect a crash!!!");
}

/**
 * Function: APIC::wakeupSequence
 *
 * Summary:
 * Executes the INIT-SIPI sequence on behalf of kernel smp-init.
 *
 * Args:
 * U32 apicId - apic-id of the application-processor to wakeup
 * U8 pvect - page index for the trampoline
 *
 * Author: Shukant Pal
 */
void APIC::wakeupSequence(U32 apicId, U8 pvect)
{
	if(!x2APICModeEnabled)
	{
		ICRHigh hreg = {
				.destField = apicId
		};

		ICRLow lreg(DeliveryMode::INIT, Level::Deassert, TriggerMode::Edge);

		xAPICDriver::write(APIC_REGISTER_ICR_HIGH, hreg.value);
		xAPICDriver::write(APIC_REGISTER_ICR_LOW, lreg.value);

		lreg.vectorNo = pvect;
		lreg.delvMode = DeliveryMode::StartUp;

		Dbg("APBoot: Wakeup sequence following...");

		xAPICDriver::write(APIC_REGISTER_ICR_HIGH, hreg.value);
		xAPICDriver::write(APIC_REGISTER_ICR_LOW, lreg.value);

		DbgLine("sent");
	}
	else
		Dbg("x2apic - - not imple -- no statrup epecet crash shortly!!!");
}

