# File: Makefile
#
# Makefile: [CircuitKernel, 2.03, CONFIG_BUILD, CONFIG_RUN]
# Run: %Compile-ONLY
# Run: q [qemu-system-*]
# Run: b [bochs]
#
# Copyright (C) - Shukant Pal

ArchDir = Arch/IA32

CC = g++ -std=c++1z# C Compiler
CFLAGS = -m32 -I"." -I"../Interface" -I"../Interface/Arch" -c \
-fvisibility=default -ffreestanding -nostdlib -nostdinc -Wall -Os -fPIC \
-fno-rtti -fno-exceptions -LHAL/Build/kernel.silcos.hal

AS = nasm # Assember
ASFLAGS = -f elf32 # Asm Flags

# Ifc - Header Directories
IfcKern = ../Interface/KERNEL.h ../Interface/TYPE.h
IfcArch = ../Interface/$(ArchDir)
IfcACPI = ../Interface/ACPI
IfcHAL = ../Interface/HAL
IfcRunnable = ../Interface/Executable
IfcMemory = ../Interface/Memory
IfcModule = ../Interface/Module
IfcUtil = ../Interface/Util

#
# C o m p i l a t i o n   U n i t s
#    ( O b j e c t   F i l e s)
#

COMPILE = Compile
SOURCE = Source

COM_ARCH = Compile/Arch/IA32
SRC_ARCH = Source/Arch/IA32

COM_INTR = Compile/Intr
SRC_INTR = Source/Intr

COM_KR = Compile/KernelRoutine
SRC_KR = Source/KernelRoutine

COM_MM = Compile/Memory
SRC_MM = Source/Memory

COM_MD = Compile/ModuleLoader
SRC_MD = Source/ModuleLoader

COM_SCHED = Compile/Scheduling
SRC_SCHED = Source/Scheduling

COM_TD = Compile/Thread
SRC_TD = Source/Thread

COM_UTIL = Compile/Util
SRC_UTIL = Source/Util

#IA32 Boot Service
InitObjects = $(COM_ARCH)/InitRuntime.o $(COM_ARCH)/86InitPaging.o \
$(COM_ARCH)/BlockPager.o $(COM_ARCH)/Main.o \
$(COM_ARCH)/Multiboot.o

# IA32 Objects 
ArchObjects = $(COM_ARCH)/FrameExtractor.o $(COM_ARCH)/Pager.o \
$(COM_ARCH)/TableManipulator.o

ConfigObjects = # ACPI tables

IntrObjects = $(COM_INTR)/ExHandler.o # Core exception & device handlers

KernelRoutineObjects = $(COM_KR)/Init.o

# Memory Subsystems + Algorithms
MemoryObjects = $(COM_MM)/BuddyAllocator.o $(COM_MM)/CacheRegister.o \
$(COM_MM)/KFrameManager.o $(COM_MM)/KMemoryManager.o  $(COM_MM)/KObjectManager.o \
$(COM_MM)/Structure.o $(COM_MM)/ZoneManager.o

ThreadObjects = $(COM_TD)/Thread.o

# Core Scheduler + Scheduler Classes/Rollers
SchedulerObjects = $(COM_SCHED)/Scheduler.o $(COM_SCHED)/ScheduleRoller.o \
$(COM_SCHED)/RoundRobin.o $(COM_SCHED)/RunqueueBalancer.o 

UtilObjects = $(COM_UTIL)/CircuitPrimitive.o $(COM_UTIL)/CircularList.o \
$(COM_UTIL)/Console.o $(COM_UTIL)/Debugger.o $(COM_UTIL)/LinkedList.o \
 $(COM_UTIL)/Stack.o

moduleLoaderObjects = $(COM_MD)/ElfManager.o $(COM_MD)/ElfAnalyzer.o \
$(COM_MD)/ModuleLoader.o $(COM_MD)/MSI.o $(COM_MD)/ElfLinker.o $(COM_MD)/RecordManager.o

