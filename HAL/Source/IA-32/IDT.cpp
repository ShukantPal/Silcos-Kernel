/**
 * @file IDT.cpp
 * -------------------------------------------------------------------
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#define NAMESPACE_IA32_IDT

#include "IRQCallbacks.h"
#include <IA32/IDT.h>
#include <IA32/IntrHook.h>
#include <IA32/IO.h>
#include <IA32/Processor.h>
#include <Utils/Memory.h>

import_asm void ExecuteLIDT(IDTPointer *);
import_asm void Spurious();
import_asm void KiClockRespond(void);
import_asm void RR_BalanceRunqueue(void);
import_asm void Executable_ProcessorBinding_IPIRequest_Invoker();
import_asm void hpetTimer();

IDTEntry defaultIDT[256] __attribute__((aligned(8)));
IDTPointer defaultIDTPointer;

decl_c void MapHandler(unsigned short handlerNo, unsigned int handlerAddress,
				IDTEntry *pIDT)
{
	pIDT[handlerNo].offLow = (unsigned short)(handlerAddress & 0xFFFF);
	pIDT[handlerNo].sel = 0x08;
	pIDT[handlerNo].rfield = 0;
	pIDT[handlerNo].gateType = INTERRUPT_GATE_386;
	pIDT[handlerNo].storageSegment = 0;
	pIDT[handlerNo].dpl = 0;
	pIDT[handlerNo].present = 1;
	pIDT[handlerNo].offHigh = (unsigned short)(handlerAddress >> 16);
}

static inline void waitIO(void) {
	/* Taken from OSDev Wiki. */
	asm volatile("jmp 1f\n\t"
			"1:jmp 2f\n\t"
			"2:");
}

/**
 * Disables the XT-PIC controller to prevent conflicts during initialization
 * of the APIC.
 */
decl_c void DisablePIC(void)
{
	WritePort(0xA0, 0x11);
	waitIO();
	WritePort(0x20, 0x11);
	waitIO();
	WritePort(0xA1, 0x30);
	waitIO();
	WritePort(0x21, 0x40);
	waitIO();
	WritePort(0xA1, 4);
	waitIO();
	WritePort(0x21, 2);
	waitIO();
	WritePort(0xA1, 0x1);
	waitIO();
	WritePort(0x21, 0x1);
	waitIO();
	WritePort(0xA1, 0xFF);
	WritePort(0x21, 0xFF);
}

