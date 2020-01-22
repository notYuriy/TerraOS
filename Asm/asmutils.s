bits 64

global asmutils_load_p4_table
global asmutils_invalidate_cache
global asmutils_load_idt
global asmutils_div_by_zero
global asmutils_load_ext_regs
global asmutils_store_ext_regs
global asmutils_get_p4_table
global asmutils_get_rflags

asmutils_load_p4_table:
        mov cr3, rdi
        ret

asmutils_invalidate_cache:
        invlpg [rdi]
        ret

asmutils_load_idt:
        lidt [rdi]
        ret

asmutils_store_ext_regs:
        push rdx
        push rax
        mov rdx, 0xFFFFFFFFFFFFFFFF
        mov rax, 0xFFFFFFFFFFFFFFFF
        fxsave [rdi]
        pop rax
        pop rdx
        ret

asmutils_load_ext_regs:
        push rdx
        push rax
        mov rdx, 0xFFFFFFFFFFFFFFFF
        mov rax, 0xFFFFFFFFFFFFFFFF
        fxrstor [rdi]
        pop rax
        pop rdx
        ret

asmutils_get_p4_table:
        mov rax, cr3
        ret

asmutils_get_rflags:
        pushfq
        pop rax
        ret