# Header Dependencies

BuildChain : BuildInit BuildArch BuildKernelRoutines BuildIntr BuildMemory BuildScheduler BuildThread BuildUtil BuildModuleLoader Link

$(IfcHAL)/Processor.h: $(IfcMemory)/CacheRegister.h $(IfcUtil)/AVLTree.h $(IfcUtil)/CircularList.h

$(IfcModule)/Elf/ELF.h: $(IfcModule)/ModuleLoader.h
$(IfcModule)/Elf/ElfManager.hpp: $(IfcModule)/Elf/ELF.h

$(COM_ARCH)/86InitPaging.o: $(SRC_ARCH)/Paging/86InitPaging.asm
	$(AS) $(ASFLAGS) $(SRC_ARCH)/Paging/86InitPaging.asm -o $(COM_ARCH)/86InitPaging.o

$(COM_ARCH)/BlockPager.o: $(SRC_ARCH)/Paging/BlockPager.cpp
	$(CC) $(CFLAGS) $(SRC_ARCH)/Paging/BlockPager.cpp -o $(COM_ARCH)/BlockPager.o

$(COM_ARCH)/InitRuntime.o: $(SRC_ARCH)/Boot/InitRuntime.asm
	$(AS) $(ASFLAGS) $(SRC_ARCH)/Boot/InitRuntime.asm -o $(COM_ARCH)/InitRuntime.o

$(COM_ARCH)/Main.o: $(SRC_ARCH)/Boot/Main.cpp
	$(CC) $(CFLAGS) $(SRC_ARCH)/Boot/Main.cpp -o $(COM_ARCH)/Main.o

$(COM_ARCH)/Multiboot.o: $(SRC_ARCH)/Boot/Multiboot.cpp
	$(CC) $(CFLAGS) $(SRC_ARCH)/Boot/Multiboot.cpp -o $(COM_ARCH)/Multiboot.o

BuildInit : $(InitObjects)

$(COM_ARCH)/FrameExtractor.o: $(SRC_ARCH)/Paging/FrameExtractor.cpp
	$(CC) $(CFLAGS) $(SRC_ARCH)/Paging/FrameExtractor.cpp -o $(COM_ARCH)/FrameExtractor.o

$(COM_ARCH)/Pager.o: $(SRC_ARCH)/Paging/Pager.cpp
	$(CC) $(CFLAGS) $(SRC_ARCH)/Paging/Pager.cpp -o $(COM_ARCH)/Pager.o

$(COM_ARCH)/TableManipulator.o: $(SRC_ARCH)/Paging/TableManipulator.cpp
	$(CC) $(CFLAGS) $(SRC_ARCH)/Paging/TableManipulator.cpp -o $(COM_ARCH)/TableManipulator.o

BuildArch: $(ArchObjects)

$(COM_INTR)/ExHandler.o: $(SRC_INTR)/ExHandler.cpp
	$(CC) $(CFLAGS) $(SRC_INTR)/ExHandler.cpp -o $(COM_INTR)/ExHandler.o

BuildIntr: $(IntrObjects)

$(COM_KR)/Init.o: $(SRC_KR)/Init.cpp
	$(CC) $(CFLAGS) $(SRC_KR)/Init.cpp -o $(COM_KR)/Init.o

BuildKernelRoutines: $(KernelRoutineObjects)

$(COM_MM)/BuddyAllocator.o: $(IfcMemory)/BuddyManager.h $(IfcMemory)/Pager.h $(SRC_MM)/BuddyAllocator.cpp
	$(CC) $(CFLAGS) $(SRC_MM)/BuddyAllocator.cpp -o $(COM_MM)/BuddyAllocator.o

$(COM_MM)/CacheRegister.o: $(IfcHAL)/Processor.h $(IfcMemory)/CacheRegister.h $(SRC_MM)/CacheRegister.cpp	
	$(CC) $(CFLAGS) $(SRC_MM)/CacheRegister.cpp -o $(COM_MM)/CacheRegister.o

