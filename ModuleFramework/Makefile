# File: Makefile
# Module: ModuleFramework
# Package: com.silcos.mdfrwk
#
# Build: AUTO

AS = nasm # Assember
ASFLAGS = -f elf32 # Asm Flags

# Utilities are compiled here (although they lie in the global-NS)
MDFRWK_UTIL = Compile/ArrayList.o Compile/Arrays.o \
Compile/BinaryTree.o Compile/HashMap.o \
Compile/Object.o Compile/RBTree.o Compile/String.o

# Code that is not classifed is compiled here!
MDFRWK = Compile/Heap.o Compile/Main.o

Compile/ArrayList.o: Framework/ArrayList.cpp
	$(CC) $(CFLAGS) Framework/ArrayList.cpp -o Compile/ArrayList.o

Compile/Arrays.o: Framework/Arrays.cpp
	$(CC) $(CFLAGS) Framework/Arrays.cpp -o Compile/Arrays.o

Compile/BinaryTree.o: Framework/BinaryTree.cpp
	$(CC) $(CFLAGS) Framework/BinaryTree.cpp -o Compile/BinaryTree.o

Compile/HashMap.o: Framework/HashMap.cpp
	$(CC) $(CFLAGS) Framework/HashMap.cpp -o Compile/HashMap.o

Compile/Object.o: Framework/Object.cpp
	$(CC) $(CFLAGS) Framework/Object.cpp -o Compile/Object.o

Compile/RBTree.o: Framework/RBTree.cpp
	$(CC) $(CFLAGS) Framework/RBTree.cpp -o Compile/RBTree.o

Compile/Heap.o: Heap.cpp
	$(CC) $(CFLAGS) Heap.cpp -o Compile/Heap.o

Compile/Main.o: Main.cpp
	$(CC) $(CFLAGS) Main.cpp -o Compile/Main.o

Compile/String.o: Framework/String.cpp
	$(CC) $(CFLAGS) Framework/String.cpp -o Compile/String.o

MfMake: $(MDFRWK_UTIL) $(MDFRWK)
	$(CC) $(MDFRWK_UTIL) $(MDFRWK) $(LFLAGS) -o Build/silcos.mdfrwk
	cp Build/silcos.mdfrwk ../Modules/Builtin/