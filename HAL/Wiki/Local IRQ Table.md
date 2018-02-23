# Introduction

Since Silcos 3.02, `IRQHandler` records are kept in an `ArrayList` for 192 interrupts in the local APIC on each for each processor (see Legacy IRQ's for information on handling the programmable interrupt controller). This has become a need as -

1.
After the arrival of message-signalled interrupts, it has become neccesary to keep track of irq-handlers for each vector in the local APIC. Devices may bypass the IOAPIC and directly assert interrupts on the local APIC of any CPU.

2.
A group of device IRQs may originate from a external interrupt on the IOAPIC. If the IOAPIC driver decides to map a IRQ to another CPU then it would be expensive to move all the interrupt-handlers separately. Further more, it could cause race conditions if a interrupt fires while moving.

## Interrupt Mapping

The IDT maps interrupts as follows - 

i) **0 - 31** are reserved for exeception and error handling.

ii) **32 - 223** are used for device IRQs and drivers may register there IRQ-handlers in this range.

iii) **224 - 255** are reserved for system usage. See the Kernel Interrupt Docs for more info (ToDo: make them :))