///
/// Maps all the irq-handlers to the IDT before kernel initialization. This
/// includes the exception handlers, device IRQs, syscall entries, traps, etc.
/// and after calling MapIDT().
///
/// @version 1.0
/// @since Silcos 2.01
/// @author Shukant Pal
///
decl_c void MapIDT()
{
	IDTEntry *pIDT = defaultIDT;
	IDTPointer *pIDTPointer = &(defaultIDTPointer);

	MapHandler(0x8, (unsigned int) &DoubleFault, pIDT);
	MapHandler(0xA, (unsigned int) &InvalidTSS, pIDT);
	MapHandler(0xB, (unsigned int) &SegmentNotPresent, pIDT);
	MapHandler(0xD, (unsigned int) &GeneralProtectionFault, pIDT);
	MapHandler(0xE, (unsigned int) &PageFault, pIDT);
	MapHandler(0xFD, (unsigned int) &Executable_ProcessorBinding_IPIRequest_Invoker, pIDT);
	MapHandler(0xFE, (unsigned int) &Spurious, pIDT);
	MapHandler(0x20, (unsigned int) &KiClockRespond, pIDT);
	MapHandler(0x21, (unsigned int) &Spurious, pIDT);

	DbgInt(*(unsigned short*)&pIDT[0x21]); Dbg(" ");

	// Here, we are mapping the 160 device-IRQs in the range 32-191
	// inclusive. As there was "no" other way to map them, we have just
	// call MapHandler 160 times (using a pointer array was wasteful)
#define map_irqClbk(vt) MapHandler(vt, (unsigned int) &__irqCallback##vt, pIDT);
{
	//map_irqClbk(32); map_irqClbk(33); map_irqClbk(34); map_irqClbk(35);
	map_irqClbk(36); map_irqClbk(37); map_irqClbk(38); map_irqClbk(39);
	map_irqClbk(40); map_irqClbk(41); map_irqClbk(42); map_irqClbk(43);
	map_irqClbk(44); map_irqClbk(45); map_irqClbk(46); map_irqClbk(47);
	map_irqClbk(48); map_irqClbk(49); map_irqClbk(50); map_irqClbk(51);
	map_irqClbk(52); map_irqClbk(53); map_irqClbk(54); map_irqClbk(55);
	map_irqClbk(56); map_irqClbk(57); map_irqClbk(58); map_irqClbk(59);
	map_irqClbk(60); map_irqClbk(61); map_irqClbk(62); map_irqClbk(63);
	map_irqClbk(64); map_irqClbk(65); map_irqClbk(66); map_irqClbk(67);
	map_irqClbk(68); map_irqClbk(69); map_irqClbk(70); map_irqClbk(71);
	map_irqClbk(72); map_irqClbk(73); map_irqClbk(74); map_irqClbk(75);
	map_irqClbk(76); map_irqClbk(77); map_irqClbk(78); map_irqClbk(79);
	map_irqClbk(80); map_irqClbk(81); map_irqClbk(82); map_irqClbk(83);
	map_irqClbk(84); map_irqClbk(85); map_irqClbk(86); map_irqClbk(87);
	map_irqClbk(88); map_irqClbk(89); map_irqClbk(90); map_irqClbk(91);
	map_irqClbk(92); map_irqClbk(93); map_irqClbk(94); map_irqClbk(95);
	map_irqClbk(96); map_irqClbk(97); map_irqClbk(98); map_irqClbk(99);
	map_irqClbk(100); map_irqClbk(101); map_irqClbk(102);
	map_irqClbk(103); map_irqClbk(104); map_irqClbk(105);
	map_irqClbk(106); map_irqClbk(107); map_irqClbk(108);
	map_irqClbk(109); map_irqClbk(110); map_irqClbk(111);
	map_irqClbk(112); map_irqClbk(113); map_irqClbk(114);
	map_irqClbk(115); map_irqClbk(116); map_irqClbk(117);
	map_irqClbk(118); map_irqClbk(119);
	map_irqClbk(120); map_irqClbk(121); map_irqClbk(122);
	map_irqClbk(123); map_irqClbk(124); map_irqClbk(125);
	map_irqClbk(126); map_irqClbk(127); map_irqClbk(128);
	map_irqClbk(129); map_irqClbk(130); map_irqClbk(131);
	map_irqClbk(132); map_irqClbk(133); map_irqClbk(134);
	map_irqClbk(135); map_irqClbk(136); map_irqClbk(137);
	map_irqClbk(138); map_irqClbk(139); map_irqClbk(140);
	map_irqClbk(141); map_irqClbk(142); map_irqClbk(143);
	map_irqClbk(144); map_irqClbk(145); map_irqClbk(146);
	map_irqClbk(147); map_irqClbk(148); map_irqClbk(149);
	map_irqClbk(150); map_irqClbk(151); map_irqClbk(152);
	map_irqClbk(153); map_irqClbk(154); map_irqClbk(155);
	map_irqClbk(156); map_irqClbk(157); map_irqClbk(158);
	map_irqClbk(159); map_irqClbk(160); map_irqClbk(161);
	map_irqClbk(162); map_irqClbk(163); map_irqClbk(164);
	map_irqClbk(165); map_irqClbk(166); map_irqClbk(167);
	map_irqClbk(168); map_irqClbk(169); map_irqClbk(170);
	map_irqClbk(171); map_irqClbk(172); map_irqClbk(173);
	map_irqClbk(174); map_irqClbk(175); map_irqClbk(176);
	map_irqClbk(177); map_irqClbk(178); map_irqClbk(179);
	map_irqClbk(180); map_irqClbk(181); map_irqClbk(182);
	map_irqClbk(183); map_irqClbk(184); map_irqClbk(185);
	map_irqClbk(186); map_irqClbk(187); map_irqClbk(188);
	map_irqClbk(189);
	map_irqClbk(190); map_irqClbk(191);
}
	pIDTPointer->Limit = (sizeof(IDTEntry) * 256) - 1;
	pIDTPointer->Base = (unsigned int) pIDT;
}

/* Part of processor initialization series */
decl_c void SetupIDT()
{
	IDTPointer *pIDTPointer = &(defaultIDTPointer);
	ExecuteLIDT(pIDTPointer);
}
