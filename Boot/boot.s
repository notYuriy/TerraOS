; taken from phill-opp higher half github demo

global start
extern system_earlyinit
global p2_table

KERNEL_MAPPING_BASE equ 0xffff800000000000

section .inittext
bits 32
start:

        mov esi, ebx
        mov esp, stack_top - KERNEL_MAPPING_BASE  ; set up stack
        mov edi, ebx

        call check_multiboot
        call check_cpuid
        call check_long_mode

        call setup_page_tables
        call enable_paging

        call check_sse
        call check_apic
        call enable_sse

        lgdt [gdt64.pointer_low - KERNEL_MAPPING_BASE]
        ; update selectors
        mov ax, gdt64.kernel_data
        mov ss, ax
        mov ds, ax
        mov es, ax

        jmp gdt64.kernel_code:prestart64

error:
        ; prints 'ERR: ' and an error code (one ASCII character in al) to the screen
        mov dword [0xb8000], 0x4f524f45
        mov dword [0xb8004], 0x4f3a4f52
        mov dword [0xb8008], 0x4f204f20
        mov byte  [0xb800a], al
        hlt

check_multiboot:
        ; returns error code 0 if the bootloader is not Multiboot-compliant
        cmp eax, 0x36d76289
        jne .no_multiboot
        ret
.no_multiboot:
        mov al, '0'
        jmp error


check_cpuid:
        ; returns error code 1 if the CPUID instruction is unsupported
        pushfd
        pop eax
        mov ecx, eax
        xor eax, 1 << 21
        push eax
        popfd
        pushfd
        pop eax
        push ecx
        popfd
        cmp eax, ecx
        je .no_cpuid
        ret
.no_cpuid:
        mov al, "1"
        jmp error

check_long_mode:
        ; test if extended processor info in available
        mov eax, 0x80000000        ; implicit argument for CPUID
        cpuid                                  ; get highest supported argument
        cmp eax, 0x80000001        ; it needs to be at least 0x80000001
        jb .no_long_mode           ; if it's less, the CPU is too old for long mode

        ; use extended info to test if long mode is available
        mov eax, 0x80000001        ; argument for extended processor info
        cpuid                                  ; returns various feature bits in ecx and edx
        test edx, 1 << 29          ; test if the LM-bit is set in the D-register
        jz .no_long_mode           ; If it's not set, there is no long mode
        ret
.no_long_mode:
        mov al, "2"
        jmp error

setup_page_tables:
        mov eax, p3_table - KERNEL_MAPPING_BASE
        or eax, 0b11        ; present + writable
        mov [p4_table - KERNEL_MAPPING_BASE], eax
        mov [p4_table - KERNEL_MAPPING_BASE + 256 * 8], eax
        mov eax, p4_table - KERNEL_MAPPING_BASE
        or eax, 0b11
        mov [p4_table - KERNEL_MAPPING_BASE + 511 * 8], eax

        mov eax, p2_table - KERNEL_MAPPING_BASE
        or eax, 0b11        ; present + writable
        mov [p3_table - KERNEL_MAPPING_BASE], eax

        ; map each PD entry to a huge (2MiB) page
        mov ecx, 0
.next_pd_entry:
        mov eax, 0x200000
        mul ecx
        or eax, 0b10000011  ; present + writable + huge
        mov [p2_table - KERNEL_MAPPING_BASE + ecx * 8], eax
        inc ecx
        cmp ecx, 32
        jne .next_pd_entry        

        ret

enable_paging:
        ; load PML4 to cr3 register
        mov eax, p4_table - KERNEL_MAPPING_BASE
        mov cr3, eax

        ; enable PAE flag in cr4
        mov eax, cr4
        or eax, 1 << 5
        mov cr4, eax

        ; set the long mode bit in the EFER MSR
        mov ecx, 0xC0000080
        rdmsr
        or eax, 1 << 8
        wrmsr

        ; enable paging in the cr0 register
        mov eax, cr0
        or eax, 1 << 31
        mov cr0, eax

        ret

check_sse:
        mov eax, 0x1
        cpuid
        test edx, 1<<25
        jz .no_sse
        ret
.no_sse:
        mov al, '4'
        jmp error

enable_sse:
        mov eax, cr0
        and ax, 0xFFFB
        or ax, 0x2
        mov cr0, eax
        mov eax, cr4
        or ax, 3 << 9
        mov cr4, eax
        ret

bits 64
prestart64:
        mov rax, start64
        jmp rax

section .text
;bits 64
start64:
        mov rsp, stack_top
        mov rax, gdt64.pointer
        lgdt [rax]

        ; update selectors
        mov ax, gdt64.kernel_data
        mov ss, ax
        mov ds, ax
        mov es, ax

        jmp start64_2

start64_2:
        mov rax, p4_table
        mov qword [rax], 0
        invlpg [0]
        mov edi, esi
        call system_earlyinit
        cli
.halt:
        hlt
        jmp .halt

section .bss
align 4096
p4_table:
        resb 4096
p3_table:
        resb 4096
p2_table:
        resb 4096
stack_bottom: ; code segment
        resb 3 * 4096
stack_top:
bootinfo: resb 4

section .rodata
align 8
gdt64:
        dq 0                                                                                                ; zero entry
.kernel_code: equ $ - gdt64
        dq (1<<44) | (1<<47) | (1<<41) | (1<<43) | (1<<53)  ; kernel code segment
.kernel_data: equ $ - gdt64
        dq (1<<44) | (1<<47) | (1<<41)                                          ; kernel data segment
.user_code: equ $ - gdt64
        dq (1<<44) | (1<<47) | (1<<41) | (1<<43) | (1<<53)  | (1<<46) | (1<<45) ; user code segment
.user_data: equ $ - gdt64
        dq (1<<44) | (1<<47) | (1<<41) | (1<<46) | (1<<45) ; user data segment
.end:
.pointer:
        dw gdt64.end - gdt64 - 1
        dq gdt64
.pointer_low:
        dw gdt64.end - gdt64 - 1
        dq gdt64 - KERNEL_MAPPING_BASE