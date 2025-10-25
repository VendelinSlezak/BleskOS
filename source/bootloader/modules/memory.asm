;;;;;
;;
;; BleskOS
;;
;; MIT License
;; Copyright (c) 2023-2025 BleskOS developers
;; Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
;; The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;;
;;;;;

; BOOTLOADER FUNCTION
read_physical_memory_map_from_bios:
    ; READ PHYSICAL MEMORY MAP FROM BIOS
    ; es is set to zero by partition bootloader
    mov di, BIOS_PHYSICAL_MEMORY_MAP_START
    mov ebx, 0 ; this register will contain pointer to next memory entry
    .bios_get_physical_memory_map_entry: 
        ; read memory entry from BIOS
        mov eax, 0xE820
        mov ecx, BIOS_PHYSICAL_MEMORY_MAP_ENTRY_SIZE
        mov edx, 0x534D4150 ; 'SMAP' signature
        int 15h

        BOOTLOADER_PANIC jc, 'can not read physical memory map entry'

        ; update variables for next entry
        inc word [number_of_bios_phy_mem_entires]
        mov ax, 0
        mov es, ax ; set es to zero to be sure that int 15h did not messed it up
        add di, BIOS_PHYSICAL_MEMORY_MAP_ENTRY_SIZE

        ; check if page is full of data
        cmp di, (BIOS_PHYSICAL_MEMORY_MAP_START + PAGE_SIZE - BIOS_PHYSICAL_MEMORY_MAP_ENTRY_SIZE)
        ja .bios_get_physical_memory_map_entry_end

        ; check if this was last entry of list
        cmp ebx, 0
        jne .bios_get_physical_memory_map_entry
    .bios_get_physical_memory_map_entry_end:

    ret

number_of_bios_phy_mem_entires dw 0

; BOOTLOADER FUNCTION
create_standardized_phy_memory_map:
    ; CREATE STANDARDIZED PHY MEMORY MAP
    ; dw number_of_entries
    ; entries:
    ;  dw type (1 = free, 2 = used)
    ;  qw start_of_memory
    ;  qw size_of_memory
    mov si, BIOS_PHYSICAL_MEMORY_MAP_START
    mov di, STANDARDIZED_PHY_MEM_MAP_START

    ; save number of entries
    movzx ecx, word [number_of_bios_phy_mem_entires]
    mov dword [di], ecx
    add di, 4

    ; add all entries in standardized form
    .add_entry_to_standardized_phy_mem_map:
        ; set memory type
        mov dword [di+standardized_memory_map_entry.type], STANDARDIZED_PHY_MEM_MAP_USED_ENTRY
        cmp dword [si+bios_memory_map_entry.type], 1
        jne .standardized_phy_mem_map_if_free_entry
            mov dword [di+standardized_memory_map_entry.type], STANDARDIZED_PHY_MEM_MAP_FREE_ENTRY
        .standardized_phy_mem_map_if_free_entry:

        ; set memory start
        mov eax, dword [si+bios_memory_map_entry.memory_start+0]
        mov dword [di+standardized_memory_map_entry.memory_start+0], eax
        mov eax, dword [si+bios_memory_map_entry.memory_start+4]
        mov dword [di+standardized_memory_map_entry.memory_start+4], eax

        ; set memory size
        mov eax, dword [si+bios_memory_map_entry.memory_size+0]
        mov dword [di+standardized_memory_map_entry.memory_size+0], eax
        mov eax, dword [si+bios_memory_map_entry.memory_size+4]
        mov dword [di+standardized_memory_map_entry.memory_size+4], eax

        ; move to next entry
        add si, BIOS_PHYSICAL_MEMORY_MAP_ENTRY_SIZE
        add di, STANDARDIZED_PHY_MEM_MAP_ENTRY_SIZE
    loop .add_entry_to_standardized_phy_mem_map

    ret