$(COM_MM)/KFrameManager.o: $(IfcKern) $(IfcHAL)/Processor.h $(IfcMemory)/KFrameManager.h $(SRC_MM)/KFrameManager.cpp
	$(CC) $(CFLAGS) $(SRC_MM)/KFrameManager.cpp -o $(COM_MM)/KFrameManager.o

$(COM_MM)/KMemoryManager.o: $(IfcHAL)/Processor.h $(IfcMemory)/KMemoryManager.h $(SRC_MM)/KMemoryManager.cpp
	$(CC) $(CFLAGS) $(SRC_MM)/KMemoryManager.cpp -o $(COM_MM)/KMemoryManager.o

$(COM_MM)/KObjectManager.o: $(IfcMemory)/KObjectManager.h $(IfcMemory)/MemoryTransfer.h $(IfcMemory)/KMemoryManager.h $(SRC_MM)/KObjectManager.cpp
	$(CC) $(CFLAGS) $(SRC_MM)/KObjectManager.cpp -o $(COM_MM)/KObjectManager.o

$(COM_MM)/Structure.o: $(SRC_MM)/Structure.cpp
	$(CC) $(CFLAGS) $(SRC_MM)/Structure.cpp -o $(COM_MM)/Structure.o

$(COM_MM)/ZoneManager.o: $(IfcMemory)/ZoneManager.h $(SRC_MM)/ZoneManager.cpp
	$(CC) $(CFLAGS) $(SRC_MM)/ZoneManager.cpp -o $(COM_MM)/ZoneManager.o

BuildMemory: $(MemoryObjects)

$(COM_SCHED)/Scheduler.o: $(SRC_SCHED)/Scheduler.cpp
	$(CC) $(CFLAGS) $(SRC_SCHED)/Scheduler.cpp -o $(COM_SCHED)/Scheduler.o

$(COM_SCHED)/ScheduleRoller.o: $(SRC_SCHED)/ScheduleRoller.cpp
	$(CC) $(CFLAGS) $(SRC_SCHED)/ScheduleRoller.cpp -o $(COM_SCHED)/ScheduleRoller.o

$(COM_SCHED)/RoundRobin.o: $(SRC_SCHED)/RoundRobin.cpp
	$(CC) $(CFLAGS) $(SRC_SCHED)/RoundRobin.cpp -o $(COM_SCHED)/RoundRobin.o

$(COM_SCHED)/RunqueueBalancer.o: $(SRC_SCHED)/RunqueueBalancer.cpp
	$(CC) $(CFLAGS) $(SRC_SCHED)/RunqueueBalancer.cpp -o $(COM_SCHED)/RunqueueBalancer.o

$(COM_SCHED)/RR.o: $(IfcHAL)/Processor.h $(IfcRunnable)/Scheduler.h $(IfcRunnable)/RoundRobin.h $(SRC_SCHED)/RR.cpp
	$(CC) $(CFLAGS) $(SRC_SCHED)/RR.cpp -o $(COM_SCHED)/RR.o

BuildScheduler: $(SchedulerObjects)

$(COM_TD)/Thread.o: $(SRC_TD)/Thread.cpp
	$(CC) $(CFLAGS) $(SRC_TD)/Thread.cpp -o $(COM_TD)/Thread.o

BuildThread: $(ThreadObjects)

$(COM_UTIL)/AVLTree.o: $(IfcUtil)/AVLTree.h $(SRC_UTIL)/AVLTree.cpp
	$(CC) $(CFLAGS) $(SRC_UTIL)/AVLTree.cpp -o $(COM_UTIL)/AVLTree.o

$(COM_UTIL)/CircuitPrimitive.o: $(SRC_UTIL)/CircuitPrimitive.cpp
	$(CC) $(CFLAGS) $(SRC_UTIL)/CircuitPrimitive.cpp -o $(COM_UTIL)/CircuitPrimitive.o

