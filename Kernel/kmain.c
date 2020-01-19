#include <video.h>
#include <utils.h>
#include <multiboot2.h>
#include <phmmngr.h>
#include <vmcore.h>
#include <kheap.h>
#include <pathsplit.h>
#include <idt.h>
#include <spinlock.h>
#include <ksbrk.h>
#include <timer.h>
#include <portio.h>

void kmain(uint64_t physinfo){
    video_init();
    printf("[Kernel Init] Kernel identity paging initialized\n");
    printf("[Kernel Init] SSE enabled\n");
    printf("[Kernel Init] GDT status: Standing by\n");
    printf("[Kernel Init] Kernel is now in long mode\n");
    printf("[Kernel Init] Kernel is now mapped to the higher half\n");
    printf("[Kernel Init] Cache invalidated for the initial mapping\n");
    printf("[Kernel Init] C kernel entry point called\n");
    printf("[Kernel Init] Video initialized\n");
    boot_info_t* info = (boot_info_t*)(physinfo + KERNEL_MAPPING_BASE);
    printf("[Kernel Init] Examining multiboot2 boot information\n");
    boot_tag_t* tag = boot_get_first_tag(info);
    uint64_t kernel_begin = (uint64_t)-1;
    uint64_t kernel_end = 0;
    uint64_t multiboot_begin = physinfo;
    uint64_t multiboot_end = multiboot_begin + info->total_size;
    uint64_t ramdisk_begin = 0;
    uint64_t ramdisk_end = 0;
    uint32_t memory_map_entries_count = 0;
    boot_memory_map_entry_t* entries = NULL;
    uint64_t old_rsdp_address = 0;
    uint64_t new_rsdp_address = 0;
    bool is_new_rsdp = false;
    while(!boot_is_terminator(tag))
    {
        if(tag->type == boot_memory_map_tag_id)
        {
            memory_map_entries_count = boot_get_memory_map_entries_count(tag);
            entries = boot_get_memory_map_entries(tag);
        }
        if(tag->type == boot_elf_sections_tag_id)
        {
            uint32_t sectionCount = boot_get_section_headers_count(tag);
            boot_elf_section_header_t* headers = boot_get_section_headers(tag);
            for(uint32_t i = 0; i < sectionCount; ++i)
            {
                uint64_t physaddr = headers[i].offset;
                if(physaddr > KERNEL_MAPPING_BASE){
                    physaddr -= KERNEL_MAPPING_BASE;
                }
                if(physaddr == 0)
                {
                    continue;
                }
                if(headers[i].section_size == 0)
                {
                    continue;
                }
                if(physaddr < kernel_begin)
                {
                    kernel_begin = physaddr;
                }
                if((physaddr + headers[i].section_size) > kernel_end)
                {
                    kernel_end = physaddr + headers[i].section_size;
                }
            }
        }
        if(tag->type == boot_module_tag_id)
        {
            ramdisk_begin = boot_get_module_physical_base(tag);
            ramdisk_end = boot_get_module_physical_limit(tag);
        }
        if(tag->type == boot_acpi_old_rsdp_table)
        {
            old_rsdp_address = (uint64_t)(tag + 1);
            printf("Old rsdp address: %llp\n", old_rsdp_address);
        }
        if(tag->type == boot_acpi_new_rsdp_table)
        {
            new_rsdp_address = (uint64_t)(tag + 1);
            is_new_rsdp = true;
            printf("New rsdp address: %llp\n", new_rsdp_address);
        }
        tag = boot_go_to_next_entry(tag);
    }
    kernel_begin = down_align(kernel_begin, 4096);
    kernel_end = up_align(kernel_end, 4096);
    multiboot_begin = down_align(multiboot_begin, 4096);
    multiboot_end = up_align(multiboot_end, 4096);
    ramdisk_begin = down_align(ramdisk_begin, 4096);
    ramdisk_end = up_align(ramdisk_end, 4096);
    phmeminfo_t phinfo;
    phinfo.preserved_area_physical_base = kernel_begin;
    phinfo.preserved_area_physical_limit = kernel_end;
    if(multiboot_begin < phinfo.preserved_area_physical_base)
    {
        phinfo.preserved_area_physical_base = multiboot_begin;
    }
    if(ramdisk_begin < phinfo.preserved_area_physical_base)
    {
        phinfo.preserved_area_physical_base = ramdisk_begin;
    }
    if(ramdisk_end > phinfo.preserved_area_physical_limit)
    {
        phinfo.preserved_area_physical_limit = ramdisk_end;
    }
    phinfo.mmap_entries_count = memory_map_entries_count;
    phinfo.mmap_entries = entries;
    kheap_init(phinfo);
    printf("[Kernel Init] Kernel Heap status: Standing by\n");
    kheap_traverse();
    phmmngr_init(phinfo);
    printf("[Kernel Init] Physical memory manager status: Standing by\n");
    splitter_init();
    printf("[Kernel Init] Path splitter status: Standing by\n");
    idt_init();
    printf("[Kernel Init] IDT status: Standing by\n");
    ksbrk_init();
    printf("[Kernel Init] Ksbrk standing by\n");
    printf("=(\n");
    timer_init(1000);
}