; BOOTLOADER FUNCTION
create_standardized_32_bit_free_memory_map:
    ; CREATE STANDARDIZED 32 BIT FREE MEMORY MAP
    ; area below 0x10000 is reserved and will not be added to this map
    ; dw number_of_entries
    ; entries:
    ;  dw start_of_memory
    ;  dw size_of_memory
    mov si, STANDARDIZED_PHY_MEM_MAP_START
    mov di, STANDARDIZED_FREE_PHY_MEM_MAP_START

    ; reset number of entries
    mov dword [di], 0
    add di, 4

    ; add all entries in standardized form
    mov cx, word [si] ; number of entries in standardized memory map
    add si, 4
    .add_entry_to_standardized_free_phy_mem_map:
        ; check if we will add this entry
        cmp dword [si+standardized_memory_map_entry.type], STANDARDIZED_PHY_MEM_MAP_FREE_ENTRY
        jne .add_entry_to_standardized_free_phy_mem_map_next_loop ; memory entry is not free
        cmp dword [si+standardized_memory_map_entry.memory_start+4], 0
        jne .add_entry_to_standardized_free_phy_mem_map_next_loop ; memory entry starts above 4 GB

        ; recalculate size of entry to fit 4 GB
        mov ebx, 0xFFFFFFFF ; ebx contains end of memory entry area
        cmp dword [si+standardized_memory_map_entry.memory_size+4], 0
        jne .if_entry_size_below_4_GB
            mov eax, dword [si+standardized_memory_map_entry.memory_start+0]
            add eax, dword [si+standardized_memory_map_entry.memory_size+0]
            cmp eax, dword [si+standardized_memory_map_entry.memory_start+0]
            jb .if_entry_size_below_4_GB ; entry is bigger than 4 GB, use ebx as 0xFFFFFFFF
            mov ebx, eax ; eax contains end of area that is below 4 GB
        .if_entry_size_below_4_GB:
        cmp ebx, 0x10000
        jb .add_entry_to_standardized_free_phy_mem_map_next_loop ; entry end is below 0x10000, do not add it do list
        sub ebx, dword [si+standardized_memory_map_entry.memory_start+0] ; ebx contains length of area

        ; check if this entry is above 0x10000
        mov eax, dword [si+standardized_memory_map_entry.memory_start+0]
        cmp eax, 0x10000
        jae .if_entry_starts_below_0x10000
            mov edx, 0x10000
            sub edx, eax ; edx now contains size of area below 0x10000
            sub ebx, edx ; recalculate size of area without part below 0x10000
            mov eax, 0x10000 ; area will start at 0x10000
        .if_entry_starts_below_0x10000:

        ; set memory start
        mov dword [di+standardized_32_bit_free_memory_map_entry.memory_start], eax

        ; set memory size
        and ebx, 0xFFFFF000 ; align for pages
        mov dword [di+standardized_32_bit_free_memory_map_entry.memory_size], ebx

        ; move to next entry
        .move_to_next_phy_mem_mam_entry:
        inc dword [STANDARDIZED_FREE_PHY_MEM_MAP_START] ; new entry was inserted
        add di, STANDARDIZED_FREE_PHY_MEM_MAP_ENTRY_SIZE
    .add_entry_to_standardized_free_phy_mem_map_next_loop:
    add si, STANDARDIZED_PHY_MEM_MAP_ENTRY_SIZE
    loop .add_entry_to_standardized_free_phy_mem_map

    ret

; BOOTLOADER FUNCTION
initialize_paging_memory_structures:
    ; initialize PAGING MEMORY STRUCTURES
    ; set page directory
    ; set pointers to all page tables that will be used
    mov di, PAGE_DIRECTORY_START
    call clear_page
    mov dword [PAGE_DIRECTORY_START+4*0], PAGE_TABLE_FIRST_MB | (1 << 1) | (1 << 0) ; page table for mapping first MB of virtual memory 0x00000000 - 0x00400000, with flags: write-back, supervisor, read-write, present
    mov dword [PAGE_DIRECTORY_START+4*1019], PAGE_TABLE_COMMAND | (1 << 1) | (1 << 0)
    mov dword [PAGE_DIRECTORY_START+4*1020], PAGE_TABLE_KERNEL | (1 << 1) | (1 << 0)
    mov dword [PAGE_DIRECTORY_START+4*1021], PAGE_TABLE_SHARED | (1 << 1) | (1 << 0)
    mov dword [PAGE_DIRECTORY_START+4*1022], PAGE_TABLE_PM_STACK | (1 << 1) | (1 << 0)
    mov dword [PAGE_DIRECTORY_START+4*1023], PAGE_DIRECTORY_START | (1 << 1) | (1 << 0) ; recursive mapping for accessing page directory and page tables 0xFFC00000 - 0xFFFFFFFF, with flags: write-back, supervisor, read-write, present

    ; clear page tables
    mov di, PAGE_TABLE_FIRST_MB
    call clear_page
    mov di, PAGE_TABLE_COMMAND
    call clear_page
    mov di, PAGE_TABLE_KERNEL
    call clear_page
    mov di, PAGE_TABLE_SHARED
    call clear_page
    mov di, PAGE_TABLE_PM_STACK
    call clear_page

    ; identity map bootloader code at page 0x1000
    mov eax, (1 << 1) | (1 << 0) ; flags: write-back, supervisor, read-write, present
    mov esi, 0x00001000 ; physical memory page
    mov edi, 0x00001000 ; virtual memory page
    call map_page_to_virtual_memory

    ; map page for kernel stack
    mov eax, PAGE_SIZE
    call phy_alloc
    mov eax, (1 << 1) | (1 << 0) ; flags: write-back, supervisor, read-write, present
    mov esi, ebp ; phy_alloc returned physical memory address in ebp
    mov edi, 0x1F000 ; we need to map page to virtual memory on 0x1F000 - 0x20000
    call map_page_to_virtual_memory

    ; map all usefull info from bootloader to kernel virtual memory
    mov esi, STANDARDIZED_PHY_MEM_MAP_START
    mov edi, 0x1C000
    call map_page_to_virtual_memory
    mov esi, STANDARDIZED_FREE_PHY_MEM_MAP_START
    mov edi, 0x1D000
    call map_page_to_virtual_memory
    mov esi, STANDARDIZED_GRAPHIC_OUTPUT_INFO_START
    mov edi, 0x1E000
    call map_page_to_virtual_memory

    ret

