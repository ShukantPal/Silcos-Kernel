# BootService

The boot service provides the hardware-dependent functionality of communicating
with bootloader, initializing the hardware abstraction layer, setting up the
C++ runtime, doing any pre-boot functions, and booting the application
processors.

## Folder - IA32/Boot

This folder contains the **BootService** code. The **Main.cpp** and
**InitRuntime.asm** files are very important. This folder contains the code for
multiboot information parsing, calling C++ global object constructors, and then
the Main() function.

### File - Main.cpp

This file contains the `Main()` function. It initializes all subsystems - ACPI
tables, pageframe-allocator, page-allocator, slab-allocator(s), processor
topology, application processors, schedulers, etc.

In short, it will boot the kernel into a mode that will allow the operating
system software to takeover initialization.

### File - InitRuntime.asm

The symbol **InitRuntime_** is the alias for **InitRuntime**. It is the entry
point for the kernel.

**InitRuntime_** provides the pre-boot functionality. It assumes that the
multiboot environment is setup for it and 32-bit protected mode is enabled with
uniform logical address space.

1. Turns off interrupts for safety.

2. It will load the default boot-time GDT and segment selectors.

3. Paging is enabled by calling into the HAL (@File 86InitRuntime.asm). This
   enables the higher-half kernel at 3GB.
 
4. Before calling **InitPaging**, EDX is set to **InitEntrance** which is the
alias for **InitEnvironment**. This is done so **InitPaging** returns to the
function **InitEnvironment**.

5. **InitEnvironment** will load the stack-register and call the global object
constructors. Then it initializes the **multiboot-parsing** subsystem and then
calls **Main()**.

6. Incase Main() returns, a 0x20 interrupt is called to execute a **scheduler-
tick**.