/**
   @file APBoot.S
   @see GCC (compile with GCC)

   Contains the trampoline code for boot-strapping application processors.

   Copyright (C) 2017, 2018 - Shukant Pal
 */
.globl APBoot
.globl apSetupInfo
.globl APBootSequenceStart
.globl APBootSequenceEnd

.section .text

#define GDT_ENTRY_SIZE 8
#define GDT_SIZE (GDT_ENTRY_SIZE * 3)
#define GDT_POINTER_SIZE 6

#define STATUS_INIT     0x0002
#define STATUS_BOOTING  0x00BB
#define STATUS_ERROR 	0x00FF

#define PADDR_TRAMP 	589824 # KB(576) Trampoline is loaded here
#define PADDR_AP_PMInit  PADDR_TRAMP + $AP_PMInit-$APBoot
#define PADDR_APSETUPRUNTIME    PADDR_TRAMP + APSetupRuntime-APBoot
#define PADDR_DEFAULT_BOOT_GDT  PADDR_TRAMP + $defaultBootGDT-$APBoot
#define PADDR_DEFAULT_BOOT_GDT_POINTER  PADDR_TRAMP + $defaultBootGDTPointer-$APBoot

.align 4

/*=========================================================
 ************* Application Processor Booting **************

 Contains code to bootstrap APs when they are sent an INIT
 signal from the BSP. APBootSequenceStart to APBootSequenceEnd
 is copied to a low-memory location. That section is called
 the trampoline, which is constructed so that the AP can
 locate the kernel environment.
 =========================================================*/

/* Start of .text */
APBoot:

/* Start of copied trampoline

 APBootSequenceStart to APBootSequenceEnd should be copied to
 a page-aligned address below 1 MB to initialize APs.

 @see ConstructTrampoline (Processor.cpp) */
APBootSequenceStart:

/**
  AP will start executing code here in real-mode. Loads the boot
  time GDT and then enters protected mode. Jumps into 0x8:AP_PMInit
  after setings PM bit.
 */
.code16
APRealModeInit:
	cli
	movw %cs, %bx
    movw %bx, %ds

	/* ebx stores our code & data base address */
	shll $4, %ebx
	movw $STATUS_BOOTING, (apBootStatusRegister - APBoot)

	movl %ebx, %eax
	addl $0+defaultBootGDT-APBoot, %eax /* linear address of boot GDT */
/* don't remove $0, it means the expression is a constant.
   tests crash if removed. */

	/* load linear address of GDT into GDT pointer */
	movl %eax, (defaultBootGDTPointer-APBoot+2)
	/* load GDT */
	lgdtl (defaultBootGDTPointer-APBoot)

	/* Load toPMJumpRegister with little-endian destination */
	movl %ebx, %eax
	addl $0+AP_PMInit-APBoot, %eax
	movl %eax, %ds:toPMJumpOffset-APBoot
	movw $0x8, %ds:toPMJumpSelector-APBoot

	movl %cr0, %eax
	orl $0x1, %eax
	movl %eax, %cr0

	/* ebx is passed as a register arg */
#	ljmpl %ds:toPMJumpAddressRegister-APBoot
	ljmpl $0x8, $589824+AP_PMInit-APBoot

/**
  (To be called in PM, only from APRealModeInit)

  Setups the data segment selectors and then jumps to the
  physical address of the HAL symbol APSetupRuntime from
  which further initialization occurs.

  Symbol addresses are calculated using the HAL load address
  and physical address.

  NOTE, that kernelVAddr, kernelPAddr, and kernelSize are
  copied from the trampoline.
 */
.code32
.align 4
AP_PMInit:
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss

	/* Copy kernelVAddr into eax */
	movl %ebx, %eax
	addl $0+kernelVAddr-APBoot, %eax
	movl (%eax), %eax

	/* Copy kernelPAddr into ecx */
	movl %ebx, %ecx
	addl $0+kernelPAddr-APBoot, %ecx
	movl (%ecx), %ecx

	/* Copy kernelSize into edx */
	movl %ebx, %edx
	addl $0+kernelSize-APBoot, %edx
	movl (%edx), %edx

	/* Keep kernelVAddr - kernelPAddr into eax */
	/* Use symbol - eax to get physical_symbol */
	/* ecx discarded */
	subl %ecx, %eax

	# symbol APSetupRuntime (phys) calculated
	# by (APSetupRuntime - halLoadAddr) + halLoadPAddr

	movl $APSetupRuntime, %ecx
	subl %eax, %ecx

	/* eax is passed as register arg (ebx should be discarded) */
	jmp %ecx

/**
  apSetupInfo is a TrampolineData struct. It contains the data
  required to locate kernel symbols in physical memory space.
 */
.align 8
apSetupInfo:

