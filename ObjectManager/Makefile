# S i l c o s (O b j e c t   M a n a g e r)   M a k e f i l e

CC = g++ # C Compiler
CFLAGS = -m32 -I"./Interface" -I"../Interface" -I"../Interface/Arch" -I"../ModuleFramework/Interface" \
-c -ffreestanding -nostdlib -nostdinc -Wall -O2 -fPIC -L ../KernelHost/Build/circuitk-1.02 \
-L../ModuleFramework/Build/silcos.mdfrwk

AS = nasm # Assember
ASFLAGS = -f elf32 # Asm Flags

Compile/Main.o: Main.cxx
	$(CC) $(CFLAGS) Main.cxx -o Compile/Main.o

ObMake: Compile/Main.o
	ld -m elf_i386 -shared --export-dynamic -T linker.ld Compile/Main.o -o Build/silcos.obmgr
	cp Build/silcos.obmgr ../Modules/Builtin/
