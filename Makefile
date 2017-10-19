# File: Makefile
#
# Makefile: [CircuitKernel, 2.03, CONFIG_BUILD, CONFIG_RUN]
# Run: %Compile-ONLY
# Run: q [qemu-system-*]
# Run: b [bochs]
#
# Note:
# Latest build structure is done bya ABCM-scripting.
#
# Copyright (C) - Shukant Pal

ArchDir = Arch/IA32

CC = gcc # C Compiler
CFLAGS = -m32 -I"." -I"./Interface" -I"./Interface/Arch" -c -ffreestanding -nostdlib -nostdinc -Wall -O2 -fPIC # -fno-strict-aliasing

AS = nasm # Assember
ASFLAGS = -f elf32 # Asm Flags

# Ifc - Header Directories
IfcKern = Interface/KERNEL.h Interface/TYPE.h
IfcArch = Interface/$(ArchDir)
IfcACPI = Interface/ACPI
IfcHAL = Interface/HAL
IfcRunnable = Interface/Exec
IfcMemory = Interface/Memory
IfcModule = Interface/Module
IfcUtil = Interface/Util

#IA32 Boot Service
InitObjects = Compile/InitRuntime.o Compile/86InitPaging.o Compile/InitADM.o Compile/BlockPager.o Compile/Main.o Compile/Multiboot.o

# IA32 Objects 
ArchObjects = Compile/APBoot.o Compile/CMOS.o Compile/CPUID.o \
Compile/FrameExtractor.o Compile/Pager.o Compile/Processor.o \
Compile/TableManipulator.o Compile/APIC.o Compile/GDT.o Compile/IDT.o \
Compile/IntrHook.o Compile/Load.o Compile/TSS.o \
Compile/SwitchRunner.o

ConfigObjects = Compile/RSDP.o Compile/RSDT.o Compile/MADT.o # ACPI tables

IntrObjects = Compile/ExHandler.o # Core exception & device handlers

InterProcessObjects =

KernelRoutineObjects = Compile/Init.o

# Memory Subsystems + Algorithms
MemoryObjects = Compile/BuddyManager.o Compile/CacheRegister.o \
Compile/KFrameManager.o Compile/KMemoryManager.o  Compile/KObjectManager.o \
Compile/PMemoryManager.o Compile/Structure.o Compile/ZoneManager.o

ProcessObjects = Compile/Process.o

ThreadObjects = Compile/Thread.o

# Core Scheduler + Scheduler Classes/Rollers
SchedulerObjects = Compile/Scheduler.o Compile/RR.o #Compile/SchedList.o


UtilObjects = Compile/AVLTree.o Compile/CircuitPrimitive.o Compile/CircularList.o Compile/Console.o Compile/Debugger.o Compile/LinkedList.o Compile/Stack.o

moduleLoaderObjects = Compile/ELF.o Compile/ModuleLoader.o \
Compile/MSI.o

# Header Dependencies

BuildChain : BuildInit BuildArch BuildConfig BuildKernelRoutines BuildIntr BuildInterProcess BuildMemory BuildProcess BuildScheduler BuildThread BuildUtil BuildModuleLoader Link

$(IfcHAL)/Processor.h: $(IfcMemory)/CacheRegister.h $(IfcUtil)/AVLTree.h $(IfcUtil)/CircularList.h

Compile/86InitPaging.o: $(ArchDir)/Paging/86InitPaging.asm
	$(AS) $(ASFLAGS) $(ArchDir)/Paging/86InitPaging.asm -o Compile/86InitPaging.o

Compile/InitADM.o: $(IfcMemory)/KMemorySpace.h Interface/Multiboot2.h $(ArchDir)/Boot/InitADM.c
	$(CC) $(CFLAGS) $(ArchDir)/Boot/InitADM.c -o Compile/InitADM.o

Compile/BlockPager.o: $(ArchDir)/Paging/BlockPager.c
	$(CC) $(CFLAGS) $(ArchDir)/Paging/BlockPager.c -o Compile/BlockPager.o

