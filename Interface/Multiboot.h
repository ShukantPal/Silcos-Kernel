/*
 * Copyright (C) 2016 - Sukant Pal
 * Copyright Silca Inc. - 2016, 2017
 * @Deprecated (only for old systems, less supported)
 */

#ifndef MULTIBOOT_H
#define MULTIBOOT_H

struct aout_symbol_table
{
	unsigned long tabsize;
	unsigned long strsize;
	unsigned long addr;
	unsigned long reserved;
};
	
struct elf_section_header_table
{
	unsigned long num;
	unsigned long size;
	unsigned long addr;
	unsigned long shndx;
};

struct MultibootInfo
{
	unsigned long flags;
	unsigned long mem_lower;
	unsigned long mem_upper;
	unsigned long boot_device;
	unsigned long cmdline;
	unsigned long mods_count;
	unsigned long mods_addr;
	union
	{
		struct aout_symbol_table aout_sym;
		struct elf_section_header_table elf_sec;
	} u;
	unsigned long mmap_length;
	unsigned long mmap_addr;
} __attribute__((__packed__)) __attribute__((aligned (4)));

typedef struct MultibootInfo BootInfo;

typedef struct MultibootInfo MULTIBOOT_INFO;

struct GRUBModule
{
	unsigned long mod_start;
	unsigned long mod_end;
	unsigned long string;
	unsigned long reserved;
};

struct MultibootMemoryMap
{
	unsigned int size;
	unsigned int base_addr_low;
	unsigned int base_addr_high;
	unsigned int length_low;
	unsigned int length_high;
	unsigned int type;
} __attribute__((packed)) __attribute__((aligned (4)));

typedef struct MultibootMemoryMap MemoryMap;

#endif


