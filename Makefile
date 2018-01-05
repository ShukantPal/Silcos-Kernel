# File: Makefile
#
# Makefile: [CircuitKernel, 2.03, CONFIG_BUILD, CONFIG_RUN]
# Run: %Compile-ONLY
# Run: q [qemu-system-*]
# Run: b [bochs]
#
# Note:
# Latest build structure is done bya AutoBuild/AutoRun (by Silca) Script (by fuctoria kernel)
#
# Copyright (C) - Shukant Pal

ArchDir = Arch/IA32

CC = g++ -std=c++1z# C Compiler
CFLAGS = -m32 -I"." -I"./Interface" -I"./Interface/Arch" -c -fvisibility=default -ffreestanding -nostdlib -nostdinc -Wall -Os -fPIC -fpermissive -fno-rtti -fno-exceptions# -fno-strict-aliasing

AS = nasm # Assember
ASFLAGS = -f elf32 # Asm Flags

# Ifc - Header Directories
IfcKern = Interface/KERNEL.h Interface/TYPE.h
IfcArch = Interface/$(ArchDir)
IfcACPI = Interface/ACPI
IfcHAL = Interface/HAL
IfcRunnable = Interface/Executable
IfcMemory = Interface/Memory
IfcModule = Interface/Module
IfcUtil = Interface/Util

#IA32 Boot Service
InitObjects = Compile/InitRuntime.o Compile/86InitPaging.o Compile/BlockPager.o Compile/Main.o Compile/Multiboot.o

# IA32 Objects 
ArchObjects = Compile/APBoot.o Compile/CMOS.o Compile/CPUID.o \
Compile/CPUDriver.o Compile/FrameExtractor.o Compile/Pager.o \
Compile/Processor.o Compile/TableManipulator.o Compile/APIC.o \
Compile/GDT.o Compile/IDT.o Compile/IntrHook.o Compile/Load.o \
Compile/TSS.o Compile/SwitchRunner.o Compile/ProcessorTopology.o

ConfigObjects = Compile/RSDP.o Compile/RSDT.o Compile/MADT.o # ACPI tables

IntrObjects = Compile/ExHandler.o # Core exception & device handlers

InterProcessObjects =

KernelRoutineObjects = Compile/Init.o

# Memory Subsystems + Algorithms
MemoryObjects = Compile/BuddyAllocator.o Compile/CacheRegister.o \
Compile/KFrameManager.o Compile/KMemoryManager.o  Compile/KObjectManager.o \
Compile/Structure.o Compile/ZoneManager.o

ProcessObjects = Compile/Process.o

ThreadObjects = Compile/Thread.o

# Core Scheduler + Scheduler Classes/Rollers
SchedulerObjects = Compile/Scheduler.o Compile/ScheduleRoller.o \
Compile/RoundRobin.o Compile/RunqueueBalancer.o #Compile/SchedList.o

UtilObjects = Compile/AVLTree.o Compile/CircuitPrimitive.o Compile/CircularList.o \
Compile/Console.o Compile/Debugger.o Compile/LinkedList.o Compile/Stack.o

moduleLoaderObjects = Compile/ElfManager.o Compile/ElfAnalyzer.o \
Compile/ModuleLoader.o Compile/MSI.o Compile/ElfLinker.o Compile/RecordManager.o

# Header Dependencies

BuildChain : BuildInit BuildArch BuildConfig BuildKernelRoutines BuildIntr BuildInterProcess BuildMemory BuildProcess BuildScheduler BuildThread BuildUtil BuildModuleLoader Link

$(IfcHAL)/Processor.h: $(IfcMemory)/CacheRegister.h $(IfcUtil)/AVLTree.h $(IfcUtil)/CircularList.h

$(IfcModule)/Elf/ELF.h: $(IfcModule)/ModuleLoader.h
$(IfcModule)/Elf/ElfManager.hpp: $(IfcModule)/Elf/ELF.h

Compile/86InitPaging.o: $(ArchDir)/Paging/86InitPaging.asm
	$(AS) $(ASFLAGS) $(ArchDir)/Paging/86InitPaging.asm -o Compile/86InitPaging.o

Compile/BlockPager.o: $(ArchDir)/Paging/BlockPager.cpp
	$(CC) $(CFLAGS) $(ArchDir)/Paging/BlockPager.cpp -o Compile/BlockPager.o

Compile/InitRuntime.o: $(ArchDir)/Boot/InitRuntime.asm
	$(AS) $(ASFLAGS) $(ArchDir)/Boot/InitRuntime.asm -o Compile/InitRuntime.o

Compile/Main.o: $(ArchDir)/Boot/Main.cpp
	$(CC) $(CFLAGS) $(ArchDir)/Boot/Main.cpp -o Compile/Main.o