Compile/InitRuntime.o: $(ArchDir)/Boot/InitRuntime.asm
	$(AS) $(ASFLAGS) $(ArchDir)/Boot/InitRuntime.asm -o Compile/InitRuntime.o

Compile/Main.o: $(ArchDir)/Boot/Main.c
	$(CC) $(CFLAGS) $(ArchDir)/Boot/Main.c -o Compile/Main.o

Compile/Multiboot.o: $(ArchDir)/Boot/Multiboot.c
	$(CC) $(CFLAGS) $(ArchDir)/Boot/Multiboot.c -o Compile/Multiboot.o

BuildInit : $(InitObjects)

Compile/APBoot.o: $(ArchDir)/HAL/APBoot.asm
	$(AS) $(ASFLAGS) $(ArchDir)/HAL/APBoot.asm -o Compile/APBoot.o

Compile/CMOS.o: $(ArchDir)/HAL/CMOS.c
	$(CC) $(CFLAGS) $(ArchDir)/HAL/CMOS.c -o Compile/CMOS.o

Compile/CPUID.o: $(ArchDir)/HAL/CPUID.asm
	$(AS) $(ASFLAGS) $(ArchDir)/HAL/CPUID.asm -o Compile/CPUID.o

Compile/FrameExtractor.o: $(ArchDir)/Paging/FrameExtractor.c
	$(CC) $(CFLAGS) $(ArchDir)/Paging/FrameExtractor.c -o Compile/FrameExtractor.o

Compile/Pager.o: $(ArchDir)/Paging/Pager.c
	$(CC) $(CFLAGS) $(ArchDir)/Paging/Pager.c -o Compile/Pager.o

