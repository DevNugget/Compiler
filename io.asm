;; Input <- rax as ptr to string
;; Output -> print string at rax
print_s:
    ;push rax
    mov rbx, 0
print_s_loop:
    inc rax ;; increment ptr in rax to the next character
    inc rbx ;; rbx is the count of the chars until 0 is encountered

    ;; rax holds a ptr, [] is the value at the ptr
    ;; cl will then hold the character
    mov cl, [rax]

    cmp cl, 0 ;; 0 denotes the end of a string
    jne print_s_loop ;; if cl != 0 then loop

    mov rax, 1
    mov rdi, 1
    pop rsi
    mov rdx, rbx
    syscall

    ret
