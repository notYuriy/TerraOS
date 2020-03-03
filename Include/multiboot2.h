#ifndef __MULTIBOOT2_H_INCLUDED__
#define __MULTIBOOT2_H_INCLUDED__

#include <utils.h>
#include <video.h>

//structure layouts taken from https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html

#define BOOT_MEMORY_MAP_TAG_ID 6
#define BOOT_ELF_SECTIONS_TAG_ID 9
#define BOOT_MODULE_TAG_ID 3
#define BOOT_MEMORY_MAP_AVAILABLE 1
#define BOOT_ACPI_OLD_RSDP_TABLE 14
#define BOOT_ACPI_NEW_RSDP_TABLE 15

typedef struct boot_info_struct {
    uint32_t total_size;
    uint32_t reserved;
} __attribute__((packed)) boot_info_t;

typedef struct boot_tag_struct {
    uint32_t type;
    uint32_t size;
} __attribute__((packed)) boot_tag_t;

typedef struct boot_memory_map_tag_struct{
    boot_tag_t base;
    uint32_t entry_size;
    uint32_t entry_version;
} __attribute__((packed)) boot_memory_map_tag_t;

typedef struct boot_memory_map_entry_struct {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed)) boot_memory_map_entry_t;

typedef struct boot_elf_sections_tag_struct {
    uint16_t num;
    uint16_t entsize;
    uint16_t shndx;
    uint16_t reserved;
    //It requires strange 4 bytes alignment. Do not ask me why. IDK
    uint32_t align;
} __attribute__((packed)) boot_elf_sections_tag_t;

typedef struct boot_module_tag_struct{
    boot_tag_t base;
    uint32_t start;
    uint32_t end;
} __attribute__((packed)) boot_module_tag_t;

//section header layout from http://man7.org/linux/man-pages/man5/elf.5.html

typedef struct boot_elf_section_header_struct {
    uint32_t section_name;
    uint32_t section_type;
    uint64_t section_flags;
    uint64_t addr;
    uint64_t offset;
    uint64_t section_size;
    uint32_t section_link;
    uint32_t section_info;
    uint64_t section_addr_align;
    uint64_t section_ent_size;
} __attribute__((packed)) boot_elf_section_header_t;

inline boot_tag_t* boot_go_to_next_entry(boot_tag_t* current){
    return (boot_tag_t*)(
        up_align((uint64_t)((char*)current + current->size), 8)
    );
}

inline boot_tag_t* boot_get_first_tag(boot_info_t* begin){
    return (boot_tag_t*)(begin + 1);
}

inline bool boot_is_terminator(boot_tag_t* tag){
    return tag->type == 0;
}

inline boot_memory_map_entry_t* boot_get_memory_map_entries(boot_tag_t* tag){
    boot_memory_map_tag_t* mmaptag = (boot_memory_map_tag_t*)tag;
    return (boot_memory_map_entry_t*)(mmaptag + 1);
}

inline uint32_t boot_get_memory_map_entries_count(boot_tag_t* tag){
    boot_memory_map_tag_t* mmaptag = (boot_memory_map_tag_t*)tag;
    return (mmaptag->base.size - sizeof(boot_memory_map_tag_t)) 
    / sizeof(boot_memory_map_entry_t);
}

inline boot_elf_section_header_t* boot_get_section_headers(boot_tag_t* tag){
    boot_elf_sections_tag_t* elfstag = (boot_elf_sections_tag_t*)tag;
    uint64_t addr = (uint64_t)(elfstag + 1);
    return (boot_elf_section_header_t*)addr;
}

inline uint32_t boot_get_section_headers_count(boot_tag_t* tag){
    boot_elf_sections_tag_t* elfstag = (boot_elf_sections_tag_t*)tag;
    return (uint32_t)elfstag->num;
}

inline uint64_t boot_get_module_physical_base(boot_tag_t* tag){
    boot_module_tag_t* moduletag = (boot_module_tag_t*)tag;
    return (uint64_t)(moduletag->start);
}

inline uint64_t boot_get_module_physical_limit(boot_tag_t* tag){
    boot_module_tag_t* moduletag = (boot_module_tag_t*)tag;
    return (uint64_t)(moduletag->end);
}

inline char* boot_get_module_name(boot_tag_t* tag){
    boot_module_tag_t* moduletag = (boot_module_tag_t*)tag;
    char* result = (char*)(moduletag + 1);
    return (char*)result;
}

#endif