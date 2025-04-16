; /** defines bool y puntero **/
%define NULL 0
%define TRUE 1
%define FALSE 0

section .data
err_list_create db "Error: malloc fallo en string_proc_list_create", 10, 0
err_node_create db "Error: malloc fallo en string_proc_node_create", 10, 0
err_concat db "Error: malloc fallo en string_proc_list_concat", 10, 0

section .text

global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm

extern malloc
extern free
extern str_concat
extern strlen
extern strcpy
extern strcat
extern fprintf
extern exit

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; string_proc_list* string_proc_list_create_asm()
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
string_proc_list_create_asm:
    push rdi
    mov rdi, 16
    call malloc
    test rax, rax
    jz .done

    mov qword [rax], 0
    mov qword [rax + 8], 0

.done:
    pop rdi
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; string_proc_node* string_proc_node_create_asm(uint8_t type, char* hash)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
string_proc_node_create_asm:
    push rbp
    mov rbp, rsp
    push rbx
    push r12

    mov bl, sil
    mov r12, rdx

    mov rdi, 32
    call malloc
    test rax, rax
    jz .done

    mov qword [rax], 0
    mov qword [rax + 8], 0
    mov byte [rax + 16], bl
    mov [rax + 24], r12

.done:
    pop r12
    pop rbx
    leave
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; void string_proc_list_add_node_asm(list*, type, hash*)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
string_proc_list_add_node_asm:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13

    mov rbx, rdi
    mov r12b, sil
    mov r13, rdx

    movzx rsi, r12b
    mov rdx, r13
    call string_proc_node_create_asm
    test rax, rax
    jz .done

    cmp qword [rbx], 0
    jne .append

    mov [rbx], rax
    mov [rbx + 8], rax
    jmp .done

.append:
    mov rcx, [rbx + 8]
    mov [rcx], rax
    mov [rax + 8], rcx
    mov [rbx + 8], rax

.done:
    pop r13
    pop r12
    pop rbx
    leave
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; char* string_proc_list_concat_asm(list*, type, hash*)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
string_proc_list_concat_asm:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15

    mov rbx, rdi
    mov r12d, esi
    mov r13, rdx

    test rbx, rbx
    jz .return_null

    mov dil, r12b
    mov rsi, r13
    mov rdi, rbx
    call string_proc_list_add_node_asm

    mov rdi, r13
    call strlen
    inc rax
    mov rdi, rax
    call malloc
    mov r15, rax
    test r15, r15
    jz .return_null

    mov rdi, r15
    mov rsi, r13
    call strcpy

    mov r14, [rbx]
.loop:
    test r14, r14
    jz .done

    movzx eax, byte [r14 + 16]
    cmp eax, r12d
    jne .next_node

    mov rdi, r15
    mov rsi, [r14 + 24]
    call str_concat
    test rax, rax
    jz .next_node

    mov rdi, r15
    mov r15, rax
    call free

.next_node:
    mov r14, [r14]
    jmp .loop

.done:
    mov rax, r15
    jmp .return

.return_null:
    xor eax, eax

.return:
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    leave
    ret