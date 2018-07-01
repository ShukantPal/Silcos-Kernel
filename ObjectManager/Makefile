# S i l c o s (O b j e c t   M a n a g e r)   M a k e f i l e

Compile/Main.o: Main.cxx
	$(CC) $(CFLAGS) Main.cxx -o Compile/Main.o

ObMake: Compile/Main.o
	$(CC) Compile/Main.o $(LFLAGS) -o Build/silcos.obmgr -T../DefaultModuleLinkage.ld
	cp Build/silcos.obmgr ../Modules/Builtin/
