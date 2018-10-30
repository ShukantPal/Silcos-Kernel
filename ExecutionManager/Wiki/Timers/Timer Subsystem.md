# Kernel Timers

Silcos kernel has a class hierarchy for the timer subsystem which starts at the `TimerDevice` class. This
class already has the boilerplate details common to all timer objects, which includes the following properties:

1. `InvocationMode`: Timer objects can be in one of three defined modes - `TDIM_EXTERNAL`, `TDIM_INTERRUPT`, `TDIM_LOCKED`. The details are explained later on in this article.

2. `IntId`: This property holds an identifier that represents how the timer generates interrupt. It is used by the timer subclasses.

3. `ExternalInvocationLock`: This is a `Lockable` used to prevent a timer object from being accessed
simultaneously and coming into the `TDIM_LOCKED` mode.

## Invocation Modes

### 1. TDIM_EXTERNAL

`TDIM_EXTERNAL` is an "idle" mode of the object, where it is not doing any operation. This means that the object is not writing to the event-queues held by it.

### 2. TDIM_INTERRUPT

`TDIM_INTERRUPT` mode occurs when the object is handling an event that just occured in an interrupt context. No other accesses should be made during this mode.

### 3. TDIM_LOCKED

`TDIM_LOCKED` occurs when external kernel code calls the timer object to do some event-related operation, such as adding a event to occur in the future, or removing an event from being fired.

## Contract to kernel-timer objects

Events that are pending can occur any time. Hence, to prevent the `TDIM_INTERRUPT` mode from turning on, while the timer object is doing another operation, interrupts must be turned off. This only works if the executing CPU will receive the timer interrupts. Hence, to prevent `TDIM_INTERRUPT` mode, when `TDIM_LOCKED` mode is already set, the timer object must actually transfer the operation to the CPU receiving the timer interrupts.

This is done using `TimerOperation` objects. The timer-object initializes an `TimerOperation` using the factory methods `packageNotifyAfter` and `packageCancelEvent`. Then using the proper method, the request should be sent to the CPU recieving the interrupts (either by `CPUDriver::sentRequest` for direct interrupt [FSB] or `IOAPIC::carryRequestToCPU` for IOAPIC aided interrupts).

# API for kernel-timers

Note that `TimerDevice`, `HardwareTimer`, `PIT`, `HPET`, etc. are all timer-internal APIs. They are not intended for actually scheduling events. An external wrapper API is going to build (named `System.Time`) for doing so. The reasons for doing so are:

1. `TimerOperation`s that are sent to another CPU must return the `Event` object actually formed. This requires either a new `IPIRequest` to carry it back, or some sort of locking to hold the other CPU until the operation is done. The external wrapper will handle this.