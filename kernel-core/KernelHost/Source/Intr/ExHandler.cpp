/**
 * Copyright (C) 2017 - Shukant Pal
 */

#include <IA32/Processor.h>
#include <Debugging.h>
#include <Types.h>

#ifdef x86 /* x86 -------------------------- x86 */

extern U32 regInfo;

export_asm void HandleDF(U32 zero) {
	DbgLine("Double Fault - System Down");
	//asm volatile("cli; hlt;");
}

export_asm void HandleIT(U32 v) {
	DbgLine("Internal Failure - ");
	DbgLine("Details - x86 : TSS segment failed to load."); 
	DbgLine("Recovery - Try to reboot this device.");
	DbgLine("Exiting - CircuitX");
	asm volatile("cli; hlt;");
}

export_asm void HandleSNP(U32 s) {
	DbgLine("Internal Failure - ");
	DbgLine("Details - x86 : Segment Not Present - GDT Failure");
	DbgLine("Recovery - Try to reboot this device");
	DbgLine("Exiting - CircuitX");
	asm volatile("cli; hlt;");
}

export_asm void HandleGPF(U32 e) {
	Dbg("General Protection Fault - Error Code ");
	DbgInt(e);
	Dbg(", PROCESSOR_ID: ");
	DbgInt(PROCESSOR_ID);
	DbgLine("");
	asm volatile("cli; hlt;");
}

export_asm void HandlePF(U32 errorCode) {
	//if(FixPF(regInfo))
	//	return;

	bool bitP = errorCode & 1;
	bool bitW = errorCode & (1 << 1);
	bool bitU = errorCode & (1 << 2);
	bool bitR = errorCode & (1 << 3);
	bool bitI = errorCode & (1 << 4);

	Dbg("Page Fault: ");

	Dbg(" Processor: ");
	DbgInt(PROCESSOR_ID);

	if(bitP) Dbg(" Page Protection Fault, ");
	else Dbg(" Page Not Present, ");

	if(bitW) Dbg("Write, ");
	else Dbg("Read, ");

	if(bitU) Dbg("User, ");
	else Dbg("Kernel ");

	if(bitR) Dbg("Reserved, ");

	if(bitI) Dbg("IFetch ");

	//DbgInt(regInfo / 1024);
	Dbg(" KB Address, ");

	asm volatile("cli; hlt;");
}

#endif /* x86 ----------------------------- x86 */
