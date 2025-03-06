//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void shutdown(void) {
    if(components->p_acpi == STATUS_FALSE) {
        return;
    }

    // TODO: Method \_PTS has to be called here for proper shutdown
    // without it, in some cases computer will be stuck in state between running and shutting down and will need to be powered off by power button

    // set S5 power state (shutdown)
    if(components->acpi.pm1a_control_reg != 0) {
        outw(components->acpi.pm1a_control_reg, components->acpi.pm1a_control_reg_shutdown_s5_value);
    }
    if(components->acpi.pm1b_control_reg != 0) {
        outw(components->acpi.pm1b_control_reg, components->acpi.pm1b_control_reg_shutdown_s5_value);
    }

    // wait for shutdown to happen
    for(dword_t i = 0; i < 1000; i++) {
        asm("hlt");
    }
}