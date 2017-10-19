; Copyright (C) 2017 - Shukant Pal

section .text ; ---------------------

	; ---------------------------

	; global
	;
	; KernelEnter - 
	;
	; Handles a system call through a interrupt
	; gate. The thread exits from the intr context
	; and comes into kernel mode.
	;
	; Syscalls are not preemptible. Thus, the PreemptFlag
	; of the PoC is set.
	;
	; @Version 1
	; @Since Circuit 2.01
	global KernelIntrEnter
	extern VAPICBase
	extern CtPoC
	KernelIntrEnter:
		mov eax, [VAPICBase]
		mov eax, [eax + 0x20]

		push esi
		push ecx
		push eax
		call CtPoC
		add esp, 4
		pop ecx
		pop esi

		mov edi, 0
		xchg [eax + 48], edi ; No Preemption

		mov edi, [esp]  ; Save EIP (in a register that
				;	    is saved in scheduler
				;	    a tick)

		cmp dword [esp + 4], 0x8
		je EnterFromKernel

		; internal
		EnterFromUser:
		add esp, 8
		popfd
		mov esp, [esp]
		jmp StackRestore

		; internal
		EnterFromKernel:
		mov dword [esp], SimplyKernel
		iret		

	; ---------------------------

	StackRestore:
		push eax
		mov ax, 0x10
		mov ds, ax
		mov ss, ax
		pop eax

		jmp SysBranch

	; ---------------------------

	SimplyKernel:
		push edi

	; ---------------------------

	extern BranchTable
	SysBranch:
		mov edi, 1 ; Used to exchange & allow preemption
		imul ebx, 4
		add ebx, BranchTable
		jmp [ebx]

	; ----------------------------

	global SyTID
	extern XTID
	SyTID:
		xchg [eax + 48], edi ; Can Preempt
		mov eax, [eax + 24] ; Caller Thread
		mov eax, [eax]
		jmp KernelIntrExit

	; ----------------------------

	global SyCreateThread
	extern XCreateThread
	SyCreateThread:
		push eax

		push esi
		push ecx
		push ebp
		push dword [eax + 24]
		call XCreateThread
		xchg bx, bx
		mov ebx, eax
		xchg bx, bx
		add esp, 16

		pop eax
		jmp SyscallSched

	; ----------------------------

	global SyExitThread
	extern XExitThread
	extern TTable
	SyExitThread:
		push eax

		push esi
		push dword [eax + 24]
		call XExitThread
		add esp, 8
		
		mov eax, [esp]
		mov eax, [eax + 24]
		mov ebx, 0x6
		xchg [eax + 32], ebx
		pop eax

		jmp SyscallSched

	; ----------------------------

	global SyWaitForThread
	extern XWaitForThread
	SyWaitForThread:
		push eax
		push esi
		push dword [eax + 24]
		call XWaitForThread
		add esp, 8

		pop eax
		jmp SyscallSched

	; ----------------------------

	global SySleepThread
	extern XSleepThread
	SySleepThread:
		push eax

		push esi ; Higher 32-bits of SleepTime
		push ecx  ; Lower 32-bits of SleepTime
		push dword [eax + 24]
		call XSleepThread
		add esp, 12

		pop eax
		jmp SyscallSched

	; ----------------------------

	global SyWaitForThreadTill
	extern XWaitForThreadTill
	SyWaitForThreadTill:
		push eax

		push esi
		push ecx
		push ebp
		push dword [eax + 24]
		call XWaitForThreadTill
		add esp, 16

		pop eax
		jmp SyscallSched

	; ----------------------------

	global SyDeleteThread
	extern XDeleteThread
	SyDeleteThread:
		push eax

		push esi
		push dword [eax + 24]
		call XDeleteThread
		mov ebx, eax
		add esp, 8

		pop eax
		jmp SyscallSched

	; ----------------------------

	global SyStopThread
	extern XStopThread
	SyStopThread:
		push eax

		push esi
		push ecx
		push ebp
		push dword [eax + 24]
		call XStopThread
		add esp, 16

		pop eax
		jmp SyscallSched

	; ----------------------------

	global SyWakeupThread
	extern XWakeupThread
	SyWakeupThread:
		push eax

		push esi
		push dword [eax + 24]
		call XWakeupThread
		add esp, 8

		pop eax
		jmp SyscallSched

	; ----------------------------

	; global
	; KernelIntrExit - 
	;
	; This exits the kernel, without invoking the scheduler. It
	; is used when a fast-syscall (like SyTID) is executed. This
	; reduces number of scheduling bursts on multiple CPU
	; systems.
	;
	; Few syscalls, which manipulate the Exec subsystem
	; structures, should never use this. The scheduler must
	; be invoked after the syscall.
	;
	; @Version 1
	; @Since Circuit 2.01
	KernelIntrExit:
		mfence
		
		ExitToKernel:
			pop edi
			xchg bx, bx
			jmp edi

	; ----------------------------

	; global
	; SyscallSched -
	;
	; Exit to the scheduler. Primarily used when manipulating
	; the Exec subsytem.
	;
	global SyscallSched
	extern CtPoC
	extern IvkSched
	extern ExtSched
	SyscallSched:
		pop edi
		mov eax, [eax + 24]
		mov [eax + 36], edi
		mov [eax + 44], esp
		LOCK sub dword [eax + 44], 28

		push ebx
		mov edx, [VAPICBase]
		mov edx, [edx + 0x20]
		push edx
		call CtPoC
		mov edx, eax
		add esp, 4
		pop ebx

		jmp IvkSched

	; ----------------------------

	global TTID
	TTID:
		mov ebx, 0
		int 150
		ret

	; ----------------------------

	global TCreateThread
	TCreateThread:
		push ebp
		mov esi, [esp + 16]
		mov ecx, [esp + 12]
		mov ebp, [esp + 8]
		mov ebx, 1
		int 150
		pop ebp
		xchg bx, bx
		mov eax, ebx ; TID
		xchg bx, bx
		ret

	; ----------------------------

	global TExitThread
	TExitThread:
		mov esi, [esp + 4]
		mov ebx, 2
		int 150
		ret

	; ----------------------------

	global TWaitForThread
	TWaitForThread:
		mov esi, [esp + 4]
		mov ebx, 3
		int 150
		ret

	; ----------------------------

	global TSleepThread
	TSleepThread:
		mov esi,[esp + 8]
		mov ecx, [esp + 4]
		mov ebx, 4
		int 150
		ret

	; ----------------------------

	global TWaitForThreadTill
	TWaitForThreadTill:
		mov esi, [esp + 12]
		mov ecx, [esp + 8]
		push ebp
		mov ebp, [esp + 8]
		mov ebx, 5
		int 150
		pop ebp
		ret

	; ----------------------------

	global TDeleteThread
	TDeleteThread:
		mov esi, [esp + 4]
		mov ebx, 6
		int 150
		ret

	; ----------------------------

	global TStopThread
	TStopThread:
		mov esi, [esp + 12]
		mov ecx, [esp + 8]
		push ebp
		mov ebp, [esp + 8]
		mov ebx, 7
		int 150
		pop ebp
		ret

	; ----------------------------

	global TWakeupThread
	TWakeupThread:
		mov esi, [esp + 4]
		mov ebx, 8
		int 150
		ret

	; ----------------------------

; ------------------------------------
