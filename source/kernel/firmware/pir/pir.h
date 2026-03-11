/* 
* BleskOS
*
* MIT License
* Copyright (c) 2023-2026 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

typedef struct {
    uint8_t bus;
    uint8_t func : 3;
    uint8_t dev : 5;
    uint8_t link[4];
    uint16_t bitmap[4];
    uint8_t slot;
    uint8_t reserved;
} __attribute__((packed)) pir_table_entry_t;

typedef struct {
    uint32_t signature;
    uint16_t version;
    uint16_t table_size;
    uint8_t router_bus;
    uint8_t router_func : 3;
    uint8_t router_dev : 5;
    uint16_t exclusive_irqs;
    uint32_t compat_router;
    uint32_t miniport_data;
    uint8_t reserved[11];
    uint8_t checksum;
    pir_table_entry_t entries[];
}__attribute__((packed)) pir_table_t;