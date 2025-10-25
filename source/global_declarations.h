/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

typedef unsigned int size_t;
typedef unsigned char byte_t;
typedef unsigned short word_t;
typedef unsigned int dword_t;
typedef unsigned long long qword_t;

#define TRUE 1
#define FALSE 0
#define SUCCESS 1
#define ERROR 0
#define NULL 0
#define INVALID 0xFFFFFFFF
#define EOF -1

#define KERNEL_ENTITY 0

#define offsetof(type, member) ((size_t) &(((type *)0)->member))

// #define MINOR_DEBUG
// #define MAJOR_DEBUG

#define PM_KERNEL_PAGE_DIRECTORY 0x4000

#define PT_MEM_BOOT_COMMAND 0x00000000
    #define P_MEM_STANDARDIZED_PHY_MEM_MAP (PT_MEM_BOOT_COMMAND + 0x1C000) // 1 page
    #define P_MEM_STANDARDIZED_FREE_PHY_MEM_MAP (PT_MEM_BOOT_COMMAND + 0x1D000) // 1 page
    #define P_MEM_STANDARDIZED_GRAPHIC_OUTPUT_INFO (PT_MEM_BOOT_COMMAND + 0x1E000) // 1 page
    #define P_MEM_KERNEL_STACK (PT_MEM_BOOT_COMMAND + 0x1F000) // 1 page

#define END_OF_DATA_VM 0xFDC00000
#define PT_MEM_THIRD_VM_MAP 0xFE800000
    #define P_MEM_THIRD_PAGE_TABLE (PT_MEM_THIRD_VM_MAP + 0x00000000) // 1023 pages
    #define P_MEM_THIRD_PAGE_DIRECTORY (PT_MEM_THIRD_VM_MAP + 0x003FF000) // 1 page
#define PT_MEM_SECOND_VM_MAP 0xFEC00000
    #define P_MEM_SECOND_PAGE_TABLE (PT_MEM_SECOND_VM_MAP + 0x00000000) // 1023 pages
    #define P_MEM_SECOND_PAGE_DIRECTORY (PT_MEM_SECOND_VM_MAP + 0x003FF000) // 1 page
#define PT_MEM_KERNEL 0xFF000000
    #define P_MEM_RAMDISK (PT_MEM_KERNEL + 0x00000000) // 256 pages
    #define P_MEM_CE_VM_BITMAP (PT_MEM_KERNEL + 0x00100000) // 1 page
    #define P_MEM_CE_PAGE_DIRECTORY (PT_MEM_KERNEL + 0x00101000) // 1 page
    #define P_MEM_CE_PAGE_TABLE (PT_MEM_KERNEL + 0x00102000) // 1 page
    #define P_MEM_CE_PAGE_DATA (PT_MEM_KERNEL + 0x00103000) // 1 page
    #define P_MEM_KERNEL_VMA_BITMAP (PT_MEM_KERNEL + 0x00104000) // 1 page
    #define P_MEM_FLOATING_STACK (PT_MEM_KERNEL + 0x00105000) // 1 page
    #define P_MEM_ABOVE_COMMAND_LIST (PT_MEM_KERNEL + 0x00106000) // 1 page
    #define P_MEM_COMMAND_KERNEL_STACK (PT_MEM_KERNEL + 0x00107000) // 1 page
#define PT_MEM_SHARED 0xFF400000
    #define P_MEM_GDT (PT_MEM_SHARED + 0x00000000) // 1 page
    #define P_MEM_IDT (PT_MEM_SHARED + 0x00001000) // 1 page
    #define P_MEM_TSS (PT_MEM_SHARED + 0x00002000) // 1 page
    #define P_MEM_ENTITIES_METADATA (PT_MEM_SHARED + 0x00003000) // 1 page
    #define P_MEM_ENTITIES (PT_MEM_SHARED + 0x00004000) // 256 pages
    #define P_MEM_COMMANDS_METADATA (PT_MEM_SHARED + 0x00084000) // 1 page
    #define P_MEM_COMMANDS (PT_MEM_SHARED + 0x00085000) // 256 pages
    #define P_MEM_SCHEDULER_RUNNING_COMMANDS (PT_MEM_SHARED + 0x00105000) // 1 page
    #define P_MEM_SCHEDULER_SLEEPING_COMMANDS (PT_MEM_SHARED + 0x00106000) // 1 page
    #define P_MEM_SCHEDULER_WAITING_COMMANDS (PT_MEM_SHARED + 0x00107000) // 1 page
    #define P_MEM_COMMAND_INFO (PT_MEM_SHARED + 0x00108000) // 1 page
    #define P_MEM_IDLE_COMMAND_STACK (PT_MEM_SHARED + 0x00109000) // 1 page
#define PT_MEM_PM_STACK 0xFF800000
    #define P_MEM_PM_STACK (PT_MEM_PM_STACK + 0x00000000) // 1024 pages
#define PT_MEM_VM_MAP 0xFFC00000
    #define P_MEM_PAGE_TABLE (PT_MEM_VM_MAP + 0x00000000) // 1023 pages
    #define P_MEM_PAGE_DIRECTORY (PT_MEM_VM_MAP + 0x003FF000) // 1 page

extern void magic_breakpoint(void);