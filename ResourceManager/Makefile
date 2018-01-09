# S i l c o s   (R e s o u r c e   M a n a g e r)   M a k e f i l e
# 
# P a c k a g e --- c o m . s i l c o s . r s m g r
#
# B u i l d : A U T O (n o   c o m m a n d s)

CC = g++
CFLAGS = -m32 -I"." -I"./Interface" -I"../Interface" -I"../Interface/Arch" -I"../ModuleFramework/Interface" \
 -c -ffreestanding -nostdlib -nostdinc \
 -Wall -O2 -fPIC -L../KernelHost/Build/circuitk-1.02 -fno-exceptions -fno-rtti

AS = nasm
ASFLAGS = -f elf32

RS = Compile/ContextManager.o Compile/Init.o Compile/MemoryImage.o

COMPILE = Compile
SOURCE = Source

$(COMPILE)/ContextManager.o: $(SOURCE)/ContextManager.cpp
	$(CC) $(CFLAGS) $(SOURCE)/ContextManager.cpp -o $(COMPILE)/ContextManager.o

$(COMPILE)/Init.o: $(SOURCE)/Init.cpp
	$(CC) $(CFLAGS) $(SOURCE)/Init.cpp -o $(COMPILE)/Init.o
	
$(COMPILE)/MemoryImage.o: $(SOURCE)/MemoryImage.cpp
	$(CC) $(CFLAGS) $(SOURCE)/MemoryImage.cpp -o $(COMPILE)/MemoryImage.o

RsMake: $(RS)
	ld -m elf_i386 -shared -T linker.ld $(RS) -o Build/silcos.rsmgr
	cp Build/silcos.rsmgr ../Modules/Builtin/