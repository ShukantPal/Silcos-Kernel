# S i l c o s   (I n i t i a l i z e r)   M a k e f i l e
#
# Build:
# The initializer is a **static** executable. It loads the kernel environment
# and abstracts the bootloader from the system. It performs the crucial role
# of bootstrapping the KernelHost.
#
#
#
#

COM= ./Compile/$(TargetArchitecture)
SRC = ./Source/$(TargetArchitecture)

COM_GENERIC = ./Compile/Generic
SRC_GENERIC = ./Source/Generic

COM_GENELF = ./Compile/Generic/Elf
SRC_GENELF = ./Source/Generic/Elf

IFC_GRC = ./Headers/Generic
IFC_GRCELF = ./Headers/Generic/Elf


LCL_FLGS = -I"./Headers/"


#
# I A 3 2   B u i l d   S y s t e m
#

IA32_Build = $(COM)/Startup.o $(COM)/LoadKernel.o

$(COM)/Startup.o: $(SRC)/Startup.S
	$(GNU_AS) $(GNU_ASFLAGS) $(SRC)/Startup.S -o $(COM)/Startup.o
	
$(COM)/LoadKernel.o: $(SRC)/LoadKernel.c
	$(OC) $(OFLAGS) $(LCL_FLGS) $(SRC)/LoadKernel.c -o $(COM)/LoadKernel.o

Generic_Build = $(COM_GENELF)/BinaryAnalyzer.o $(COM_GENERIC)/EarlyConsole.o \
$(COM_GENERIC)/EarlyMultiboot.o \
$(COM_GENELF)/KernelModule.o $(COM_GENELF)/Relocator.o

$(COM_GENELF)/BinaryAnalyzer.o: $(SRC_GENELF)/BinaryAnalyzer.c $(IFC_GRCELF)/BinaryAnalyzer.h
	$(OC) $(OFLAGS) $(LCL_FLGS) $(SRC_GENELF)/BinaryAnalyzer.c -o $(COM_GENELF)/BinaryAnalyzer.o

$(COM_GENERIC)/EarlyConsole.o: $(SRC_GENERIC)/EarlyConsole.c $(IFC_GRC)/EarlyConsole.h
	$(OC) $(OFLAGS) $(LCL_FLGS) $(SRC_GENERIC)/EarlyConsole.c -o $(COM_GENERIC)/EarlyConsole.o

$(COM_GENERIC)/EarlyMultiboot.o: $(SRC_GENERIC)/EarlyMultiboot.c $(IFC_GRC)/EarlyMultiboot.h
	$(OC) $(OFLAGS) $(LCL_FLGS) $(SRC_GENERIC)/EarlyMultiboot.c -o $(COM_GENERIC)/EarlyMultiboot.o

$(COM_GENELF)/KernelModule.o: $(SRC_GENELF)/KernelModule.c $(IFC_GRCELF)/KernelModule.h
	$(OC) $(OFLAGS) $(LCL_FLGS) $(SRC_GENELF)/KernelModule.c -o $(COM_GENELF)/KernelModule.o

$(COM_GENELF)/Relocator.o: $(SRC_GENELF)/Relocator.c $(IFC_GRCELF)/Relocator.h
	$(OC) $(OFLAGS) $(LCL_FLGS) $(SRC_GENELF)/Relocator.c -o $(COM_GENELF)/Relocator.o

IA32_INITOR: $(IA32_Build) $(Generic_Build)
	echo $(LD)
	$(LD) $(IA32_Build) $(Generic_Build) -o Build/Initor -T InitorBuild_IA32.ld
	cp Build/Initor ../Modules/Builtin/
