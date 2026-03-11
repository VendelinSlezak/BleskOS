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

#define true 1
#define false 0
#define SUCCESS 1
#define ERROR 0
#define NULL 0
#define INVALID 0xFFFFFFFF
#define EOF -1

#define offsetof(type, member) ((size_t) &(((type *)0)->member))
#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)

#define PM_KERNEL_PAGE_DIRECTORY 0x4000

#define MEM_USER_HEAP_METADATA 0x1000
#define MEM_USER_HEAP_BASE 0x400000
#define TEMPORARY_KERNEL_ALLOCATOR_BASE 0x400000

#define MEM_KERNEL_HEAP_START 0xFB000000
    #define P_MEM_KERNEL_HEAP_METADATA (MEM_KERNEL_HEAP_START + 0x00000000)
#define PT_MEM_KERNEL 0xFF000000
    #define P_MEM_RAMDISK (PT_MEM_KERNEL + 0x00000000) // 1020 pages
    #define P_MEM_STANDARDIZED_PHY_MEM_MAP (PT_MEM_KERNEL + 0x3FC000) // 1 page
    #define P_MEM_STANDARDIZED_FREE_PHY_MEM_MAP (PT_MEM_KERNEL + 0x3FD000) // 1 page
    #define P_MEM_STANDARDIZED_GRAPHIC_OUTPUT_INFO (PT_MEM_KERNEL + 0x3FE000) // 1 page
    #define P_MEM_KERNEL_STACK (PT_MEM_KERNEL + 0x3FF000) // 1 page
    #define P_MEM_FOR_CLEARING_MEMORY (PT_MEM_KERNEL + 0x400000) // 1024 pages
    #define P_MEM_NEW_PAGE_DIRECTORY (PT_MEM_KERNEL + 0x400000) // 1 page
    #define P_MEM_NEW_PAGE_TABLE (PT_MEM_KERNEL + 0x401000) // 1 page
#define MEM_PM_STACK 0xFF800000 // 1024 pages
#define PT_MEM_VM_MAP 0xFFC00000
    #define P_MEM_PAGE_TABLE (PT_MEM_VM_MAP + 0x00000000) // 1023 pages
    #define P_MEM_PAGE_DIRECTORY (PT_MEM_VM_MAP + 0x003FF000) // 1 page

extern void magic_breakpoint(void);