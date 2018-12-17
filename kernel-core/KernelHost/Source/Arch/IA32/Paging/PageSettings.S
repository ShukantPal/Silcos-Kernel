/**
  @file PageSetting.S

  Controls the hardware functionality of pagination. This requires low-level
  access to the x86 control registers, and therefore, can only be done in
  assembly.

  @version 1.0
  @since Silcos 3.05
  @origin 86InitPaging.asm (formerly named)
  @author Shukant Pal
 */
.section .text

/*
  Enables page-protection, by setting the PE bit in the CR4 register
  and jumps the specified address. It also adjusts the stack to point
  in the higher-half space.

  @param ulong retAddr
  @param U64* pdpt(phys)
 */
.extern CallMain
.global SetupPageProtection
.type SetupPageProtection, @function
SetupPageProtection:
	pushl %eax
	pushl %ebx
	pushl %ecx
	movl 20(%esp), %eax
	movl 16(%esp), %ebx

	movl %cr4, %ecx
	bts $5, %ecx
	movl %ecx, %cr4

	movl %eax, %cr3

	movl %cr0, %ecx
	orl $0x80000000, %ecx
	movl %ecx, %cr0

	jmpl *%ebx

.global SetupPageProtectionWithoutStack
.type SetupPageProtectionWithoutStack, @function
/**
  Enables page-protection without using the stack. This is done by
  passing the PDPT (physical) address in a register.

  @destroy ecx
  @param edi=addrOf(PDPT)
  @param ebx=return address
 */
SetupPageProtectionWithoutStack:
	movl %cr4, %ecx
	bts $5, %ecx
	movl %ecx, %cr4

	movl %edi, %cr3

	movl %cr0, %ecx
	orl $0x80000000, %ecx
	movl %ecx, %cr0

	jmp %ebx
