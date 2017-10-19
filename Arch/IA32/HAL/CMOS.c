/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * File: CMOS.c
 *
 * Summary: This file contains the code for interfacing with the CMOS registers
 * and the RTC.
 *
 * @Version 1
 * @Since Circuit 2.03
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
 
 #include <IA32/IO.h>
 
 UBYTE ReadCMOSRegister(ULONG registerOffset){
	UBYTE regValue;
	WritePort(0x70, registerOffset | (1 << 7));
	regValue = ReadPort(0x71);
	return (regValue);
}

VOID WriteCMOSRegister(ULONG registerOffset, UBYTE value){
	WritePort(0x70, registerOffset | (1 << 7));
	WritePort(0x71, value);
}
