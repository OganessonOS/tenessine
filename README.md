# Tenessine Kernel
Tenessine is the kernel Oganesson runs on.

# Design Goals
For code simplicity reasons, Tenessine will not attempt to be a "normal" microkernel. However, Tenessine will aim to combine aspects from both monolithic- and microkernels (hybrid kernel), so that the advantages of both can be exploited. In this vain, the following will remain in-kernel:
- VFS
- "Basic" drivers such as IDE, AHCI, NVMe...
- Backends on which more complicated drivers (aka drivers with a bigger potential to have bugs in them) can communicate with hardware while remaining in user-space
  - Maybe revive NDI?
  - This will mean seperate "classes" of processes, which would also allow other interesting features in the future.

At this point, I have not yet fully decided whether I will implement a standard POSIX system in-kernel.
Additionally, Tenessine will also aim to remain extremly modular, so that features can be easily added, removed, or ported to other systems.
