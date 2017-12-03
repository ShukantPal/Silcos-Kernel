# File: Makefile
# Module: ModuleFramework
# Package: com.silcos.mdfrwk
#
# Build: AUTO

CC = g++ # C Compiler
CFLAGS = -m32 -I"." -I"./Interface" -I"../../Interface" -I"../../Interface/Arch" -c -ffreestanding -nostdlib -nostdinc \
 -Wall -O2 -fPIC -L../../circuitk-1.02 -fno-exceptions -fno-rtti # -fno-strict-aliasing

AS = nasm # Assember
ASFLAGS = -f elf32 # Asm Flags

MDFRWK_OBJECTS = Compile/MDFSTART.o Compile/Arrays.o Compile/BinaryTree.o Compile/RBTree.o Compile/Heap.o Compile/Main.o Compile/String.o

Compile/MDFSTART.o: MDF_START.asm
	$(AS) $(ASFLAGS) MDF_START.asm -o Compile/MDFSTART.o

Compile/Arrays.o: Framework/Arrays.cxx
	$(CC) $(CFLAGS) Framework/Arrays.cxx -o Compile/Arrays.o

Compile/BinaryTree.o: Framework/BinaryTree.cxx
	$(CC) $(CFLAGS) Framework/BinaryTree.cxx -o Compile/BinaryTree.o

Compile/RBTree.o: Framework/RBTree.cxx
	$(CC) $(CFLAGS) Framework/RBTree.cxx -o Compile/RBTree.o

Compile/Heap.o: Heap.cxx
	$(CC) $(CFLAGS) Heap.cxx -o Compile/Heap.o

Compile/Main.o: Main.cpp
	$(CC) $(CFLAGS) Main.cpp -o Compile/Main.o

Compile/String.o: Framework/String.cxx
	$(CC) $(CFLAGS) Framework/String.cxx -o Compile/String.o

MfMake: $(MDFRWK_OBJECTS)
	ld -m elf_i386 -shared -T linker.ld $(MDFRWK_OBJECTS) -o com.silcos.circuit.mdfrwk.190
	cp com.silcos.circuit.mdfrwk.190 ../Builtin/