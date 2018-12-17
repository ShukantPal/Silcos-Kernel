# KernelHost

## Origin

The microkernel was split into further modules to make development easier and
to isolate the critical scheduling, threading, and IPC code from the binary
which also was bloated by boot-code, HAL, memory-management, module-loading
and so on.

**The boot-time, memory-management and module-loading binary has become the KernelHost since then**.

Why? This is because boot-time code must run first
and then memory-management & module-loading are so primitive that nothing
else can run without them.

### Compilation

The makefile in this folder will only compile the kernel host. It will split
all object files into their categories and put them in their respective
folders, unlike before where all object files were in one Compile folder.

The testing & building **main makefile has been shifted to parent folder**.
