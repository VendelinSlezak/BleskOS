/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

typedef unsigned int size_t;
typedef unsigned int uintptr_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef long long int64_t;

#define true        1
#define false       0
#define SUCCESS     1
#define ERROR       0
#define NULL        0
#define INVALID     0xFFFFFFFF
#define EOF         -1

#define offsetof(type, member)  ((size_t) &(((type *)0)->member))
#define very_likely(x)          __builtin_expect(!!(x), 1)
#define very_unlikely(x)        __builtin_expect(!!(x), 0)

#define HALT_FOREVER while(true) { asm volatile("hlt"); }
#define WAIT_FOREVER while(true) { asm volatile("pause"); }

#define PM_KERNEL_BASE_PAGE_DIRECTORY           0x4000

#define VM_TEMP_ALLOCATOR_START                 0x00400000
#define VM_USER_SPACE_START                     0x00400000
#define VM_VIRTUAL_HARDWARE_INTERFACE           0x80000000
#define VM_USER_SPACE_END                       0xC0000000
#define VM_KERNEL_SPACE_START                   0xC0000000
#define VM_RAMDISK                              (VM_KERNEL_SPACE_START + 0)
#define VM_STANDARDIZED_PHY_MEM_MAP             (VM_KERNEL_SPACE_START + 0x3FC000)
#define VM_STANDARDIZED_FREE_PHY_MEM_MAP        (VM_KERNEL_SPACE_START + 0x3FD000)
#define VM_STANDARDIZED_GRAPHIC_OUTPUT_INFO     (VM_KERNEL_SPACE_START + 0x3FE000)
#define VM_FOR_CLEARING_MEMORY                  (VM_KERNEL_SPACE_START + 0x400000)
#define VM_NEW_PAGE_DIRECTORY                   (VM_KERNEL_SPACE_START + 0x400000)
#define VM_NEW_PAGE_TABLE                       (VM_KERNEL_SPACE_START + 0x401000)
#define VM_PREVIOUS_PAGE_DIRECTORY              (VM_KERNEL_SPACE_START + 0x402000)
#define VM_KERNEL_HEAP_START                    0xC0800000
#define VM_KERNEL_HEAP_END                      0xE0000000
#define VM_PERM_ALLOCATOR_START                 0xE0000000
#define VM_PERM_ALLOCATOR_END                   0xFF000000
#define VM_AREA_FOR_CLEARING_PAGES              0xFF000000
#define VM_PHYSICAL_PAGES_STACK                 0xFF800000
#define VM_PAGE_TABLES                          0xFFC00000
#define VM_PAGE_DIRECTORY                       0xFFFFF000

extern void magic_breakpoint(void);