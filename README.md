# TerraOS
Experimental 64 bit operating system.

Printf is used from https://github.com/mpaland/printf. 
Based on these great osdev tutorials: https://os.phil-opp.com/first-edition/ 
(first edition is less dependent on Rust)

Goals
Completed:
* Long mode
* Higher half kernel
* Physical Memory Management
* Virtual Memory Management
* Kernel Heap
* Kernel sbrk
* Interrupts
* Spinlocks for thread-safety
* PIC
* Timer support
* Keyboard support (not thread safe)

Not completed:
* Filesystem
* Threading
* Userspace
* Elf parsing/loading/executing
* Newlib port for userspace
