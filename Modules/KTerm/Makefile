# File: Makefile
#
# Summary:
# This file contains the build script for KTerm support. KTerm is a kernel-based
# extension for dynamic querying of subsystem status from user-space.
#
# Copyright (C) 2017 - Shukant Pal

CC = gcc # C Compiler
CFLAGS = -m32 -I"." -I"../../Interface" -I"../../Interface/Arch" -c -ffreestanding -nostdlib -nostdinc -Wall -O2 -fPIC # -fno-strict-aliasing

AS = nasm # Assember
ASFLAGS = -f elf32 # Asm Flags

Main.o: Main.c
	$(CC) $(CFLAGS) Main.c -o Compile/Main.o -L../../circuitk-1.02

KTerm_Make: Main.o
	ld -m elf_i386 -shared -T linker.ld Compile/Main.o -o com.silcos.circuit.kterm.110
	cp com.silcos.circuit.kterm.110 ../Builtin/
