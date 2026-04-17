/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <kernel/cpu/interrupt.h>
#include <kernel/cpu/mutex.h>

#define PAGE_SIZE 0x1000
#define PAGE_MASK(x) ((x) & 0xFFFFF000)
#define PAGE_OFFSET_MASK(x) ((x) & 0xFFF)
#define PAGE_TABLE_MASK(x) ((x) & 0xFFC00000)
#define NUMBER_OF_PAGES_TO_FLUSH_BY_INVLPG 16

#define VM_FLAG_NOT_PRESENT (0 << 0)
#define VM_FLAG_PRESENT     (1 << 0)
#define VM_FLAG_READ_ONLY   (0 << 1)
#define VM_FLAG_READ_WRITE  (1 << 1)
#define VM_FLAG_SUPERVISOR  (0 << 2)
#define VM_FLAG_USER        (1 << 2)
#define VM_FLAG_GLOBAL      (1 << 8)
#define VM_FLAG_SPINLOCK    (1 << 11)

#define VM_PTE_PAT_BIT_SET      (1 << 7) 
#define VM_PTE_PCD_BIT_SET      (1 << 4)
#define VM_PTE_PWT_BIT_SET      (1 << 3)
#define VM_PTE_PAT_BIT_CLEAR    (0 << 7)
#define VM_PTE_PCD_BIT_CLEAR    (0 << 4)
#define VM_PTE_PWT_BIT_CLEAR    (0 << 3)
#define VM_WRITE_BACK      (VM_PTE_PAT_BIT_CLEAR | VM_PTE_PCD_BIT_CLEAR | VM_PTE_PWT_BIT_CLEAR)
#define VM_WRITE_THROUGH   (VM_PTE_PAT_BIT_CLEAR | VM_PTE_PCD_BIT_CLEAR | VM_PTE_PWT_BIT_SET)
#define VM_WRITE_COMBINED  (VM_PTE_PAT_BIT_SET | VM_PTE_PCD_BIT_CLEAR | VM_PTE_PWT_BIT_CLEAR)
#define VM_UNCACHEABLE     (VM_PTE_PAT_BIT_CLEAR | VM_PTE_PCD_BIT_SET | VM_PTE_PWT_BIT_SET)

#define VM_FLAGS_TYPE (0b111 << 9) // bits 9, 10 and 11 are used for type of mapping
#define VM_LAZY_ALLOCATION (0b001 << 9)
#define VM_COW_ALLOCATION (0b010 << 9)
#define VM_SPAWN_TEMPLATE (0b011 << 9)

#define VM_PAGE_TABLE (VM_FLAG_PRESENT | VM_FLAG_READ_WRITE | VM_FLAG_USER)
#define VM_KERNEL (VM_FLAG_PRESENT | VM_FLAG_READ_WRITE | VM_FLAG_SUPERVISOR)
#define VM_MMIO (VM_KERNEL | VM_UNCACHEABLE)
#define VM_USER (VM_FLAG_PRESENT | VM_FLAG_READ_WRITE | VM_FLAG_USER)

extern uint32_t pm_of_page_directory;
// extern uint32_t pm_of_lazy_allocation_page_table;
// extern uint32_t pm_of_lazy_allocation_page;
extern mutex_t creating_page_directory_mutex;