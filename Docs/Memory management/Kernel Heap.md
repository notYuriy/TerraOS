Kernel Heap is essential for Terra OS. It begins with 64 MB of memory, provides memory for physical memory manager and kernel itself.

Kernel Heap is defined in files kheap.c/kheap.h. Allocation algorithm is straightforward: iterate all memory areas and return the first suitable area.

Sbrk, which is a method for resizing kernel heap, is implemented in files ksbrk.c / ksbrk.h. It allocates 4 KB pages for the heap use at the end of kernel memory.
Allocation of aligned memory is also possible. After rewriting boot.s to boot from 4 KB pages, it would be possible to do

    char* pages = kheap_malloc_aligned(4096 * PAGE_SIZE, 4096);

This will allocate PAGE_SIZE pages, which then can be mapped using vmcore to some physical addresses (this is useful for things like ACPI)