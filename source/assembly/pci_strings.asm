;BleskOS

;;;;;
;; MIT License
;; Copyright (c) 2023-2024 Vendelín Slezák
;; Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
;; The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;;;;;

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