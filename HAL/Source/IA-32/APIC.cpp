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

extern "C" void try_spur_int__() {
	WriteLine("Hello");
}

/**
 * Function: KiClockRespond
 *
 * Summary:
 * Handles the tick related to clock on ticked kernel configurations.
 */
import_asm void KiClockRespond(void);

/*
 * Enables the early-boot timer which ticks at regular intervals. These
 * intervals are not fixed, but are usable to wait for atleast some time.
 *
 * @version 1.0
 * @since Silcos 2.05
 * @author Shukant Pal
 */
void APIC::setupEarlyTimer(void)
{
	if(!x2APICModeEnabled)
	{
		xAPICDriver::write(ErrorStatus, 0);

		MapHandler(0x20, (unsigned int) &TimerUpdate, defaultIDT);
		xAPICDriver::write(SpurIntrVector, 0xFE | (1 << 8));
		xAPICDriver::write(DivideConfig, 128);
		xAPICDriver::write(LVT_Timer, (1 << 17) | 0x20);
		xAPICDriver::write(InitialCount, 1 << 28);
	}
}

/*
 * Enables scheduler-ticks that fire at regular intervals. This is used in
 * non-tickless kernel configurations. When the boot-strap processor calls
 * this, then the KiClockRespond irq is also mapped.
 *
 * @version 1.0
 * @since Silcos 2.05
 * @author Shukant Pal
 */
void APIC::setupScheduleTicks(void)
{
	extern U32 BSP_ID;
	if(xAPICDriver::read(Id) == BSP_ID)
		MapHandler(0x20, (unsigned int) &KiClockRespond,
						(IDTEntry*) &defaultIDT);
	xAPICDriver::write(SpurIntrVector, 0xFE | (1 << 8));
	xAPICDriver::write(DivideConfig, 32);
	xAPICDriver::write(LVT_Timer, (1 << 17) | 0x20);
	xAPICDriver::write(InitialCount, 1 << 28);
}

/**
 * Triggers a inter-processor interrupt from the running local APIC using the
 * default values (fixed-delivery, physical-destination, level-deassert, and
 * edge-trigger with no shorthand).
 *
 * @arg apicId - apic-id for the destination processor
 * @arg vect - vector number for the interrupt handler
 * @version 1.0
 * @since Silcos 2.05
 * @author Shukant Pal
 */
void APIC::triggerIPI(U32 apicId, U8 vect)
{
	if(!x2APICModeEnabled)
	{
		while((xAPICDriver::read((unsigned long) ICR_Low) >> 12) & 1
				== DeliveryStatus::SendPending);

		ICRHigh hreg = { .destField = apicId };
		ICRLow lreg;

		lreg.vectorNo = vect;

		xAPICDriver::write(ICR_High, hreg.value);
		xAPICDriver::write(ICR_Low, lreg.value);
	}
	else
		Dbg("x2apic not impl sorry , expect a crash!!!");
}

/*
 * Sends a startup interrupt to the specified local-apic.
 *:
 * @arg apicId - apic-id of the application-processor to wakeup
 * @arg pvect - page index for the trampoline
 * @version 1.0
 * @since Silcos 2.05
 * @author Shukant Pal
 */
void APIC::wakeupSequence(U32 apicId, U8 pvect)
{
	if(!x2APICModeEnabled)
	{
		ICRHigh hreg = { .destField = apicId };
		ICRLow lreg(INIT, Deassert, Edge);

		xAPICDriver::write(ICR_High, hreg.value);
		xAPICDriver::write(ICR_Low, lreg.value);

		lreg.vectorNo = pvect;
		lreg.delvMode = StartUp;

		xAPICDriver::write(ICR_High, hreg.value);
		xAPICDriver::write(ICR_Low, lreg.value);
	}
	else
		Dbg("x2apic - - not imple -- no statrup epecet crash shortly!!!");
}