; input: eax = size of memory
; output: ebp = pointer to memory
phy_alloc:
    mov si, STANDARDIZED_FREE_PHY_MEM_MAP_START ; pointer to map that we generated
    mov cx, word [si] ; number of entries
    add si, 4 ; move pointer to first entry data
    .read_entry:
        cmp dword [si+standardized_32_bit_free_memory_map_entry.memory_size], eax
        jae .allocate_from_entry ; entry is equal/bigger than requested size, we can allocate from it

        add si, STANDARDIZED_FREE_PHY_MEM_MAP_ENTRY_SIZE ; go to next entry
    loop .read_entry

    BOOTLOADER_PANIC jmp, 'no free physical memory'

    .allocate_from_entry:
    mov ebp, dword [si+standardized_32_bit_free_memory_map_entry.memory_start] ; save start of memory entry
    add dword [si+standardized_32_bit_free_memory_map_entry.memory_start], eax ; move after allocated size
    sub dword [si+standardized_32_bit_free_memory_map_entry.memory_size], eax ; subtract allocated size

    ret

; input: data_input = physical memory source, data_output = physical memory destination, data_size = size of data
; output: nothing
copy_data:
    ; check if we can copy in real mode, or if we need to use unreal mode
    cmp dword [data_output], 0x100000
    jae .set_unreal_mode

    pusha

    ; copy in real mode
    mov ax, 0
    mov ds, ax
    mov si, word [data_input]
    mov ax, word [data_output+2]
    shl ax, 12
    mov es, ax
    mov di, word [data_output]
    mov cx, word [data_size]
    shr cx, 2 ; cx /= 4
    rep movsd

    ; reset es
    mov ax, 0
    mov es, ax

    popa
    ret

    ; save values of registers, we can not save 32 bit registers in stack
    .set_unreal_mode:
        mov dword [eax_hold], eax
        mov dword [ecx_hold], ecx
        mov dword [esi_hold], esi
        mov dword [edi_hold], edi

        ; we do not want interrupts during changing processor mode
        cli

        ; load GDT of unreal mode
        lgdt [unreal_mode_gdt_wrap]

        ; set protected mode flag
        mov eax, cr0
        or  eax, 0x00000001
        mov cr0, eax

    ; go to protected mode with 16-bit instructions
    jmp 0x08:.protected_mode

    .protected_mode:
        ; load GDT data segment with 4 GB limit
        mov ax, 0x10
        mov ds, ax

        ; clear protected mode flag
        mov eax, cr0
        and eax, 0xFFFFFFFE
        mov cr0, eax

    ; go back to real mode but with new data segment
    jmp 0x0000:.unreal_mode

.unreal_mode:
    ; set data segment to original value, but now it has loaded limit 4 GB
    xor ax, ax
    mov ds, ax

    ; copy data
    mov esi, [data_input]
    mov edi, [data_output]
    mov ecx, [data_size]
    shr ecx, 2 ; ecx /= 4
    .copy_dword:
        mov eax, dword [esi]
        mov dword [edi], eax
        add esi, 4
        add edi, 4
    loop .copy_dword
    
    ; restore values of registers
    mov eax, dword [eax_hold]
    mov ecx, dword [ecx_hold]
    mov esi, dword [esi_hold]
    mov edi, dword [edi_hold]

    ; now interrupts can come
    sti

    ret

data_input dd 0
data_output dd 0
data_size dd 0

eax_hold dd 0
esi_hold dd 0
edi_hold dd 0
ecx_hold dd 0

; input: eax = flags of page, esi = physical memory page, edi = virtual memory page
; output: nothing
map_page_to_virtual_memory:
    pusha

    ; calculate index to page directory
    mov ebx, edi
    shr ebx, 22

    ; calculate index to page table
    mov ecx, edi
    shr ecx, 12
    and ecx, 0x3FF

    ; set flags of page table (it must be already mapped)
    mov bp, bx
    shl bp, 2 ; mul 4
    add bp, PAGE_DIRECTORY_START
    mov edx, dword [bp]
    and edx, 0x0000F000 ; remove flags, page table must be in first 64 KB

    ; map page
    or esi, eax
    mov bp, cx
    shl bp, 2 ; mul 4
    add bp, dx
    mov dword [bp], esi

    popa
    ret

; input: di = address of page table
; output: nothing
clear_page:
    mov ax, 0
    mov cx, 2048
    rep stosw

    ret

; STRUCTURES
struc bios_memory_map_entry
    .memory_start           resq 1
    .memory_size            resq 1
    .type                   resd 1
    .extended_atrributes    resd 1
endstruc

struc standardized_memory_map_entry
    .type           resd 1
    .memory_start   resq 1
    .memory_size    resq 1
endstruc

struc standardized_32_bit_free_memory_map_entry
    .memory_start   resd 1
    .memory_size    resd 1
endstruc