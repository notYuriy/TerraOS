bits 64

global asmutils_load_p4_table
global asmutils_invalidate_cache
global asmutils_load_idt
global asmutils_div_by_zero
global asmutils_kek_handler

asmutils_load_p4_table:
        mov cr3, rdi
        ret

asmutils_invalidate_cache:
        invlpg [rdi]
        ret

asmutils_load_idt:
        lidt [rdi]
        ret

asmutils_div_by_zero:
        int 35
        ret


