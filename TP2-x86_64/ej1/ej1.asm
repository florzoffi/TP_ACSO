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
    push rbp
    mov rbp, rsp

    mov rdi, 16                ; sizeof(list) = 2 punteros
    call malloc
    test rax, rax
    je .malloc_fail_list

    mov qword [rax], 0         ; list->first = NULL
    mov qword [rax + 8], 0     ; list->last = NULL

    pop rbp
    ret

.malloc_fail_list:
    mov edi, 1
    mov rsi, err_list_create
    call fprintf
    mov edi, 1
    call exit

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; string_proc_node* string_proc_node_create_asm(uint8_t type, char* hash)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
string_proc_node_create_asm:
    push rbp
    mov rbp, rsp

    mov rdi, 32
    call malloc
    test rax, rax
    je .malloc_fail_node

    mov qword [rax], 0         ; node->next = NULL
    mov qword [rax + 8], 0     ; node->previous = NULL
    mov byte [rax + 16], dil   ; node->type
    mov qword [rax + 24], rsi  ; node->hash

    pop rbp
    ret

.malloc_fail_node:
    mov edi, 1
    mov rsi, err_node_create
    call fprintf
    mov edi, 1
    call exit

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; void string_proc_list_add_node_asm(list*, type, hash*)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
string_proc_list_add_node_asm:
    push rbp
    mov rbp, rsp
    push rbx
    push r12

    mov rbx, rdi              ; list
    mov r12b, sil             ; type
    mov rdx, rdx              ; hash ya está en rdx

    movzx rsi, r12b           ; preparar arg para node_create
    mov rsi, rdx              ; hash
    call string_proc_node_create_asm
    test rax, rax
    je .done

    cmp qword [rbx], 0
    jne .append

    ; lista vacía
    mov [rbx], rax           ; list->first = node
    mov [rbx + 8], rax       ; list->last = node
    jmp .done

.append:
    mov rcx, [rbx + 8]        ; rcx = list->last
    mov [rcx], rax            ; list->last->next = node
    mov [rax + 8], rcx        ; node->previous = list->last
    mov [rbx + 8], rax        ; list->last = node

.done:
    pop r12
    pop rbx
    pop rbp
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

    mov rbx, rdi              ; list
    mov r12b, sil             ; type
    mov r13, rdx              ; hash

    call string_proc_list_add_node_asm

    mov rdi, r13
    call strlen
    inc rax
    mov rdi, rax
    call malloc
    test rax, rax
    je .malloc_fail_concat

    mov r15, rax              ; result

    mov rdi, r15
    mov rsi, r13
    call strcpy

    mov r14, [rbx]            ; node = list->first

.loop:
    test r14, r14
    je .done

    mov al, [r14 + 16]        ; node->type
    cmp al, r12b
    jne .next_node

    mov rdi, r15              ; result
    mov rsi, [r14 + 24]       ; node->hash
    call str_concat
    test rax, rax
    je .next_node

    mov rdi, r15
    mov r15, rax
    call free

.next_node:
    mov r14, [r14]            ; node = node->next
    jmp .loop

.done:
    mov rax, r15
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

.malloc_fail_concat:
    mov edi, 1
    mov rsi, err_concat
    call fprintf
    mov edi, 1
    call exit