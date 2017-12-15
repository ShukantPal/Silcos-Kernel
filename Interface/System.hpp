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
	char clientName[10];
	unsigned long accessIdx;
	Void *memoryBuffer;
};

// Raw Ideas just yet
class NotificationManager
{
public:
	NotificationManager(char &nName);
};

// For System Integration
class SystemBoard
{
public:
	static unsigned long kernelVersion;
	static unsigned long kernelType;
	static unsigned long kernelHardConfig;
	static char *kernelName;

	static unsigned long getKernelPage();
	static unsigned long retKernelPage();

	static void getMemoryFrame();
	static void retMemoryFrame();

	static Void *getModule(char &moduleName);

};

}

#endif /* System.hpp */
