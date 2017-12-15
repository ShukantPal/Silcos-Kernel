/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef X86_PORT_IO_H
#define X86_PORT_IO_H

#include <TYPE.h>

import_asm unsigned char ReadPort(unsigned short portNo);

import_asm void WritePort(unsigned short portNo, unsigned char byte);

export_asm UBYTE ReadCMOSRegister(unsigned long cmosRegisterIndex);

export_asm void WriteCMOSRegister(unsigned long cmosRegisterIndex, UBYTE cmosRegisterValue);

#endif /* IO.h */
