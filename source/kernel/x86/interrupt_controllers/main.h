/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <hardware/controllers/pci/device.h>

extern void (*connect_legacy_interrupt_to_handler)(dword_t irq, void (*handler)(void));

typedef struct {
    byte_t bus;
    byte_t devfunc;
    byte_t link[4];
    word_t bitmap[4];
    byte_t slot;
    byte_t reserved;
} __attribute__((packed)) pir_table_entry_t;

typedef struct {
    dword_t signature;
    word_t version;
    word_t table_size;
    byte_t router_bus;
    byte_t router_devfunc;
    word_t exclusive_irqs;
    dword_t compat_router;
    dword_t miniport_data;
    byte_t reserved[11];
    byte_t checksum;
    pir_table_entry_t entries[];
}__attribute__((packed)) pir_table_t;