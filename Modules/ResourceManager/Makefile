# File: Makefile
# Module: ResourceManager
# Package: com.silcos.rsmgr
#
# Build: AUTO

CC = g++ # C Compiler
CFLAGS = -m32 -I"." -I"./Interface" -I"../../Interface" -I"../../Interface/Arch" -I"../ModuleFramework/Interface" \
 -c -ffreestanding -nostdlib -nostdinc \
 -Wall -O2 -fPIC -L../../circuitk-1.02 -fno-exceptions -fno-rtti # -fno-strict-aliasing

AS = nasm # Assember
ASFLAGS = -f elf32 # Asm Flags

RS = Compile/ContextManager.o Compile/Init.o Compile/MemoryImage.o

Compile/ContextManager.o: Source/ContextManager.cpp
	$(CC) $(CFLAGS) Source/ContextManager.cpp -o Compile/ContextManager.o

Compile/Init.o: Source/Init.cpp
	$(CC) $(CFLAGS) Source/Init.cpp -o Compile/Init.o
	
Compile/MemoryImage.o: Source/MemoryImage.cpp
	$(CC) $(CFLAGS) Source/MemoryImage.cpp -o Compile/MemoryImage.o

RsMake: $(RS)
	ld -m elf_i386 -shared -T ../linker.ld $(RS) -o com.silcos.circuit.rsmgr.250
	cp com.silcos.circuit.rsmgr.250 ../Builtin/