# IOAPIC

IO Advanced Programmable Interrupt Controller, or IOAPIC, is a device that is used to route PCI & ISA interrupts from peripheral devices to individual processors. This devices helps SMP systems to distribute interrupts amongst all the processors in the system. It is the central component in lieu of programming basic hardware.

The IOAPIC has an array of input signals, defined by 24 redirection entries for each input. IO devices trigger interrupts in the system by asserting an input pin the IOAPIC. The IOAPIC finds the corresponding entry in the redirection table, and uses that to format a interrupt message to the local APIC of a processor. This table is programmable in the kernel.

To access IOAPIC functionality, add the following code:

    #include <HAL/IOAPIC.hpp>