;;;;;
;;
;; BleskOS
;;
;; MIT License
;; Copyright (c) 2023-2026 BleskOS developers
;; Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
;; The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;;
;;;;;

%define PT_MEM_VM_MAP 0xFFC00000
%define P_MEM_PAGE_TABLE        (PT_MEM_VM_MAP + 0x00000000)
%define P_MEM_PAGE_DIRECTORY    (PT_MEM_VM_MAP + 0x003FF000)

%define VM_FLAG_NOT_PRESENT (0 << 0)
%define VM_FLAG_PRESENT     (1 << 0)
%define VM_FLAG_READ_ONLY   (0 << 1)
%define VM_FLAG_READ_WRITE  (1 << 1)
%define VM_FLAG_SUPERVISOR  (0 << 2)
%define VM_FLAG_USER        (1 << 2)
%define VM_FLAG_SPINLOCK    (1 << 11)

%define VM_PAGE_TABLE   (VM_FLAG_PRESENT | VM_FLAG_READ_WRITE | VM_FLAG_SUPERVISOR | VM_FLAG_USER)
%define VM_USER         (VM_FLAG_PRESENT | VM_FLAG_READ_WRITE | VM_FLAG_USER)

%define VM_LAZY_ALLOCATION  (0b001 << 9)
%define VM_COW_ALLOCATION   (0b010 << 9)

extern pm_alloc_page
extern kernel_panic
extern get_copy_on_write_page

extern user_space_allocation_end

global page_fault_handler_i686
page_fault_handler_i686:
    ; save state
    pushad
    push ds
    mov ax, 0x10 ; GDT selector for kernel data segment
    mov ds, ax

    ; read exact location where page fault occured
    mov eax, cr2

    ; get page where page fault occured
    and eax, 0xFFFFF000
    mov ebp, eax

    ; calculate where is page entry in page table
    mov ebx, eax
    shr ebx, 10 ; PTE index
    add ebx, P_MEM_PAGE_TABLE  ; add page table base

    ; calculate where is page table entry in page directory
    mov eax, ebp
    shr eax, 22
    shl eax, 2 ; multiply by 4
    add eax, P_MEM_PAGE_DIRECTORY ; add page directory base
    mov edx, eax

    ; check if page table is allocated
    test dword [edx], VM_FLAG_PRESENT
    jz .lazy_page_table_fault

    ; read page entry
    mov eax, dword [ebx]

    ; check case
    cmp ebp, 0
    je .null_page_fault
    cmp ebp, dword [user_space_allocation_end]
    jae .kernel_page_fault
    test eax, VM_FLAG_PRESENT
    jz .lazy_page_fault

    ; extract page type
    and eax, (0b111 << 9)
    cmp eax, VM_COW_ALLOCATION
    je .cow_page_fault

    jmp .unknown_state

    ; handle lazy allocation with allocation of page table
    .lazy_page_table_fault:
        ; get spinlock of page table
        .lock_spinlock_on_page_table:
            mov eax, dword [edx]
            mov ecx, eax
            or ecx, VM_FLAG_SPINLOCK
            lock cmpxchg dword [edx], ecx
            jnz .lock_spinlock_on_page_table
        
        ; check if this page table is already allocated - it can happen if more processors generate page fault at same time at same page table
        test dword [edx], VM_FLAG_PRESENT
        jz .allocate_page_table
            ; unlock spinlock
            mov ecx, dword [edx]
            and ecx, ~VM_FLAG_SPINLOCK
            mov dword [edx], ecx

            ; flush address of page entry = flush page table that was just allocated
            invlpg [ebx]

            jmp .lazy_page_fault
        .allocate_page_table:

        ; allocate new page table and write it to page directory
        push edx
        call pm_alloc_page
        pop edx
        or eax, VM_PAGE_TABLE
        mov dword [edx], eax ; this will clear spinlock

    ; handle lazy allocation
    .lazy_page_fault:
        ; get spinlock of page
        .lock_spinlock_on_page:
            mov eax, dword [ebx]
            mov ecx, eax
            or ecx, VM_FLAG_SPINLOCK
            lock cmpxchg dword [ebx], ecx
            jnz .lock_spinlock_on_page

        ; check if this page is already allocated - it can happen if more processors generate page fault at same time at same page
        test dword [ebx], VM_FLAG_PRESENT
        jz .allocate_new_page
            ; unlock spinlock
            mov ecx, dword [ebx]
            and ecx, ~VM_FLAG_SPINLOCK
            mov dword [ebx], ecx

            ; flush address of page
            invlpg [ebp]

            ; restore state
            pop ds
            popad
            add esp, 4 ; pop error code
            iretd
        .allocate_new_page:

        ; allocate new clear page and write it to page table
        call pm_alloc_page
        or eax, VM_USER
        mov dword [ebx], eax ; this will clear spinlock

        ; restore state
        pop ds
        popad
        add esp, 4 ; pop error code
        iretd

    ; handle copy on write
    ; TODO: only with one page
    .cow_page_fault:
        ; copy page content to temporal page buffer
        call get_copy_on_write_page
        mov esi, ebp
        mov edi, eax
        mov ecx, 1024
        rep movsd

        ; allocate new page and write it to page table
        call pm_alloc_page
        or eax, VM_USER
        mov dword [ebx], eax

        ; flush address of page
        invlpg [ebp]

        ; copy page content from temporal buffer back to new page
        call get_copy_on_write_page
        mov esi, eax
        mov edi, ebp
        mov ecx, 1024
        rep movsd

        ; restore state
        pop ds
        popad
        add esp, 4 ; pop error code
        iretd

    ; special case null page fault TODO: close when only user thread
    .null_page_fault:
        push null_page_fault
        call kernel_panic

    ; special case page fault in kernel
    .kernel_page_fault:
        push kernel_space_page_fault
        call kernel_panic

    ; this situation should never occur, it is here mainly for debugging purposes
    .unknown_state:
        push unknown_type_page_fault
        call kernel_panic

unknown_type_page_fault db 'Page has unknown type', 0
kernel_space_page_fault db 'Page fault in kernel space', 0
null_page_fault db 'Null page fault', 0

; TODO: special handler for one core