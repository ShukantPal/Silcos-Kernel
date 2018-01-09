# HAL

The kernel from Silcos 3.02 has become homogenous in the matter of kernel
modules. It has further divided the legacy microkernel in further modules
which are KernelHost, HAL, and the ExecutionManager.

## Functions

The HAL abstracts the underlying hardware and creates platform-independence for
other boot modules. All of its code is dependent on the architecture and
provides all services other than boot-strap code for the booting processor.