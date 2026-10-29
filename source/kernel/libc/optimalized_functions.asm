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

global memset
memset:
    push ebp
    mov ebp, esp
    push edi

    mov edi, [ebp + 8]   ; destination
    mov eax, [ebp + 12]  ; value
    mov ecx, [ebp + 16]  ; count

    ; test if count is zero
    test ecx, ecx
    jz .done

    ; create 32 bit value for writing (al = 0x12 will be expanded to eax = 0x12121212)
    mov ah, al
    mov dx, ax
    shl eax, 16
    mov ax, dx

    ; write 4 byte blocks
    mov edx, ecx
    shr ecx, 2
    rep stosd

    ; write remaining bytes
    mov ecx, edx
    and ecx, 3
    rep stosb

    .done:
    mov eax, [ebp + 8] ; return original pointer
    pop edi
    pop ebp

    ret

global dwmemset
dwmemset:
    push ebp
    mov ebp, esp
    push edi

    mov edi, [ebp + 8]   ; destination
    mov eax, [ebp + 12]  ; value
    mov ecx, [ebp + 16]  ; count

    ; test if count is zero
    test ecx, ecx
    jz .done

    ; write bytes
    rep stosd

    .done:
    mov eax, [ebp + 8] ; return original pointer
    pop edi
    pop ebp

    ret

; global memcpy
; memcpy:
;     push ebp
;     mov ebp, esp
;     push edi
;     push esi

;     mov edi, [ebp + 8]   ; destination
;     mov esi, [ebp + 12]  ; source
;     mov ecx, [ebp + 16]  ; count

;     ; test if count is zero
;     test ecx, ecx
;     jz .done

;     ; set direction flag
;     cld

;     ; write 4 byte blocks
;     mov edx, ecx
;     shr ecx, 2
;     rep movsd

;     ; write remaining bytes
;     mov ecx, edx
;     and ecx, 3
;     rep movsb

;     .done:
;     mov eax, [ebp + 8]   ; return original pointer
;     pop esi
;     pop edi
;     pop ebp

;     ret

; minimal instruction set: i386
; input: void *dest, void *src, size_t count
; input requirements: nothing
; output: nothing
; this functions copies memory in 1 byte blocks
global memcpy1
memcpy1:
    push esi
    push edi

    mov edi, [esp + 12] ; destination
    mov esi, [esp + 16] ; source
    mov ecx, [esp + 20] ; count
    test ecx, ecx
    jz .done
    cld
    rep movsb

    .done:
    pop edi
    pop esi

    ret

; minimal instruction set: i386
; input: void *dest, void *src, size_t count
; input requirements: count is a multiple of 4
; output: nothing
; this functions copies memory in 4 byte blocks
global memcpy4
memcpy4:
    push esi
    push edi

    mov edi, [esp + 12] ; destination
    mov esi, [esp + 16] ; source
    mov ecx, [esp + 20] ; count
    shr ecx, 2          ; count /= 4
    test ecx, ecx
    jz .done
    cld
    rep movsd

    .done:
    pop edi
    pop esi

    ret

; minimal instruction set: sse1
; input: void *dest, void *src, size_t count
; input requirements: dest and src must be aligned to 16 bytes, count is a multiple of 128
; output: nothing
; this functions copies memory in 128 byte blocks on aligned memory, keeps data in cache
global memcpy128a
memcpy128a:
    mov edx, [esp + 4]  ; destination
    mov ecx, [esp + 8] ; source
    mov eax, [esp + 12] ; count
    shr eax, 7          ; count /= 128
    jz .done

    .loop:
        ; prefetch data for next iteration
        prefetcht0 [ecx + 256]

        ; load data to registers
        movaps xmm0, [ecx]
        movaps xmm1, [ecx + 16]
        movaps xmm2, [ecx + 32]
        movaps xmm3, [ecx + 48]
        movaps xmm4, [ecx + 64]
        movaps xmm5, [ecx + 80]
        movaps xmm6, [ecx + 96]
        movaps xmm7, [ecx + 112]

        ; write data to memory
        movaps [edx], xmm0
        movaps [edx + 16], xmm1
        movaps [edx + 32], xmm2
        movaps [edx + 48], xmm3
        movaps [edx + 64], xmm4
        movaps [edx + 80], xmm5
        movaps [edx + 96], xmm6
        movaps [edx + 112], xmm7

        ; next iteration
        add ecx, 128
        add edx, 128
        dec eax
    jnz .loop

    .done:
    ret

