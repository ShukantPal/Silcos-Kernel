/**
 * File: System.hpp
 *
 * Summary:
 * System.hpp allows clients to get a C++ interface to system status & management
 * functionalities.
 *
 * Copyright (C) 2017 - Shukant Pal
 */

#ifndef _INTERFACE_SYSTEM_HPP_
#define _INTERFACE_SYSTEM_HPP_

#include "TYPE.h"

namespace System
{

// Raw Ideas
struct NotficationClient
{
	CHAR clientName[10];
	ULONG accessIdx;
	Void *memoryBuffer;
};

// Raw Ideas just yet
class NotificationManager
{
public:
	NotificationManager(CHAR &nName);
};

// For System Integration
class SystemBoard
{
public:
	static ULONG kernelVersion;
	static ULONG kernelType;
	static ULONG kernelHardConfig;
	static CHAR *kernelName;

	static ULONG getKernelPage();
	static ULONG retKernelPage();

	static VOID getMemoryFrame();
	static VOID retMemoryFrame();

	static Void *getModule(CHAR &moduleName);

};

}

#endif /* System.hpp */