Compile/Multiboot.o: $(ArchDir)/Boot/Multiboot.cpp
	$(CC) $(CFLAGS) $(ArchDir)/Boot/Multiboot.cpp -o Compile/Multiboot.o

BuildInit : $(InitObjects)

Compile/APBoot.o: $(ArchDir)/HAL/APBoot.asm
	$(AS) $(ASFLAGS) $(ArchDir)/HAL/APBoot.asm -o Compile/APBoot.o

Compile/CMOS.o: $(ArchDir)/HAL/CMOS.cpp
	$(CC) $(CFLAGS) $(ArchDir)/HAL/CMOS.cpp -o Compile/CMOS.o

Compile/CPUDriver.o: $(ArchDir)/HAL/CPUDriver.cpp
	$(CC) $(CFLAGS) $(ArchDir)/HAL/CPUDriver.cpp -o Compile/CPUDriver.o

Compile/CPUID.o: $(ArchDir)/HAL/CPUID.asm
	$(AS) $(ASFLAGS) $(ArchDir)/HAL/CPUID.asm -o Compile/CPUID.o

Compile/FrameExtractor.o: $(ArchDir)/Paging/FrameExtractor.cpp
	$(CC) $(CFLAGS) $(ArchDir)/Paging/FrameExtractor.cpp -o Compile/FrameExtractor.o

Compile/Pager.o: $(ArchDir)/Paging/Pager.cpp
	$(CC) $(CFLAGS) $(ArchDir)/Paging/Pager.cpp -o Compile/Pager.o

