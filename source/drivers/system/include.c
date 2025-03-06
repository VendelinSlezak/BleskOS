//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "memory/ram.c"
#include "memory/allocator.c"
#include "memory/physical_memory.c"

#include "interrupts/interrupts.c"
#include "interrupts/idt.c"
#include "interrupts/isr.c"
#include "interrupts/pic.c"

#include "acpi/acpi.c"
#include "acpi/aml.c"
#include "acpi/power_state.c"

#include "buses/pci.c"

#include "bios/cmos.c"
#include "bios/bios_data_area.c"

#include "user_input.c"
#include "scheduler.c"

#include "timers/pit.c"
#include "timers/hpet.c"
#include "timers/timer.c"

#include "cpu/cpu.c"
#include "cpu/commands.c"
#include "cpu/mtrr.c"

#include "deep_debugger/deep_debugger.c"

#include "logging/logging.c"
#include "logging/e9_debug_device.c"