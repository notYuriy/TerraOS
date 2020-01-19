bits 64

global spinlock_lock
global spinlock_unlock

spinlock_lock:
        ;rdi - spinlock addr
        push rax
        push rbx
.wait:
        mov rax, 0
        mov rbx, 1
        cmpxchg qword [rdi], rbx
        jnz .wait
        pop rbx
        pop rax
        ret

spinlock_unlock:
        ;rdi - spinlock addr
        push rax
        push rbx
        mov rax, 1
        mov rbx, 0
        cmpxchg qword [rdi], rbx
        pop rbx
        pop rax
        ret
        
