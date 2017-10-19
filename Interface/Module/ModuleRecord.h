#ifndef __MODULE_RECORD_H__
#define __MODULE_RECORD_H__

#include <Memory/KObjectManager.h>
#include "ELF.h"

// remember, KAF applications are not modules. They are loaded by KAF clients
typedef enum {
	KMT_KDF_DRIVER	= 1,// Kernel-mode Driver
	KMT_EXC_MODULE	= 2,// Executive Module
	KMT_KMCF_EXTEN	= 3,// KMCF Extension (Kmodule Communicator Framework)
	KMT_UNKNOWN	= 4// Unknown (restricted form)
} KM_TYPE;

typedef enum {
	KMB_ELF = 0
} KM_BI;

typedef
struct _KMOD_RECORD {
	CHAR Name[16];
	ULONG Version;
	KM_TYPE Type;
	KM_BI BinaryInterface;
	KMOD_ECACHE ECache;
} KMOD_RECORD;

extern OBINFO *tKMOD_RECORD;
extern LINKED_LIST LoadedKModules;

#endif