/**
  Boot-time GDT used by default for the kernel. Note that this
  is also referenced by the KernelHost for BSP setup.

  NOTE: Alternative GDT setups are not yet supported. May be
  required in the future.

  @see KernHost/Boot/Entry.S (IA32)
 */
.globl defaultBootGDT
defaultBootGDT:
defaultBootGDTStart:
	.quad 0x0000000000000000

    .word 	0xFFFF
	.word	0x0000
	.byte 	0x00
    .byte 	0x9A
    .byte   0xCF
	.byte 	0x00

	.word 	0xFFFF
	.word 	0x0000
	.byte 	0x00
	.byte 	0x92
	.byte 	0xCF
	.byte 	0x00
defaultBootGDTEnd:
		
/**
  Used during debugging to test whether the AP (in dual-CPU mode)
  has woken up or not. Note that this field is shared by all the
  APs.

  @deprecated
 */
apBootStatusRegister: .word STATUS_INIT

/**
  Boot GDT pointer used in APRealModeInit. Fields are computed at
  runtime.
 */
.globl defaultBootGDTPointer
defaultBootGDTPointer:
	.word GDT_SIZE - 1
	.long 0 /* calculated by (CS=DS)*16+defaultBootGDT-APBoot */

kernelVAddr:.long 0
kernelPAddr:.long 0
kernelSize:	.long 0
apPDPTAddr:	.long 0

/* As a far/long jump after enabling the PM bit must be computed at
   runtime, this register will contain the destination address.
 */
toPMJumpAddressRegister:
	toPMJumpOffset: .long 0
	toPMJumpSelector: .word 0

/* End of copied trampoline */
APBootSequenceEnd:
		
#define AP_MAIN32_INVOKER APInvokeMain32 -0xC0000000
#define KCPUINFO (0xC0000000 + 20 << 20)

.align 4
.extern APInvokeMain32 /* moved to InitRuntime.asm (needed a fixed load address) */
.extern APMain
.extern VAPICBase
.extern APEarlyMain
.extern SetupPageProtectionWithoutStack
/**
  Setups the runtime environment for the AP. It must enable pagination and
  locate the global processor stack. It then goes to APMain.

  @param eax=*kernelVAddr-*kernelPAddr
  @param edx=*kernelSize
 */
APSetupRuntime:
	/* Copy apPDPTAddr into edi (from trampoline, using ebx base) */
	movl $0+apPDPTAddr-APBoot, %edi
	addl %ebx, %edi
	movl $589824+apPDPTAddr-APBoot, %edi
	movl (%edi), %edi

	/* Required virtual address */
	movl $.LocateStackAndCallAPMain, %ebx

	/* args edi and ebx */
	movl $SetupPageProtectionWithoutStack, %esi
	subl %eax, %esi
	jmp %esi

.LocateStackAndCallAPMain:
	movl (VAPICBase), %eax
	movl 20(%eax), %eax
	shrl $9, %eax /* eax = PROCESSOR_ID * PAGE */

	movl $KCPUINFO, %ebx
	addl %eax, %ebx
	movl 28(%ebx), %esp

	call APMain
/*** NO RETURN ***/

/*=========================================================
 ********** Application Processor Synchronization **********

 Contains code to wait until the BSP unlocks the permit-lock
 to allow APs to start their schedulers.
 ==========================================================*/

.extern apPermitLock
.globl APWaitForPermit
APWaitForPermit:
		push %edx			/* save EDX on stack as we are using it */
		apTryForPermitAgain: /* loop until we get the permit */
			pause			/* pause - as per spin-lock semantics */
			pause
			pause
			movl (apPermitLock), %edx	/* load the apPermitLock for checking */
			cmpl $0, %edx		/* check if the lock was granted */
			jne apGotPermit		/* goto apGotPermit if lock was granted */
			je apTryForPermitAgain	/* otherwise, loop again */
			
		apGotPermit:
			push %eax
			push %ecx
			movl 16(%esp), %ecx
			rdtsc
			movl (%ecx), %eax
			movl 4(%ecx), %edx
			pop %ecx
			pop %eax
			pop %edx /* restore EDX as we are done with it */
			ret

.globl BSPGrantPermit
BSPGrantPermit:
	movl $1, (apPermitLock)
	sfence
	ret

.globl SpinLock
SpinLock:
	push %eax
	push %ecx
	movl 12(%esp), %eax
	SpinLoop:
		pause
		movl $1,%ecx
		xchg %ecx, (%eax)
		cmp $0, %ecx
		je SpinLoop
	mfence
	pop %ecx
	pop %eax
	ret
		
.globl SpinUnlock
SpinUnlock:
	push %eax
	movl 8(%esp), %eax
	movl $0, (%eax)
	mfence
	pop %eax
	ret
