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
   UINT PreviousTSS;
   UINT ESP_0;
   UINT SS_0;
   UINT ESP_1;
   UINT SS_1;
   UINT ESP_2;
   UINT SS_2;
   UINT CR3;
   UINT EIP;
   UINT EFLAGS;
   UINT EAX;
   UINT ECX;
   UINT EDX;
   UINT EBX;
   UINT ESP;
   UINT EBP;
   UINT ESI;
   UINT EDI;
   UINT ES;         
   UINT CS;        
   UINT SS;        
   UINT DS;        
   UINT FS;       
   UINT GS;         
   UINT LDT;      
   USHORT TRAP;
   USHORT IOMAP_BASE;
} __attribute__((packed)) TSS;

#endif

#endif /* TSS.h */
