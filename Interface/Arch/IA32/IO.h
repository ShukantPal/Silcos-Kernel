/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef X86_PORT_IO_H
#define X86_PORT_IO_H

#include <TYPE.h>

import_asm UCHAR ReadPort(USHORT portNo);

import_asm void WritePort(USHORT portNo, UCHAR byte);

export_asm UBYTE ReadCMOSRegister(ULONG cmosRegisterIndex);

export_asm void WriteCMOSRegister(ULONG cmosRegisterIndex, UBYTE cmosRegisterValue);

#endif /* IO.h */