; minimal instruction set: sse1
; input: void *dest, void *src, size_t count
; input requirements: dest must be aligned to 16 bytes, count is a multiple of 128
; output: nothing
; this functions copies memory in 128 byte blocks from unaligned memory to aligned memory, keeps data in cache
global memcpy128u
memcpy128u:
    mov edx, [esp + 4]  ; destination
    mov ecx, [esp + 8] ; source
    mov eax, [esp + 12] ; count
    shr eax, 7          ; count /= 128
    jz .done

    .loop:
        ; prefetch data for next iteration
        prefetcht0 [ecx + 256]

        ; load data to registers
        movups xmm0, [ecx]
        movups xmm1, [ecx + 16]
        movups xmm2, [ecx + 32]
        movups xmm3, [ecx + 48]
        movups xmm4, [ecx + 64]
        movups xmm5, [ecx + 80]
        movups xmm6, [ecx + 96]
        movups xmm7, [ecx + 112]

        ; write data to memory
        movaps [edx], xmm0
        movaps [edx + 16], xmm1
        movaps [edx + 32], xmm2
        movaps [edx + 48], xmm3
        movaps [edx + 64], xmm4
        movaps [edx + 80], xmm5
        movaps [edx + 96], xmm6
        movaps [edx + 112], xmm7

        ; next iteration
        add ecx, 128
        add edx, 128
        dec eax
    jnz .loop

    .done:
    ret

; minimal instruction set: sse1
; input: void *dest, void *src, size_t count
; input requirements: dest and src must be aligned to 16 bytes, count is a multiple of 128
; output: nothing
; this functions copies memory in 128 byte blocks on aligned memory, skips cache
; it is up to caller method to call sfence
global memcpynt128a
memcpynt128a:
    mov edx, [esp + 4]  ; destination
    mov ecx, [esp + 8] ; source
    mov eax, [esp + 12] ; count
    shr eax, 7          ; count /= 128
    jz .done

    .loop:
        ; prefetch data for next iteration
        prefetchnta [ecx + 256]

        ; load data to registers
        movaps xmm0, [ecx]
        movaps xmm1, [ecx + 16]
        movaps xmm2, [ecx + 32]
        movaps xmm3, [ecx + 48]
        movaps xmm4, [ecx + 64]
        movaps xmm5, [ecx + 80]
        movaps xmm6, [ecx + 96]
        movaps xmm7, [ecx + 112]

        ; write data to memory
        movntps [edx], xmm0
        movntps [edx + 16], xmm1
        movntps [edx + 32], xmm2
        movntps [edx + 48], xmm3
        movntps [edx + 64], xmm4
        movntps [edx + 80], xmm5
        movntps [edx + 96], xmm6
        movntps [edx + 112], xmm7

        ; next iteration
        add ecx, 128
        add edx, 128
        dec eax
    jnz .loop

    .done:
    ret

; minimal instruction set: sse1
; input: void *dest, void *src, size_t count
; input requirements: dest be aligned to 16 bytes, count is a multiple of 128
; output: nothing
; this functions copies memory in 128 byte blocks from unaligned memory to aligned memory, skips cache
; it is up to caller method to call sfence
global memcpynt128u
memcpynt128u:
    mov edx, [esp + 4]  ; destination
    mov ecx, [esp + 8] ; source
    mov eax, [esp + 12] ; count
    shr eax, 7          ; count /= 128
    jz .done

    .loop:
        ; prefetch data for next iteration
        prefetchnta [ecx + 256]

        ; load data to registers
        movups xmm0, [ecx]
        movups xmm1, [ecx + 16]
        movups xmm2, [ecx + 32]
        movups xmm3, [ecx + 48]
        movups xmm4, [ecx + 64]
        movups xmm5, [ecx + 80]
        movups xmm6, [ecx + 96]
        movups xmm7, [ecx + 112]

        ; write data to memory
        movntps [edx], xmm0
        movntps [edx + 16], xmm1
        movntps [edx + 32], xmm2
        movntps [edx + 48], xmm3
        movntps [edx + 64], xmm4
        movntps [edx + 80], xmm5
        movntps [edx + 96], xmm6
        movntps [edx + 112], xmm7

        ; next iteration
        add ecx, 128
        add edx, 128
        dec eax
    jnz .loop

    .done:
    ret

; end of memcpy

global memmove
memmove:
    push ebp
    mov ebp, esp
    push edi
    push esi

    mov edi, [ebp + 8]   ; destination pointer
    mov esi, [ebp + 12]  ; source pointer
    mov ecx, [ebp + 16]  ; number of bytes

    ; test if count is zero
    test ecx, ecx
    jz .done

    ; Overlap check:
    ; If dest < src, we can safely copy forward.
    ; If dest >= src + count, regions don't overlap, we can copy forward.
    cmp edi, esi
    jb .forward

    mov eax, esi
    add eax, ecx       ; EAX = src + count
    cmp edi, eax
    jae .forward       ; dest is beyond source end, copy forward

    ; --- BACKWARD COPY (Overlap detected where dest > src) ---
    std ; set direction flag
    
    ; Adjust pointers to the end of the blocks
    lea edi, [edi + ecx - 1]
    lea esi, [esi + ecx - 1]

    ; Copy byte by byte backwards
    rep movsb
    
    cld ; clear direction flag
    jmp .done

    ; --- FORWARD COPY ---
    .forward:
    cld                ; ensure direction is forward
    mov edx, ecx
    shr ecx, 2         ; count / 4
    rep movsd          ; copy Dwords
    
    mov ecx, edx
    and ecx, 3         ; remaining bytes
    rep movsb          ; copy remaining bytes

    .done:
    mov eax, [ebp+8]   ; return destination pointer
    pop esi
    pop edi
    pop ebp
    ret