Compile/Processor.o: $(IfcArch)/* $(IfcHAL)/Processor.h $(ArchDir)/HAL/Processor.c
	$(CC) $(CFLAGS) $(ArchDir)/HAL/Processor.c -o Compile/Processor.o

Compile/SysBranch.o: $(ArchDir)/Syscall/SysBranch.c
	$(CC) $(CFLAGS) $(ArchDir)/Syscall/SysBranch.c -o Compile/SysBranch.o

Compile/Syscall.o: $(ArchDir)/Syscall/Syscall.asm
	$(AS) $(ASFLAGS) $(ArchDir)/Syscall/Syscall.asm -o Compile/Syscall.o

Compile/TableManipulator.o: $(ArchDir)/Paging/TableManipulator.c
	$(CC) $(CFLAGS) $(ArchDir)/Paging/TableManipulator.c -o Compile/TableManipulator.o

Compile/APIC.o: $(ArchDir)/HAL/APIC.c
	$(CC) $(CFLAGS) $(ArchDir)/HAL/APIC.c -o Compile/APIC.o

Compile/GDT.o: $(ArchDir)/HAL/GDT.c
	$(CC) $(CFLAGS) $(ArchDir)/HAL/GDT.c -o Compile/GDT.o

Compile/IDT.o: $(ArchDir)/HAL/IDT.c
	$(CC) $(CFLAGS) $(ArchDir)/HAL/IDT.c -o Compile/IDT.o

Compile/IntrHook.o: $(ArchDir)/HAL/IntrHook.asm
	$(AS) $(ASFLAGS) $(ArchDir)/HAL/IntrHook.asm -o Compile/IntrHook.o

Compile/IO.o: $(ArchDir)/HAL/IO.asm
	$(AS) $(ASFLAGS) $(ArchDir)/HAL/IO.asm -o Compile/IO.o

Compile/Load.o: $(ArchDir)/HAL/Load.asm
	$(AS) $(ASFLAGS) $(ArchDir)/HAL/Load.asm -o Compile/Load.o

Compile/TSS.o: $(ArchDir)/HAL/TSS.c
	$(CC) $(CFLAGS) $(ArchDir)/HAL/TSS.c -o Compile/TSS.o

Compile/SwitchRunner.o: $(ArchDir)/HAL/SwitchRunner.asm
	$(AS) $(ASFLAGS) $(ArchDir)/HAL/SwitchRunner.asm -o Compile/SwitchRunner.o

BuildArch: $(ArchObjects)

Compile/RSDP.o: $(IfcACPI)/RSDP.h CoreX/ACPI/RSDP.c
	$(CC) $(CFLAGS) CoreX/ACPI/RSDP.c -o Compile/RSDP.o

Compile/RSDT.o: $(IfcACPI)/RSDT.h CoreX/ACPI/RSDT.c
	$(CC) $(CFLAGS) CoreX/ACPI/RSDT.c -o Compile/RSDT.o

Compile/MADT.o: $(IfcACPI)/RSDT.h $(IfcACPI)/MADT.h CoreX/ACPI/MADT.c
	$(CC) $(CFLAGS) CoreX/ACPI/MADT.c -o Compile/MADT.o

BuildConfig: $(ConfigObjects)

Compile/ExHandler.o: CoreX/Intr/ExHandler.c
	$(CC) $(CFLAGS) CoreX/Intr/ExHandler.c -o Compile/ExHandler.o

BuildIntr: $(IntrObjects)

Compile/MessagePassing.o: CoreX/InterProcess/MessagePassing.c
	$(CC) $(CFLAGS) CoreX/InterProcess/MessagePassing.c -o Compile/MessagePassing.o

BuildInterProcess: $(InterProcessObjects)

Compile/Init.o: CoreX/KernelRoutine/Init.c
	$(CC) $(CFLAGS) CoreX/KernelRoutine/Init.c -o Compile/Init.o

BuildKernelRoutines: $(KernelRoutineObjects)

Compile/BuddyManager.o: $(IfcMemory)/BuddyManager.h $(IfcMemory)/Pager.h CoreX/Memory/BuddyManager.c
	$(CC) $(CFLAGS) CoreX/Memory/BuddyManager.c -o Compile/BuddyManager.o

Compile/CacheRegister.o: $(IfcHAL)/Processor.h $(IfcMemory)/CacheRegister.h CoreX/Memory/CacheRegister.c	
	$(CC) $(CFLAGS) CoreX/Memory/CacheRegister.c -o Compile/CacheRegister.o

Compile/KFrameManager.o: $(IfcKern) $(IfcHAL)/Processor.h $(IfcMemory)/KFrameManager.h CoreX/Memory/KFrameManager.c
	$(CC) $(CFLAGS) CoreX/Memory/KFrameManager.c -o Compile/KFrameManager.o

Compile/KMemoryManager.o: $(IfcHAL)/Processor.h $(IfcMemory)/KMemoryManager.h CoreX/Memory/KMemoryManager.c
	$(CC) $(CFLAGS) CoreX/Memory/KMemoryManager.c -o Compile/KMemoryManager.o

Compile/KObjectManager.o: $(IfcMemory)/KObjectManager.h $(IfcMemory)/MemoryTransfer.h $(IfcMemory)/KMemoryManager.h CoreX/Memory/KObjectManager.c
	$(CC) $(CFLAGS) CoreX/Memory/KObjectManager.c -o Compile/KObjectManager.o

Compile/PMemoryManager.o: $(IfcMemory)/KMemoryManager.h $(IfcMemory)/Pager.h $(IfcMemory)/MemoryManager.h  $(IfcMemory)/PMemoryManager.h CoreX/Memory/PMemoryManager.c
	$(CC) $(CFLAGS) CoreX/Memory/PMemoryManager.c -o Compile/PMemoryManager.o

Compile/Structure.o: CoreX/Memory/Structure.c
	$(CC) $(CFLAGS) CoreX/Memory/Structure.c -o Compile/Structure.o

Compile/ZoneManager.o: $(IfcMemory)/ZoneManager.h CoreX/Memory/ZoneManager.c
	$(CC) $(CFLAGS) CoreX/Memory/ZoneManager.c -o Compile/ZoneManager.o

BuildMemory: $(MemoryObjects)

Compile/Process.o: CoreX/Process/Process.c
	$(CC) $(CFLAGS) CoreX/Process/Process.c -o Compile/Process.o

BuildProcess: $(ProcessObjects)

Compile/Scheduler.o: CoreX/Scheduling/Scheduler.c
	$(CC) $(CFLAGS) CoreX/Scheduling/Scheduler.c -o Compile/Scheduler.o

Compile/RR.o: $(IfcHAL)/Processor.h $(IfcRunnable)/Scheduler.h $(IfcRunnable)/RR.h CoreX/Scheduling/RR.c
	$(CC) $(CFLAGS) CoreX/Scheduling/RR.c -o Compile/RR.o

BuildScheduler: $(SchedulerObjects)

Compile/Thread.o: CoreX/Thread/Thread.c
	$(CC) $(CFLAGS) CoreX/Thread/Thread.c -o Compile/Thread.o

BuildThread: $(ThreadObjects)

Compile/AVLTree.o: $(IfcUtil)/AVLTree.h Util/AVLTree.c
	$(CC) $(CFLAGS) Util/AVLTree.c -o Compile/AVLTree.o

Compile/CircuitPrimitive.o: Util/CircuitPrimitive.c
	$(CC) $(CFLAGS) Util/CircuitPrimitive.c -o Compile/CircuitPrimitive.o

Compile/CircularList.o: $(IfcUtil)/CircularList.h Util/CircularList.c
	$(CC) $(CFLAGS) Util/CircularList.c -o Compile/CircularList.o

Compile/Console.o: Util/Console.c
	$(CC) $(CFLAGS) Util/Console.c -o Compile/Console.o

Compile/Debugger.o: Util/Debugger.c
	$(CC) $(CFLAGS) Util/Debugger.c -o Compile/Debugger.o

Compile/LinkedList.o: $(IfcUtil)/LinkedList.h Util/LinkedList.c
	$(CC) $(CFLAGS) Util/LinkedList.c -o Compile/LinkedList.o

Compile/Stack.o: $(IfcUtil)/Stack.h Util/Stack.c
	$(CC) $(CFLAGS) Util/Stack.c -o Compile/Stack.o

BuildUtil: $(UtilObjects)

Compile/ELF.o: $(IfcModule)/ELF.h CoreX/ModuleLoader/ELF.c
	$(CC) $(CFLAGS) CoreX/ModuleLoader/ELF.c -o Compile/ELF.o

Compile/ModuleLoader.o: CoreX/ModuleLoader/ModuleLoader.c
	$(CC) $(CFLAGS) CoreX/ModuleLoader/ModuleLoader.c -o Compile/ModuleLoader.o

Compile/MSI.o: CoreX/ModuleLoader/MSI.c
	$(CC) $(CFLAGS) CoreX/ModuleLoader/MSI.c -o Compile/MSI.o

BuildModuleLoader: $(moduleLoaderObjects)

Link: Compile/IO.o
	ld -m elf_i386 -T linker.ld --export-dynamic Compile/IO.o $(InitObjects) $(ConfigObjects) $(ArchObjects) $(KernelRoutineObjects) $(IntrObjects) $(InterProcessObjects) $(MemoryObjects) $(ProcessObjects) $(SchedulerObjects) $(ThreadObjects) $(UtilObjects) $(moduleLoaderObjects) -o circuitk-1.02
	make -C ./Modules
	cp circuitk-1.02 ../circuit-iso/boot/
	cp -a Modules/Builtin/* ../circuit-iso/boot/
	#genisoimage -R -b boot/grub/core.img -no-emul-boot -boot-load-size 4 -A os -input-charset utf8 -quiet -boot-info-table -o os.iso ../circuit-iso
	grub-mkrescue -o os.iso ../circuit-iso --modules="iso9660 part_msdos multiboot"

q: BuildChain
	qemu-system-i386 -cdrom os.iso -boot d -m 512 -smp cpus=1,cores=1,sockets=1 -display sdl

b: BuildChain
	bochs
