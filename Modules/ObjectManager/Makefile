# File: Makefile
#
# Summary:
# This file contains the build script for KTerm support. KTerm is a kernel-based
# extension for dynamic querying of subsystem status from user-space.
#
# Copyright (C) 2017 - Shukant Pal

CC = g++ # C Compiler
CFLAGS = -m32 -I"." -"Interface" -I"../../Interface" -I"../../Interface/Arch" -I"../ModuleFramework/Interface" -c -ffreestanding -nostdlib -nostdinc -Wall -O2 -fPIC -L../../circuitk-1.02 -L../ModuleFramework/com.silcos.circuit.mdfrwk.190# -fno-strict-aliasing

AS = nasm # Assember
ASFLAGS = -f elf32 # Asm Flags

Main.o: Main.cxx
	$(CC) $(CFLAGS) Main.cxx -o Compile/Main.o

ObMake: Main.o
	ld -m elf_i386 -shared --export-dynamic -T linker.ld Compile/Main.o -o com.silcos.circuit.obmgr.111
	cp com.silcos.circuit.obmgr.111 ../Builtin/
