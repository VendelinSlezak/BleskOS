/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define PAGE_SIZE 0x1000

#define VM_FLAG_NOT_PRESENT (0 << 0)
#define VM_FLAG_PRESENT     (1 << 0)
#define VM_FLAG_READ_ONLY   (0 << 1)
#define VM_FLAG_READ_WRITE  (1 << 1)
#define VM_FLAG_SUPERVISOR  (0 << 2)
#define VM_FLAG_USER        (1 << 2)

#define VM_PTE_PAT_BIT_SET      (1 << 7) 
#define VM_PTE_PCD_BIT_SET      (1 << 4)
#define VM_PTE_PWT_BIT_SET      (1 << 3)
#define VM_PTE_PAT_BIT_CLEAR    (0 << 7)
#define VM_PTE_PCD_BIT_CLEAR    (0 << 4)
#define VM_PTE_PWT_BIT_CLEAR    (0 << 3)
#define VM_FLAG_WRITE_BACK      (VM_PTE_PAT_BIT_CLEAR | VM_PTE_PCD_BIT_CLEAR | VM_PTE_PWT_BIT_CLEAR)
#define VM_FLAG_WRITE_THROUGH   (VM_PTE_PAT_BIT_CLEAR | VM_PTE_PCD_BIT_CLEAR | VM_PTE_PWT_BIT_SET)
#define VM_FLAG_WRITE_COMBINED  (VM_PTE_PAT_BIT_SET | VM_PTE_PCD_BIT_CLEAR | VM_PTE_PWT_BIT_CLEAR)
#define VM_FLAG_UNCACHEABLE     (VM_PTE_PAT_BIT_CLEAR | VM_PTE_PCD_BIT_SET | VM_PTE_PWT_BIT_SET)

#define VM_FLAG_TYPE_ALLOCATED          (0 << 9)
#define VM_FLAG_TYPE_LAZY_ALLOCATED     (1 << 9)
#define VM_FLAG_TYPE_ERROR_BY_ACCESS    (2 << 9)
#define VM_FLAG_TYPE_UNUSABLE           (3 << 9)
#define VM_FLAG_TYPE_COPY_ON_WRITE      (4 << 9)

#define VM_FLAGS_UNUSABLE   (VM_FLAG_NOT_PRESENT | VM_FLAG_TYPE_UNUSABLE)
#define VM_FLAGS_KERNEL_RW  (VM_FLAG_READ_WRITE | VM_FLAG_SUPERVISOR | VM_FLAG_WRITE_BACK)
#define VM_FLAGS_KERNEL_RO  (VM_FLAG_READ_ONLY | VM_FLAG_SUPERVISOR | VM_FLAG_WRITE_BACK)
#define VM_FLAGS_USER_RW    (VM_FLAG_READ_WRITE | VM_FLAG_USER | VM_FLAG_WRITE_BACK)
#define VM_FLAGS_USER_RO    (VM_FLAG_READ_ONLY | VM_FLAG_USER | VM_FLAG_WRITE_BACK)

#define VM_FLAGS_KERNEL_STACK (VM_FLAGS_KERNEL_RW | VM_FLAG_PRESENT)
#define VM_FLAGS_USER_STACK (VM_FLAGS_USER_RW | VM_FLAG_PRESENT)
#define VM_FLAGS_PAGE_DIRECTORY (VM_FLAGS_KERNEL_RW | VM_FLAG_PRESENT)
#define VM_FLAGS_PAGE_TABLE     (VM_FLAGS_KERNEL_RW | VM_FLAG_PRESENT)
#define VM_MMIO (VM_FLAGS_KERNEL_RW | VM_FLAG_UNCACHEABLE | VM_FLAG_PRESENT)

extern dword_t *pm_zero_page;

#define SIZE_OF_BITMAP (END_OF_DATA_VM >> 20)
typedef struct {
    byte_t bitmap[SIZE_OF_BITMAP];

    dword_t free_block_start;
    dword_t free_block_size;
}__attribute__((packed)) virtual_memory_allocator_bitmap_t;

#define SIZE_OF_VM_BLOCK (128*1024)
#define NUMBER_OF_VM_BLOCKS (END_OF_DATA_VM/SIZE_OF_VM_BLOCK)
typedef struct {
    word_t allocated_blocks[NUMBER_OF_VM_BLOCKS];
}__attribute__((packed)) virtual_memory_allocator_blocks_metadata_t;