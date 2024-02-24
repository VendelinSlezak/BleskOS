;BleskOS

;;;;;
;; MIT License
;; Copyright (c) 2023-2024 Vendelín Slezák
;; Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
;; The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;;;;;

global isr_string_array

isr_string_array:
 dd .isr0
 dd .isr1
 dd .isr2
 dd .isr3
 dd .isr4
 dd .isr5
 dd .isr6
 dd .isr7
 dd .isr8
 dd .isr9
 dd .isr10
 dd .isr11
 dd .isr12
 dd .isr13
 dd .isr14
 dd .isr15
 dd .isr16
 dd .isr17
 dd .isr18

 .isr0:
 db 'Division by zero', 0

 .isr1:
 db 'Debug', 0

 .isr2:
 db 'NMI', 0

 .isr3:
 db 'Breakpoint', 0

 .isr4:
 db 'Into Detected Owerflow', 0

 .isr5:
 db 'Out of Bounds', 0

 .isr6:
 db 'Invalid Opcode', 0

 .isr7:
 db 'No Coprocessor', 0

 .isr8:
 db 'Double Fault', 0

 .isr9:
 db 'Coprocessor Segment Overrun', 0

 .isr10:
 db 'Bad TSS', 0

 .isr11:
 db 'Segment not present', 0

 .isr12:
 db 'Stack Fault', 0

 .isr13:
 db 'General Protection Fault', 0

 .isr14:
 db 'Page Fault', 0

 .isr15:
 db 'Unknown Interrupt', 0

 .isr16:
 db 'Coprocessor Fault', 0

 .isr17:
 db 'Alignment Check', 0

 .isr18:
 db 'Machine Check', 0

global pci_vendor_id_string_array

pci_vendor_id_string_array:
 dd 0x8086, .intel
 dd 0x1022, .amd
 dd 0x1002, .amd
 dd 0x14E4, .broadcom
 dd 0x10EC, .realtek
 dd 0x168C, .qualcomm_atheros
 dd 0x1969, .qualcomm_atheros
 dd 0x10DE, .nvidia
 dd 0x104C, .texas_instruments
 dd 0x14F1, .conexant_systems
 dd 0x8384, .sigmatel
 dd 0x1AF4, .red_hat
 dd 0x1234, .emulator
 dd 0

 .intel:
 db 'Intel', 0

 .amd:
 db 'AMD', 0

 .broadcom:
 db 'Broadcom', 0

 .realtek:
 db 'Realtek', 0

 .qualcomm_atheros:
 db 'Qualcomm Atheros', 0

 .nvidia:
 db 'NVidia', 0

 .texas_instruments:
 db 'Texas Instruments', 0

 .conexant_systems:
 db 'Conexant Systems', 0

 .sigmatel:
 db 'SigmaTel', 0

 .red_hat:
 db 'Red Hat', 0

 .emulator:
 db 'Emulator', 0

global pci_class_type_string_array

pci_class_type_string_array:
 dd .class_type_0
 dd .class_type_1
 dd .class_type_2
 dd .class_type_3
 dd .class_type_4
 dd .class_type_5
 dd .class_type_6
 dd .class_type_7
 dd .class_type_8
 dd .class_type_9
 dd .class_type_10
 dd .class_type_11
 dd .class_type_12
 dd .class_type_13
 dd .class_type_14
 dd .class_type_15
 dd .class_type_16

 .class_type_0:
 db 'Unclassified', 0

 .class_type_1:
 db 'Mass Storage Controller', 0

 .class_type_2:
 db 'Network Controller', 0

 .class_type_3:
 db 'Display Controller', 0

 .class_type_4:
 db 'Multimedia Controller', 0

 .class_type_5:
 db 'Memory Controller', 0

 .class_type_6:
 db 'Bridge', 0

 .class_type_7:
 db 'Simple Communication Controller', 0

 .class_type_8:
 db 'Base System Pheriphal', 0

 .class_type_9:
 db 'Input Device Controller', 0

 .class_type_10:
 db 'Docking Station', 0

 .class_type_11:
 db 'Processor', 0

 .class_type_12:
 db 'Serial Bus Controller', 0

 .class_type_13:
 db 'Wireless Controller', 0

 .class_type_14:
 db 'Intelligent Controller', 0

 .class_type_15:
 db 'Satellite Communication Controller', 0

 .class_type_16:
 db 'Encryption Controller', 0

global xml_color_numbers_string_array

xml_color_numbers_string_array:
 dd .auto, 0x000000
 dd .black, 0x000000
 dd .white, 0xFFFFFF
 dd .red, 0xFF0000
 dd .green, 0x00FF00
 dd .blue, 0x0000FF
 dd .yellow, 0xFFFF00
 dd .magneta, 0xFF00FF
 dd .cyan, 0x00FFFF
 dd .lightGrey, 0xD3D3D3
 dd .darkGrey, 0xA9A9A9
 dd .darkRed, 0x8B0000
 dd .darkGreen, 0x006400
 dd .darkBlue, 0x00008B
 dd .darkYellow, 0xF6BE00
 dd .darkMagneta, 0x8B008B
 dd .darkCyan, 0x008B8B

 .auto: db 'auto', 0
 .black: db 'black', 0
 .white: db 'white', 0
 .red: db 'red', 0
 .green: db 'green', 0
 .blue: db 'blue', 0
 .yellow: db 'yellow', 0
 .magneta: db 'magneta', 0
 .cyan: db 'cyan', 0
 .lightGrey: db 'lightGrey', 0
 .darkGrey: db 'darkGrey', 0
 .darkRed: db 'darkRed', 0
 .darkGreen: db 'darkGreen', 0
 .darkBlue: db 'darkBlue', 0
 .darkYellow: db 'darkYellow', 0
 .darkMagneta: db 'darkMagneta', 0
 .darkCyan: db 'darkCyan', 0