Compile/Processor.o: $(IfcArch)/* $(IfcHAL)/Processor.h $(ArchDir)/HAL/Processor.cpp
	$(CC) $(CFLAGS) $(ArchDir)/HAL/Processor.cpp -o Compile/Processor.o

Compile/ProcessorTopology.o: $(IfcHAL)/ProcessorTopology.hpp $(ArchDir)/HAL/ProcessorTopology.cpp
	$(CC) $(CFLAGS) $(ArchDir)/HAL/ProcessorTopology.cpp -o Compile/ProcessorTopology.o

Compile/SysBranch.o: $(ArchDir)/Syscall/SysBranch.cpp
	$(CC) $(CFLAGS) $(ArchDir)/Syscall/SysBranch.cpp -o Compile/SysBranch.o

Compile/Syscall.o: $(ArchDir)/Syscall/Syscall.asm
	$(AS) $(ASFLAGS) $(ArchDir)/Syscall/Syscall.asm -o Compile/Syscall.o

Compile/TableManipulator.o: $(ArchDir)/Paging/TableManipulator.cpp
	$(CC) $(CFLAGS) $(ArchDir)/Paging/TableManipulator.cpp -o Compile/TableManipulator.o

Compile/APIC.o: $(ArchDir)/HAL/APIC.cpp
	$(CC) $(CFLAGS) $(ArchDir)/HAL/APIC.cpp -o Compile/APIC.o

Compile/GDT.o: $(ArchDir)/HAL/GDT.cpp
	$(CC) $(CFLAGS) $(ArchDir)/HAL/GDT.cpp -o Compile/GDT.o

Compile/IDT.o: $(ArchDir)/HAL/IDT.cpp
	$(CC) $(CFLAGS) $(ArchDir)/HAL/IDT.cpp -o Compile/IDT.o

Compile/IntrHook.o: $(ArchDir)/HAL/IntrHook.asm
	$(AS) $(ASFLAGS) $(ArchDir)/HAL/IntrHook.asm -o Compile/IntrHook.o

Compile/IO.o: $(ArchDir)/HAL/IO.asm
	$(AS) $(ASFLAGS) $(ArchDir)/HAL/IO.asm -o Compile/IO.o

Compile/Load.o: $(ArchDir)/HAL/Load.asm
	$(AS) $(ASFLAGS) $(ArchDir)/HAL/Load.asm -o Compile/Load.o

Compile/TSS.o: $(ArchDir)/HAL/TSS.cpp
	$(CC) $(CFLAGS) $(ArchDir)/HAL/TSS.cpp -o Compile/TSS.o

Compile/SwitchRunner.o: $(ArchDir)/HAL/SwitchRunner.asm
	$(AS) $(ASFLAGS) $(ArchDir)/HAL/SwitchRunner.asm -o Compile/SwitchRunner.o

BuildArch: $(ArchObjects)

Compile/RSDP.o: $(IfcACPI)/RSDP.h CoreX/ACPI/RSDP.cpp
	$(CC) $(CFLAGS) CoreX/ACPI/RSDP.cpp -o Compile/RSDP.o

Compile/RSDT.o: $(IfcACPI)/RSDT.h CoreX/ACPI/RSDT.cpp
	$(CC) $(CFLAGS) CoreX/ACPI/RSDT.cpp -o Compile/RSDT.o

Compile/MADT.o: $(IfcACPI)/RSDT.h $(IfcACPI)/MADT.h CoreX/ACPI/MADT.cpp
	$(CC) $(CFLAGS) CoreX/ACPI/MADT.cpp -o Compile/MADT.o

BuildConfig: $(ConfigObjects)

Compile/ExHandler.o: CoreX/Intr/ExHandler.cpp
	$(CC) $(CFLAGS) CoreX/Intr/ExHandler.cpp -o Compile/ExHandler.o

BuildIntr: $(IntrObjects)

Compile/MessagePassing.o: CoreX/InterProcess/MessagePassing.cpp
	$(CC) $(CFLAGS) CoreX/InterProcess/MessagePassing.cpp -o Compile/MessagePassing.o

BuildInterProcess: $(InterProcessObjects)

Compile/Init.o: CoreX/KernelRoutine/Init.cpp
	$(CC) $(CFLAGS) CoreX/KernelRoutine/Init.cpp -o Compile/Init.o

BuildKernelRoutines: $(KernelRoutineObjects)

Compile/BuddyAllocator.o: $(IfcMemory)/BuddyManager.h $(IfcMemory)/Pager.h CoreX/Memory/BuddyAllocator.cpp
	$(CC) $(CFLAGS) CoreX/Memory/BuddyAllocator.cpp -o Compile/BuddyAllocator.o

Compile/CacheRegister.o: $(IfcHAL)/Processor.h $(IfcMemory)/CacheRegister.h CoreX/Memory/CacheRegister.cpp	
	$(CC) $(CFLAGS) CoreX/Memory/CacheRegister.cpp -o Compile/CacheRegister.o

Compile/KFrameManager.o: $(IfcKern) $(IfcHAL)/Processor.h $(IfcMemory)/KFrameManager.h CoreX/Memory/KFrameManager.cpp
	$(CC) $(CFLAGS) CoreX/Memory/KFrameManager.cpp -o Compile/KFrameManager.o

Compile/KMemoryManager.o: $(IfcHAL)/Processor.h $(IfcMemory)/KMemoryManager.h CoreX/Memory/KMemoryManager.cpp
	$(CC) $(CFLAGS) CoreX/Memory/KMemoryManager.cpp -o Compile/KMemoryManager.o

Compile/KObjectManager.o: $(IfcMemory)/KObjectManager.h $(IfcMemory)/MemoryTransfer.h $(IfcMemory)/KMemoryManager.h CoreX/Memory/KObjectManager.cpp
	$(CC) $(CFLAGS) CoreX/Memory/KObjectManager.cpp -o Compile/KObjectManager.o

Compile/PMemoryManager.o: $(IfcMemory)/KMemoryManager.h $(IfcMemory)/Pager.h $(IfcMemory)/MemoryManager.h  $(IfcMemory)/PMemoryManager.h CoreX/Memory/PMemoryManager.cpp
	$(CC) $(CFLAGS) CoreX/Memory/PMemoryManager.cpp -o Compile/PMemoryManager.o

Compile/Structure.o: CoreX/Memory/Structure.cpp
	$(CC) $(CFLAGS) CoreX/Memory/Structure.cpp -o Compile/Structure.o

Compile/ZoneManager.o: $(IfcMemory)/ZoneManager.h CoreX/Memory/ZoneManager.cpp
	$(CC) $(CFLAGS) CoreX/Memory/ZoneManager.cpp -o Compile/ZoneManager.o

BuildMemory: $(MemoryObjects)

Compile/Process.o: CoreX/Process/Process.cpp
	$(CC) $(CFLAGS) CoreX/Process/Process.cpp -o Compile/Process.o

BuildProcess: $(ProcessObjects)

Compile/Scheduler.o: CoreX/Scheduling/Scheduler.cpp
	$(CC) $(CFLAGS) CoreX/Scheduling/Scheduler.cpp -o Compile/Scheduler.o

Compile/ScheduleRoller.o: CoreX/Scheduling/ScheduleRoller.cpp
	$(CC) $(CFLAGS) CoreX/Scheduling/ScheduleRoller.cpp -o Compile/ScheduleRoller.o

Compile/RoundRobin.o: CoreX/Scheduling/RoundRobin.cpp
	$(CC) $(CFLAGS) CoreX/Scheduling/RoundRobin.cpp -o Compile/RoundRobin.o

Compile/RunqueueBalancer.o: CoreX/Scheduling/RunqueueBalancer.cpp
	$(CC) $(CFLAGS) CoreX/Scheduling/RunqueueBalancer.cpp -o Compile/RunqueueBalancer.o

Compile/RR.o: $(IfcHAL)/Processor.h $(IfcRunnable)/Scheduler.h $(IfcRunnable)/RoundRobin.h CoreX/Scheduling/RR.cpp
	$(CC) $(CFLAGS) CoreX/Scheduling/RR.cpp -o Compile/RR.o

BuildScheduler: $(SchedulerObjects)

Compile/Thread.o: CoreX/Thread/Thread.cpp
	$(CC) $(CFLAGS) CoreX/Thread/Thread.cpp -o Compile/Thread.o

BuildThread: $(ThreadObjects)

Compile/AVLTree.o: $(IfcUtil)/AVLTree.h Util/AVLTree.cpp
	$(CC) $(CFLAGS) Util/AVLTree.cpp -o Compile/AVLTree.o

Compile/CircuitPrimitive.o: Util/CircuitPrimitive.cpp
	$(CC) $(CFLAGS) Util/CircuitPrimitive.cpp -o Compile/CircuitPrimitive.o

Compile/CircularList.o: $(IfcUtil)/CircularList.h Util/CircularList.cpp
	$(CC) $(CFLAGS) Util/CircularList.cpp -o Compile/CircularList.o

Compile/Console.o: Util/Console.cpp
	$(CC) $(CFLAGS) Util/Console.cpp -o Compile/Console.o

Compile/Debugger.o: Util/Debugger.cpp
	$(CC) $(CFLAGS) Util/Debugger.cpp -o Compile/Debugger.o

Compile/LinkedList.o: $(IfcUtil)/LinkedList.h Util/LinkedList.cpp
	$(CC) $(CFLAGS) Util/LinkedList.cpp -o Compile/LinkedList.o

Compile/Stack.o: $(IfcUtil)/Stack.h Util/Stack.cpp
	$(CC) $(CFLAGS) Util/Stack.cpp -o Compile/Stack.o

BuildUtil: $(UtilObjects)

Compile/ElfAnalyzer.o: $(IfcModule)/Elf/ElfAnalyzer.hpp CoreX/ModuleLoader/ElfAnalyzer.cpp
	$(CC) $(CFLAGS) CoreX/ModuleLoader/ElfAnalyzer.cpp -o Compile/ElfAnalyzer.o

Compile/ElfLinker.o: $(IfcModule)/Elf/ElfLinker.hpp CoreX/ModuleLoader/ElfLinker.cpp
	$(CC) $(CFLAGS) CoreX/ModuleLoader/ElfLinker.cpp -o Compile/ElfLinker.o

Compile/ElfManager.o: $(IfcModule)/Elf/ElfManager.hpp CoreX/ModuleLoader/ElfManager.cpp
	$(CC) $(CFLAGS) CoreX/ModuleLoader/ElfManager.cpp -o Compile/ElfManager.o

Compile/ModuleLoader.o: $(IfcModule)/ModuleLoader.h CoreX/ModuleLoader/ModuleLoader.cpp
	$(CC) $(CFLAGS) CoreX/ModuleLoader/ModuleLoader.cpp -o Compile/ModuleLoader.o

Compile/MSI.o: $(IfcModule)/ModuleLoader.h CoreX/ModuleLoader/MSI.cpp
	$(CC) $(CFLAGS) CoreX/ModuleLoader/MSI.cpp -o Compile/MSI.o

Compile/RecordManager.o: $(IfcModule)/ModuleRecord.h CoreX/ModuleLoader/RecordManager.cpp
	$(CC) $(CFLAGS) CoreX/ModuleLoader/RecordManager.cpp -o Compile/RecordManager.o

BuildModuleLoader: $(moduleLoaderObjects)

Link: Compile/IO.o
	ld -m elf_i386 -T linker.ld --export-dynamic --strip-all -pie Compile/IO.o $(InitObjects) $(ConfigObjects) $(ArchObjects) $(KernelRoutineObjects) $(IntrObjects) $(InterProcessObjects) $(MemoryObjects) $(ProcessObjects) $(SchedulerObjects) $(ThreadObjects) $(UtilObjects) $(moduleLoaderObjects) -o circuitk-1.02
	make -C ./Modules
	cp circuitk-1.02 ../circuit-iso/boot/
	cp -a Modules/Builtin/* ../circuit-iso/boot/
	#genisoimage -R -b boot/grub/core.img -no-emul-boot -boot-load-size 4 -A os -input-charset utf8 -quiet -boot-info-table -o os.iso ../circuit-iso
	grub-mkrescue -o os.iso ../circuit-iso --modules="iso9660 part_msdos multiboot"

q: BuildChain
	qemu-system-i386 -cdrom os.iso -boot d -m 512 -smp cpus=2,cores=1,sockets=1 -display sdl

b: BuildChain
	bochs

CleanAndBuild:
	rm Compile/*
	BuildChain
