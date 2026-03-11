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

global memcpy
memcpy:
    push ebp
    mov ebp, esp
    push edi
    push esi

    mov edi, [ebp + 8]   ; destination
    mov esi, [ebp + 12]  ; source
    mov ecx, [ebp + 16]  ; count

    ; test if count is zero
    test ecx, ecx
    jz .done

    ; set direction flag
    cld

    ; write 4 byte blocks
    mov edx, ecx
    shr ecx, 2
    rep movsd

    ; write remaining bytes
    mov ecx, edx
    and ecx, 3
    rep movsb

    .done:
    mov eax, [ebp + 8]   ; return original pointer
    pop esi
    pop edi
    pop ebp

    ret

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