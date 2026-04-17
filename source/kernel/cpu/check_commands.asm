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

bits 32

extern invalid_opcode_return_address
extern general_protection_fault_return_address

global check_cpuid_support
check_cpuid_support:
    mov dword [invalid_opcode_return_address], cpuid_is_not_supported
    xor eax, eax
    cpuid
    mov dword [invalid_opcode_return_address], 0
    mov eax, 1 ; CPUID is supported
    ret
global cpuid_is_not_supported
cpuid_is_not_supported:
    mov dword [invalid_opcode_return_address], 0
    mov eax, 0 ; CPUID is not supported
    ret

global check_presence_of_msr
check_presence_of_msr:
    mov dword [general_protection_fault_return_address], msr_is_not_present
    mov ecx, dword [esp+4] ; MSR address passed as argument
    rdmsr
    mov dword [general_protection_fault_return_address], 0
    mov eax, 1 ; MSR is present
    ret
global msr_is_not_present
msr_is_not_present:
    mov dword [general_protection_fault_return_address], 0
    mov eax, 0 ; MSR is not present
    ret