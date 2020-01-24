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
#include <portio.h>
#include <pic.h>
#include <timer.h>
#include <time.h>
#include <init.h>
#include <kybrd.h>
#include <thread.h>

#define KINIT_STATUS_SUCCESSFUL_FAILURE 0
#define KINIT_STATUS_STANDING_BY 1

void system_log_status(char* module_name, int status){
    printf("[Kernel Init] %s status: ", module_name);
    if(status == KINIT_STATUS_STANDING_BY){
        video_packed_color_t color = video_get_packed_color();
        video_set_foreground(light_green);
        printf("Standing by\n");
        video_set_packed_color(color);
    }
    if(status == KINIT_STATUS_SUCCESSFUL_FAILURE){
        video_packed_color_t color = video_get_packed_color();
        video_set_foreground(red);
        printf("Successful failure\n");
        video_set_packed_color(color);
    }
}

void system_earlyinit(uint64_t physinfo){
    video_init();
    printf("[Kernel Init] Kernel identity paging initialized\n");
    printf("[Kernel Init] SSE enabled\n");
    system_log_status("GDT", KINIT_STATUS_STANDING_BY);
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
    while(!boot_is_terminator(tag)){
        if(tag->type == BOOT_MEMORY_MAP_TAG_ID){
            memory_map_entries_count = boot_get_memory_map_entries_count(tag);
            entries = boot_get_memory_map_entries(tag);
        }
        if(tag->type == BOOT_ELF_SECTIONS_TAG_ID){
            uint32_t sectionCount = boot_get_section_headers_count(tag);
            boot_elf_section_header_t* headers = boot_get_section_headers(tag);
            for(uint32_t i = 0; i < sectionCount; ++i){
                uint64_t physaddr = headers[i].offset;
                if(physaddr > KERNEL_MAPPING_BASE){
                    physaddr -= KERNEL_MAPPING_BASE;
                }
                if(physaddr == 0){
                    continue;
                }
                if(headers[i].section_size == 0){
                    continue;
                }
                if(physaddr < kernel_begin){
                    kernel_begin = physaddr;
                }
                if((physaddr + headers[i].section_size) > kernel_end){
                    kernel_end = physaddr + headers[i].section_size;
                }
            }
        }
        if(tag->type == BOOT_MODULE_TAG_ID){
            ramdisk_begin = boot_get_module_physical_base(tag);
            ramdisk_end = boot_get_module_physical_limit(tag);
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
    if(multiboot_begin < phinfo.preserved_area_physical_base){
        phinfo.preserved_area_physical_base = multiboot_begin;
    }
    if(ramdisk_begin < phinfo.preserved_area_physical_base){
        phinfo.preserved_area_physical_base = ramdisk_begin;
    }
    if(ramdisk_end > phinfo.preserved_area_physical_limit){
        phinfo.preserved_area_physical_limit = ramdisk_end;
    }
    phinfo.mmap_entries_count = memory_map_entries_count;
    phinfo.mmap_entries = entries;
    kheap_init(phinfo);
    system_log_status("Kernel Heap", KINIT_STATUS_STANDING_BY);
    phmmngr_init(phinfo);
    system_log_status("Physical allocator", KINIT_STATUS_STANDING_BY);
    splitter_init();
    system_log_status("Path splitter", KINIT_STATUS_STANDING_BY);
    idt_init();
    system_log_status("IDT", KINIT_STATUS_STANDING_BY);
    ksbrk_init();
    system_log_status("Kernel sbrk", KINIT_STATUS_STANDING_BY);
    pic_init();
    system_log_status("PIC", KINIT_STATUS_STANDING_BY);
    timer_init(1000);
    system_log_status("Timer 100Hz", KINIT_STATUS_STANDING_BY);
    kybrd_init();
    system_log_status("PS/2 Keyboard", KINIT_STATUS_STANDING_BY);
    thread_init_subsystem();
    system_log_status("Scheduler", KINIT_STATUS_STANDING_BY);
    printf("[Kernel Init] Kernel basic services initialized. Unmasking irqs.\n");
    timer_enable();
    kybrd_enable();
    printf("[Kernel Init] Fully initalized. Calling system_init\n");
    time_sleep(1000);
    video_clear_screen();
    system_init();
    while(1) asm("pause");
}