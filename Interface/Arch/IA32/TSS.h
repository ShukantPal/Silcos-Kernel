/**
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef X86_TSS_H
#define X86_TSS_H

#ifdef NAMESPACE_IA32_TSS

#include <TYPE.h>

typedef
struct _TSS
{
   unsigned int PreviousTSS;
   unsigned int ESP_0;
   unsigned int SS_0;
   unsigned int ESP_1;
   unsigned int SS_1;
   unsigned int ESP_2;
   unsigned int SS_2;
   unsigned int CR3;
   unsigned int EIP;
   unsigned int EFLAGS;
   unsigned int EAX;
   unsigned int ECX;
   unsigned int EDX;
   unsigned int EBX;
   unsigned int ESP;
   unsigned int EBP;
   unsigned int ESI;
   unsigned int EDI;
   unsigned int ES;         
   unsigned int CS;        
   unsigned int SS;        
   unsigned int DS;        
   unsigned int FS;       
   unsigned int GS;         
   unsigned int LDT;      
   unsigned short TRAP;
   unsigned short IOMAP_BASE;
} __attribute__((packed)) TSS;

#endif

#endif /* TSS.h */
