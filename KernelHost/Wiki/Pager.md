# Pager

The pager of the Silcos kernel tries to map virtual address to physical address in the most optimized manner.

## IA32

Currently, the Silcos IA32 KernelHost supports only the PAE-mode of paging. It is expected that the legacy paging feature will be supported in the near future.

### PAE-Mode

The kernel boots up with the `PDPT`, `GlobalDirectory`, `GlobalTable` which are the only page-structs used while initializing the memory & pre-boot subsystems. While booting the `GlobalDirectory` is mapped to the 0th and 3rd entries in the PDPT (the 0th entry is required to implement identity paging while switching to paging mode). Similarly, the `GlobalTable` is mapped to the last entry in the `GlobalDirectory`.

To efficiently access and modify these structures, the kernel makes use of **recursive mapping**.

i) The 507th, 508th, 509th, and 510th entries of the `GlobalDirectory` are actually mapped to the current PDPT's directory entries as huge pages. This means the associated addresses can be used to alter page-tables of the respective directory.

ii) The 511th entry of the `GlobalDirectory` is mapped to the `GlobalTable` to make use of 4-kilobyte pages while booting. Therefore, the 508th, 509th, 510th and 511th entires in the `GlobalTable` are mapped to the directories of the current PDPT. That means the associated addresses can be used to alter the page-directories of the current PDPT.

iii) Note that `GlobalDirectory` and `GlobalTable` are also available for modification using the symbols in the kernel. But the pager doesn't use them while mapping because of the overhead of simply using and `if` statement to use the symbols when the given address comes under their scope. The symbols are only used during bootup.

See `KernelHost/Source/IA32/Paging/86InitPaging.asm` for PAE-mode initialization code.

See `KernelHost/Source/IA32/Paging/Pager.cpp` for the PAE-mode mapping code.

### Continous Page-Tables

As the page-directories are continously mapped in the 507th to 510th entry, the page-tables can be accessed contiously in the respective addresses. This provides great benefit as the indices for the page-table are continous.