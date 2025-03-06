//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void read_extended_bios_data_area(void) {
    logf("\n\nExtended BIOS Data Area ");

    // check presence of EBDA
    if(*((word_t *) 0x40E) == 0 || *((byte_t *) 0x413) == 0) {
        logf("not present");
        return;
    }
    else {
        components->p_ebda = STATUS_TRUE;
    }

    // read EBDA position
    components->ebda.memory = (byte_t *) (*((word_t *) 0x40E) * 0x10);
    components->ebda.size = (*((byte_t *) 0x413) * 1024);

    // log
    logf("present at 0x%x with size %d Kb", (dword_t)components->ebda.memory, components->ebda.size/1024);
}