$(COM_UTIL)/CircularList.o: $(IfcUtil)/CircularList.h $(SRC_UTIL)/CircularList.cpp
	$(CC) $(CFLAGS) $(SRC_UTIL)/CircularList.cpp -o $(COM_UTIL)/CircularList.o

$(COM_UTIL)/Console.o: $(SRC_UTIL)/Console.cpp
	$(CC) $(CFLAGS) $(SRC_UTIL)/Console.cpp -o $(COM_UTIL)/Console.o

$(COM_UTIL)/Debugger.o: $(SRC_UTIL)/Debugger.cpp
	$(CC) $(CFLAGS) $(SRC_UTIL)/Debugger.cpp -o $(COM_UTIL)/Debugger.o

$(COM_UTIL)/LinkedList.o: $(IfcUtil)/LinkedList.h $(SRC_UTIL)/LinkedList.cpp
	$(CC) $(CFLAGS) $(SRC_UTIL)/LinkedList.cpp -o $(COM_UTIL)/LinkedList.o

$(COM_UTIL)/Stack.o: $(IfcUtil)/Stack.h $(SRC_UTIL)/Stack.cpp
	$(CC) $(CFLAGS) $(SRC_UTIL)/Stack.cpp -o $(COM_UTIL)/Stack.o

BuildUtil: $(UtilObjects)

$(COM_MD)/ElfAnalyzer.o: $(IfcModule)/Elf/ElfAnalyzer.hpp $(SRC_MD)/ElfAnalyzer.cpp
	$(CC) $(CFLAGS) $(SRC_MD)/ElfAnalyzer.cpp -o $(COM_MD)/ElfAnalyzer.o

$(COM_MD)/ElfLinker.o: $(IfcModule)/Elf/ElfLinker.hpp $(SRC_MD)/ElfLinker.cpp
	$(CC) $(CFLAGS) $(SRC_MD)/ElfLinker.cpp -o $(COM_MD)/ElfLinker.o

$(COM_MD)/ElfManager.o: $(IfcModule)/Elf/ElfManager.hpp $(SRC_MD)/ElfManager.cpp
	$(CC) $(CFLAGS) $(SRC_MD)/ElfManager.cpp -o $(COM_MD)/ElfManager.o

$(COM_MD)/ModuleLoader.o: $(IfcModule)/ModuleLoader.h $(SRC_MD)/ModuleLoader.cpp
	$(CC) $(CFLAGS) $(SRC_MD)/ModuleLoader.cpp -o $(COM_MD)/ModuleLoader.o

$(COM_MD)/MSI.o: $(IfcModule)/ModuleLoader.h $(SRC_MD)/MSI.cpp
	$(CC) $(CFLAGS) $(SRC_MD)/MSI.cpp -o $(COM_MD)/MSI.o

$(COM_MD)/RecordManager.o: $(IfcModule)/ModuleRecord.h $(SRC_MD)/RecordManager.cpp
	$(CC) $(CFLAGS) $(SRC_MD)/RecordManager.cpp -o $(COM_MD)/RecordManager.o

BuildModuleLoader: $(moduleLoaderObjects)

Link:
	ld -m elf_i386 -T linker.ld --export-dynamic --strip-all -pie -nostdlib \
	$(InitObjects) $(ArchObjects) $(KernelRoutineObjects) $(IntrObjects) \
	$(InterProcessObjects) $(MemoryObjects) \
	$(SchedulerObjects) $(ThreadObjects) $(UtilObjects) $(moduleLoaderObjects) --warn-unresolved-symbols -o \
	 Build/circuitk-1.02 

q: BuildChain
	qemu-system-i386 -cdrom os.iso -boot d -m 512 -smp cpus=2,cores=1,sockets=1 -display sdl

b: BuildChain
	bochs

CleanAndBuild:
	rm Compile/*
	BuildChain
