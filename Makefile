# S i l c o s   K e r n e l   B u i l d   F i l e
#        (B u i l d  &  T e s t  -  M a i n)
#
# C o m m a n d s :
#
# q, b, Build, CleanBuild
#

#
# The TargetArchitecture defines the hardware platform for which the kernel
# is currently being built. The valid values for this are IA32, IA64, ARM,
# PowerPC.
#
# Note that only IA32 is implemented for now as the kernel is in the
# development phase.
#
#
TargetArchitecture = IA32

ifeq ($(TargetArchitecture), IA32)
	export TARGET = i386-elf
else
	$(error Your architecture isn't supported by the Silcos kernel;)
endif

# Ifc - Header Directories
export Ifc = ../Interface
export IfcKern = ../Interface/KERNEL.h ../Interface/TYPE.h
export IfcArch = ../Interface/$(ArchDir)
export IfcACPI = ../Interface/ACPI
export IfcHAL = ../Interface/HAL
export IfcRunnable = ../Interface/Executable
export IfcMemory = ../Interface/Memory
export IfcModule = ../Interface/Module
export IfcUtil = ../Interface/Utils

$(Ifc)/Heap.hpp: $(IfcKern) $(IfcUtil)/Memory.h

#
# Linux is the developing environment for the Silcos kernel. The $(HOME)
# variable defines the user's home directory. The default value is '~'
# although the programmer may change it. The $(HOME)/opt/cross/bin
# directory must contain the CROSS COMPILER. See wiki.osdev.org to
# learn how to install the cross-compiler. There may also be a wiki
# on the GitHub repo of SukantPal/Silcos-Kernel
#
export HOME = /home/sukantdev

#
# C u r r e n t l y,  o n l y   I A 3 2   i s   s u p p o r t e d
#
ifeq ($(TargetArchitecture), IA32)
	export AS = nasm
	export ASFLAGS = -f elf32
	export OC = $(HOME)/opt/cross/bin/$(TARGET)-gcc
	export CC = $(HOME)/opt/cross/bin/$(TARGET)-g++
	export LD = $(HOME)/opt/cross/bin/$(TARGET)-ld
	export GNU_AS = $(HOME)/opt/cross/bin/$(TARGET)-as
	export GNU_ASFLAGS =
	export DEF_LD_SCRIPT = $(shell $(LD) --verbose)
	
	#
	# See the last line -Xassembler. It is because the assembler needs to
	# know that its building for --32 (x86) platform. If your system does
	# not use the GNU ASSEMBLER than it will be problematic.
	#
	export CFLAGS = -I"." -I"../Interface" -I"../Interface/Arch" -c \
		-fvisibility=default -ffreestanding -nostdlib -nostdinc -Wall \
		-O2 -fPIC -fno-rtti -fno-exceptions -fno-strict-aliasing
		
	export LFLAGS = -lgcc -shared -ffreestanding \
		-nostdlib -nostdinc -fno-exceptions -fno-rtti \
		-Wl,--warn-unresolved-symbols,-shared
		
	#
	# DefaultCppRuntime is used when the current-directory is just below
	# the source tree, so that the Modules folder is accessed - ../Modules
	#
	# Create another variable if the Makefile is not present at such a
	# location.
	#
	# Critical & builtin modules should use only the DefaultCppRuntime as
	# it is optimal and minimal.
	#
	export Cpp.crtbegin.o = $(shell $(CC) $(CFLAGS) --print-file-name=crtbegin.o)
	export Cpp.crtend.o = $(shell $(CC) $(CFLAGS) --print-file-name=crtend.o)
	export DefaultCppRuntime =../Modules/crti.o ../Modules/crtn.o 
	
	
				 
	#
	# Extended features may be added in the future. Cross-process app
	# modules may use extra CppRuntime features later.
	#
	export AllCppRuntime = $(DefaultCppRuntime)
else
	$(error Your architecture is not supported by the Silcos kernel;)
endif

#
# Builds the required C++ runtime objects. Currently the kernel is using the
# crti.o & crtn.o objects for global object construction and destruction.
#
BuildCppRuntime: Modules/crti.S Modules/crtn.S Modules/crt0.S
	$(GNU_AS) Modules/crti.S -o Modules/crti.o
	$(GNU_AS) Modules/crtn.S -o Modules/crtn.o
	$(GNU_AS) Modules/crt0.S -o Modules/crt0.o

#
# Builds all the module binaries and consolidates them under one *.iso file
# allowing it to be burnt on a CD/DVD or run under an emulator.
#
Build: BuildCppRuntime
	$(info WELCOME TO Silcos Kernel BUILD INFRASTRUCTURE!)
	$(MAKE) BuildChain -C ./KernelHost
	$(MAKE) IA32_HAL -C ./HAL
	$(MAKE) ExMake -C ./ExecutionManager
	$(MAKE) MfMake -C ./ModuleFramework
	$(MAKE) ObMake -C ./ObjectManager
	$(MAKE) RsMake -C ./ResourceManager
	cp KernelHost/Build/circuitk-1.02 ../circuit-iso/boot
	cp Modules/Builtin/* ../circuit-iso/boot

#
# make q - causes qemu currently in the system to test the kernel by running it
#
q: Build
	grub-mkrescue -o os.iso ../circuit-iso --modules="iso9660 part_msdos multiboot"
	qemu-system-i386 -cdrom os.iso -boot d -m 512 -smp cpus=1,cores=1,sockets=1 -display sdl -cpu Broadwell

b: Build
	grub-mkrescue -o os.iso ../circuit-iso --modules="iso9660 part_msdos multiboot"
	bochs
