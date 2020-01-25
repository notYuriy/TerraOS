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
global thread_yield
global thread_call_stub
extern thread_yield_using_frame
extern thread_exit
extern thread_preempt

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

thread_call_stub:
        call rdi
        call thread_exit
        ; this point is unreachable

; WARNING: All the code below doesn't work. Nobody knows the reason for it
thread_yield:
        push rax
; idt frame begin
        mov rax, ss
        push rax
        push 0 ; we will update this later
        pushfq
        mov rax, cs
        push rax
        lea rax, [rel .recover_point]
        push rax
        push 0
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
        mov rax, 0
        push rax
        mov rax, es
        push rax
        ; setting correct rsp
        mov rax, rsp
        mov rbx, rsp
        add rbx, 200
        mov [rbx], rax
        mov rdi, rsp
        ; even if compiler will add original pushes
        ; rsp is restored at our correct stack address
        ; this may not work from the first try
.retry:
        call thread_yield_using_frame
        mov rdi, rsp
        jmp .retry
.recover_point:
        ; if we are here, thread was again chosen to run
        ; and because registers are already restored by a scheduler
        ; at this point we only need to clean up the stack
        add rsp, 6 * 8; skipping es - cr0
        ; from that structure
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
        ; skipping errcode - ss
        add rsp, 6 * 8
        ; pop original rax
        pop rax
        ; we are done
        ret