# S i l c o s   (R e s o u r c e   M a n a g e r)   M a k e f i l e
# 
# P a c k a g e --- c o m . s i l c o s . r s m g r
#
# B u i l d : A U T O (n o   c o m m a n d s)

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
	$(CC) $(RS) $(LFLAGS) -o Build/silcos.rsmgr
	cp Build/silcos.rsmgr ../Modules/Builtin/