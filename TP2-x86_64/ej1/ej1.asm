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

; FUNCIONES auxiliares que pueden llegar a necesitar:
extern malloc
extern free
extern str_concat
extern strlen
extern strcpy
extern strcat
extern fprintf
extern exit

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; string_proc_node* string_proc_node_create_asm(uint8_t type, char* hash)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
string_proc_node_create_asm:
    push rbp
    mov rbp, rsp

    ; malloc(sizeof(string_proc_node)) = 3*8 (punteros) + 1 (type) + padding = 32
    mov rdi, 32
    call malloc
    test rax, rax
    je .malloc_fail_node

    ; args: dil = type (8-bit), rsi = hash
    ; rax contiene el puntero al nodo nuevo

    ; node->next = NULL
    mov qword [rax], 0

    ; node->previous = NULL
    mov qword [rax + 8], 0

    ; node->hash = hash (en rsi)
    mov qword [rax + 16], rsi

    ; node->type = dil (type)
    mov byte [rax + 24], dil

    pop rbp
    ret

.malloc_fail_node:
    mov edi, 1
    mov rsi, err_node_create
    call fprintf
    mov edi, 1
    call exit


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; string_proc_node* string_proc_node_create_asm(uint8_t type, char* hash)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
string_proc_node_create_asm:
    push rbp
    mov rbp, rsp

    mov rdi, 32                ; sizeof(node) con padding
    call malloc
    test rax, rax
    je .malloc_fail_node

    ; args: dil = type, rsi = hash
    mov rcx, rsi               ; guardar hash
    movzx rdx, dil             ; type → 64 bits

    mov qword [rax], 0         ; node->next = NULL
    mov qword [rax + 8], 0     ; node->previous = NULL
    mov qword [rax + 16], rcx  ; node->hash = hash
    mov byte [rax + 24], dl    ; node->type

    pop rbp
    ret

.malloc_fail_node:
    mov edi, 1
    mov rsi, err_node_create
    call fprintf
    mov edi, 1
    call exit

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; void string_proc_list_add_node_asm(list*, type, hash*)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
string_proc_list_add_node_asm:
    push rbp
    mov rbp, rsp

    ; args: rdi = list, sil = type, rdx = hash
    movzx edi, sil             ; type (para node_create)
    mov rsi, rdx               ; hash
    call string_proc_node_create_asm
    mov rbx, rax               ; node

    mov rax, [rdi]             ; list->first
    test rax, rax
    je .lista_vacia

    ; lista no vacía
    mov rcx, [rdi + 8]         ; list->last
    mov [rbx + 8], rcx         ; node->previous = list->last
    mov [rcx], rbx             ; list->last->next = node
    mov [rdi + 8], rbx         ; list->last = node
    jmp .fin

.lista_vacia:
    mov [rdi], rbx             ; list->first = node
    mov [rdi + 8], rbx         ; list->last = node

.fin:
    pop rbp
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; char* string_proc_list_concat_asm(list*, type, hash*)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
string_proc_list_concat_asm:
    push rbp
    mov rbp, rsp

    ; args: rdi=list, sil=type, rdx=hash
    mov r9, rdi                ; guardar list
    mov r10, rdx               ; guardar hash

    ; llamar a add_node
    call string_proc_list_add_node_asm

    ; strlen(hash)
    mov rdi, r10
    call strlen
    inc rax
    mov rdi, rax
    call malloc
    test rax, rax
    je .malloc_fail_concat

    mov r8, rax                ; result

    ; strcpy(result, hash)
    mov rdi, r8
    mov rsi, r10
    call strcpy

    mov rbx, [r9]              ; node = list->first

.loop:
    test rbx, rbx
    je .done

    mov al, [rbx + 24]         ; node->type
    cmp al, sil
    jne .next

    mov rdi, r8                ; result
    mov rsi, [rbx + 16]        ; node->hash
    call str_concat
    mov r8, rax                ; result = new result

.next:
    mov rbx, [rbx]             ; node = node->next
    jmp .loop

.done:
    mov rax, r8
    pop rbp
    ret

.malloc_fail_concat:
    mov edi, 1
    mov rsi, err_concat
    call fprintf
    mov edi, 1
    call exit