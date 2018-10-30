/**
 * @file Config.h
 *
 * Holds configuration settings in the form of macros, which directly manipulate
 * build specifically for different architectures.
 *
 * @version 1.0
 * @since Silcos 3.05
 * @author Shukant Pal
 */
#ifndef INITIALIZER_ARCHGENERIC_CONFIG_H_
#define INITIALIZER_ARCHGENERIC_CONFIG_H_

/**
 * "ARCH" contains the name of underlying architecture for the initializer.
 */
#define ARCH IA32

/**
 * The <tt>ARCH_32</tt> and <tt>ARCH_64</tt> macros define the bits within a
 * word.
 */
#if ARCH == IA32
	#define ARCH_32
#endif

#endif/* ArchGeneric/Config.h */
