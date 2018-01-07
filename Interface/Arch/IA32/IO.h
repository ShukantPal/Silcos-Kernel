/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef X86_PORT_IO_H
#define X86_PORT_IO_H

#include <KERNEL.h>

import_asm unsigned char ReadPort(unsigned short portNo) kxhide;
import_asm void WritePort(unsigned short portNo, unsigned char byte) kxhide;

export_asm UBYTE ReadCMOSRegister(unsigned long cmosRegisterIndex) kxhide;
export_asm void WriteCMOSRegister(unsigned long cmosRegisterIndex, UBYTE cmosRegisterValue) kxhide;

#endif /* IO.h */
