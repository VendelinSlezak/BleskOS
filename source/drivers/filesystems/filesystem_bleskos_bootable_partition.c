//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t is_partition_bleskos_bootable_partition(dword_t first_partition_sector) {
 byte_t sector[512];

 //read first sector of partition
 if(read_storage_medium(first_partition_sector, 1, (dword_t)(&sector))==STATUS_ERROR) {
  return STATUS_FALSE;
 }

 //check signature
 if(is_memory_equal_with_memory((byte_t *)(((dword_t)(&sector))+488), "BleskOS boot partition", 22)==STATUS_TRUE) {
  return STATUS_TRUE;
 }
 else {
  return STATUS_FALSE;
 }
}

//TODO: later there will be option to update BleskOS from USB with newer version and here will be methods to read binary of code