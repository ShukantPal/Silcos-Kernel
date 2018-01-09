# S i l c o s   (H a r d w a r e   A b s t r a c t i o n    L a y er)   M a k e f i l e
#
# Build:
# HAL consists of two main parts - ACPI tables & system topology layer. ACPI
# is mandatory but system topology depedends upon the architecture (see HAL_IA32)
#
# Commands:
# HAL_IA32 - build a ia32 hal Build/IA32
#


CC = g++ -std=c++1z # C + +   C o m p i l e r
CFLAGS = -m32 -I"." -I"../Interface" -I"../Interface/Arch" -c \
-fvisibility=default -ffreestanding -nostdlib -nostdinc -Wall \
-Os -fPIC -fno-rtti -fno-exceptions -L../KernelHost/Build/circuitk-1.02

AS = nasm # A s s e m b e r
ASFLAGS = -f elf32 # A s m   F l a g s

COMPILE = Compile/IA-32
SOURCE = Source/IA-32

COM_ACPI = Compile/ACPI
SRC_ACPI = Source/ACPI

#
# A C P I   B u i l d   S y s t e m
#

ACPI_Build = $(COM_ACPI)/MADT.o $(COM_ACPI)/RSDP.o \
$(COM_ACPI)/RSDT.o

$(COM_ACPI)/MADT.o: $(SRC_ACPI)/MADT.cpp
	$(CC) $(CFLAGS) $(SRC_ACPI)/MADT.cpp -o $(COM_ACPI)/MADT.o
	
$(COM_ACPI)/RSDP.o: $(SRC_ACPI)/RSDP.cpp
	$(CC) $(CFLAGS) $(SRC_ACPI)/RSDP.cpp -o $(COM_ACPI)/RSDP.o
	
$(COM_ACPI)/RSDT.o: $(SRC_ACPI)/RSDT.cpp
	$(CC) $(CFLAGS) $(SRC_ACPI)/RSDT.cpp -o $(COM_ACPI)/RSDT.o

#
# I A 3 2   B u i l d   S y s t e m
#

IA32_Build = $(COMPILE)/APBoot.o $(COMPILE)/APIC.o $(COMPILE)/CMOS.o 	\
$(COMPILE)/CPUDriver.o $(COMPILE)/CPUID.o $(COMPILE)/GDT.o 				\
$(COMPILE)/IDT.o $(COMPILE)/IntrHook.o $(COMPILE)/IO.o 					\
$(COMPILE)/Load.o $(COMPILE)/Processor.o $(COMPILE)/ProcessorTopology.o \
$(COMPILE)/Startup.o $(COMPILE)/SwitchRunner.o $(COMPILE)/TSS.o

$(COMPILE)/APBoot.o: $(SOURCE)/APBoot.asm
	$(AS) $(ASFLAGS) $(SOURCE)/APBoot.asm -o $(COMPILE)/APBoot.o

$(COMPILE)/APIC.o: $(SOURCE)/APIC.cpp
	$(CC) $(CFLAGS) $(SOURCE)/APIC.cpp -o $(COMPILE)/APIC.o

$(COMPILE)/CMOS.o: $(SOURCE)/CMOS.cpp
	$(CC) $(CFLAGS) $(SOURCE)/CMOS.cpp -o $(COMPILE)/CMOS.o
	
$(COMPILE)/CPUDriver.o: $(SOURCE)/CPUDriver.cpp
	$(CC) $(CFLAGS) $(SOURCE)/CPUDriver.cpp -o $(COMPILE)/CPUDriver.o
	
$(COMPILE)/CPUID.o: $(SOURCE)/CPUID.asm
	$(AS) $(ASFLAGS) $(SOURCE)/CPUID.asm -o $(COMPILE)/CPUID.o

$(COMPILE)/GDT.o: $(SOURCE)/GDT.cpp
	$(CC) $(CFLAGS) $(SOURCE)/GDT.cpp -o $(COMPILE)/GDT.o
	
$(COMPILE)/IDT.o: $(SOURCE)/IDT.cpp
	$(CC) $(CFLAGS) $(SOURCE)/IDT.cpp -o $(COMPILE)/IDT.o
	
$(COMPILE)/IntrHook.o: $(SOURCE)/IntrHook.asm
	$(AS) $(ASFLAGS) $(SOURCE)/IntrHook.asm -o $(COMPILE)/IntrHook.o

$(COMPILE)/IO.o: $(SOURCE)/IO.asm
	$(AS) $(ASFLAGS) $(SOURCE)/IO.asm -o $(COMPILE)/IO.o

$(COMPILE)/Load.o: $(SOURCE)/Load.asm
	$(AS) $(ASFLAGS) $(SOURCE)/Load.asm -o $(COMPILE)/Load.o

$(COMPILE)/Processor.o: $(SOURCE)/Processor.cpp
	$(CC) $(CFLAGS) $(SOURCE)/Processor.cpp -o $(COMPILE)/Processor.o

$(COMPILE)/ProcessorTopology.o: $(SOURCE)/ProcessorTopology.cpp
	$(CC) $(CFLAGS) $(SOURCE)/ProcessorTopology.cpp -o $(COMPILE)/ProcessorTopology.o

$(COMPILE)/Startup.o: $(SOURCE)/Startup.cpp
	$(CC) $(CFLAGS) $(SOURCE)/Startup.cpp -o $(COMPILE)/Startup.o

$(COMPILE)/SwitchRunner.o: $(SOURCE)/SwitchRunner.asm
	$(AS) $(ASFLAGS) $(SOURCE)/SwitchRunner.asm -o $(COMPILE)/SwitchRunner.o
	
$(COMPILE)/TSS.o: $(SOURCE)/TSS.cpp
	$(CC) $(CFLAGS) $(SOURCE)/TSS.cpp -o $(COMPILE)/TSS.o
	
#
# C o m m a n d   f o r   I A 3 2  b u i l d  --  H A L
#

IA32_HAL: $(ACPI_Build) $(IA32_Build)
	ld -m elf_i386 -shared -T linker.ld $(ACPI_Build) $(IA32_Build) -o Build/kernel.silcos.hal -h "HardwareAbstractionLayer" \
	-L../KernelHost/Build/circuitk-1.02
	cp Build/kernel.silcos.hal ../Modules/Builtin/
	