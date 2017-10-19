/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef X86_PORT_IO_H
#define X86_PORT_IO_H

#include <TYPE.h>

extern
UCHAR ReadPort(
	USHORT portNo
);

extern
VOID WritePort(
	USHORT portNo,
	UCHAR byte
);

/* Read CMOS */
UBYTE ReadCMOSRegister(
	ULONG cmosRegisterIndex
);

/* Write CMOS */
VOID WriteCMOSRegister(
	ULONG cmosRegisterIndex,
	UBYTE cmosRegisterValue
);

#endif /* IO.h */
