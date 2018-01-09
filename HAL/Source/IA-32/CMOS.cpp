 #include <IA32/IO.h>
 
 unsigned char ReadCMOSRegister(unsigned long registerOffset)
 {
	UBYTE regValue;
	WritePort(0x70, registerOffset | (1 << 7));
	regValue = ReadPort(0x71);
	return (regValue);
}

void WriteCMOSRegister(unsigned long registerOffset, unsigned char value){
	WritePort(0x70, registerOffset | (1 << 7));
	WritePort(0x71, value);
}
