# IOAPIC

The `HardwareAbstraction/IOAPIC.hpp` header defines the methods by which software can control an **I/O APIC**. As there may be multiple I/O APICs present in the system, the kernel stores in sequentially in an permanent `ArrayList` in the `IOAPIC::systemIOAPICs` field which is loaded during boot-up only.

# HAL::IOAPIC

The `IOAPIC` class is the driver class for IOAPIC class interrupt controllers.

## IOAPIC::IOAPIC

**Prototype:** `IOAPIC::IOAPIC(unsigned long regBase, unsigned long intrBase)`

**Description:**

This is the constructor for the IOAPIC driver which initializes internal variables. It maps the memory-mapped registers of the given I/O APIC at regBase to an exclusively allocated kernel-memory page.

**Parameters:**

`unsigned long regBase` - This is base-address of the I/O APIC in physical memory. It is mapped to kernel-memory.

`unsigned long intrBase` - This is the global system interrupt base for this I/O APIC. The first interrupt routed to this I/O APIC will be at this offset in the `IOAPIC::systemIOAPICInputs` array-list.

# IOAPIC::registerIOAPIC()

**Prototype:** `void IOAPIC::registerIOAPIC(MADTEntryIOAPIC *ioaEnt)

**Description:**

Adds the IOAPIC into the set of recognized interrupt-controllers and initializes it. It is added to the `systemIOAPICs` array-list in the process.

The interrupt-inputs of this IOAPIC will also be assigned to each processor serially. Once all processors have been assigned interrupt-inputs from IOAPICs then the next vector is used and so one. For example, if there are two CPUs and one IOAPIC with 24 inputs then the mappings will be done as follows

    CPU                  Input               Vector
    0                    0                   32
    1                    1                   32
    0                    2                   33
    1                    3                   33
    0                    4                   34
    1                    5                   34

and so on.

**Parameters:**

`MADTEntryIOAPIC *ioaEnt` - This is the IOAPIC entry in the multiple APIC description table parsed by the ACPI subsystem. It holds the IOAPIC's register-base and global system interrupt base.