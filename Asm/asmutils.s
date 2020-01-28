bits 64

section .text

global asmutils_load_p4_table
global asmutils_invalidate_cache
global asmutils_load_idt
global asmutils_div_by_zero
global asmutils_load_ext_regs
global asmutils_store_ext_regs
global asmutils_get_p4_table
global asmutils_get_rflags
global asmutils_get_cr0
global asmutils_get_cr2
global asmutils_get_cr4
global kthread_call_stub
global kthread_yield
global kthread_yield_end
extern kthread_yield_using_frame
extern kthread_exit

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

asmutils_get_cr0:
        mov rax, cr0
        ret

asmutils_get_cr2:
        mov rax, cr2
        ret

asmutils_get_cr4:
        mov rax, cr4
        ret

asmutils_get_rflags:
        pushfq
        pop rax
        ret

kthread_call_stub:
        call rdi
        call kthread_exit
        ; this point is unreachable

kthread_yield:
        push rax
        push rbx
        push rdi
; idt frame begin
        mov rax, ss
        push rax
        mov rax, rsp
        add rax, 8 ; skip ss
        push rax
        pushfq
        mov rax, cs
        push rax
        lea rax, [rel .recover_point]
        push rax
        xor rax, rax
        push rax
        push rax
        push rbx
        push rcx
        push rdx
        push rdi
        push rsi
        push rbp
        push r8
        push r9
        push r10
        push r11
        push r12
        push r13
        push r14
        push r15
        mov rax, cr0
        push rax
        mov rax, cr2
        push rax
        mov rax, cr3
        push rax
        mov rax, cr4
        push rax
        mov rax, 0xcafebabe
        push rax
        mov rax, es
        push rax
        mov rax, ds
        push rax
        mov rax, gs
        push rax
        mov rax, fs
        push rax
        mov rdi, rsp
        call kthread_yield_using_frame
        ; this part is called when scheduler finishes
        pop rax
        mov fs, rax
        pop rax
        mov gs, rax
        pop rax
        mov ds, rax
        pop rax 
        mov es, rax
        pop rax ; intno
        pop rax ; cr4
        mov cr4, rax
        pop rax ; cr3
        mov rbx, cr3
        cmp rax, rbx
        je .next
        mov cr3, rax
        invlpg [rax]
.next:
        pop rax ; cr2
        mov cr2, rax
        pop rax ; cr0
        mov cr0, rax
        pop r15
        pop r14
        pop r13
        pop r12
        pop r11
        pop r10
        pop r9
        pop r8
        pop rbp
        pop rsi
        pop rdi
        pop rdx
        pop rcx
        pop rbx
        pop rax
        add rsp, 8
        iretq
.recover_point:
        pop rdi
        pop rbx
        pop rax
        ret

kthread_yield_end: equ kthread_yield.recover_point
