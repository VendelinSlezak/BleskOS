//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void cmem(void *mem, size_t size) {
    if(size < 7) {
        byte_t *byte_memory_pointer = (byte_t *) mem;
        for(dword_t i = 0; i < size; i++) {
            *byte_memory_pointer = 0;
            byte_memory_pointer++;
        }
    }
    else {
        dword_t bytes_to_align = (0x4 - ((dword_t)mem & 0x3));
        if(bytes_to_align != 0x4) {
            size -= bytes_to_align;
        }
        else {
            bytes_to_align = 0;
        }
        dword_t dwords = (size/4);
        size %= 4;

        byte_t *byte_memory_pointer = (byte_t *) mem;
        for(dword_t i = 0; i < bytes_to_align; i++) {
            *byte_memory_pointer = 0;
            byte_memory_pointer++;
        }
        dword_t *dword_memory_pointer = (dword_t *) byte_memory_pointer;
        for(dword_t i = 0; i < dwords; i++) {
            *dword_memory_pointer = 0;
            dword_memory_pointer++;
        }
        byte_memory_pointer = (byte_t *) dword_memory_pointer;
        for(dword_t i = 0; i < size; i++) {
            *byte_memory_pointer = 0;
            byte_memory_pointer++;
        }
    }
}

void clear_memory(dword_t memory, dword_t length) {
 dword_t *mem32 = (dword_t *) (memory);

 for(dword_t i=0; i<(length/4); i++) {
  *mem32 = 0;
  mem32++;
 }

 byte_t *mem8 = (byte_t *) (mem32);
 
 for(dword_t i=0; i<(length%4); i++) {
  *mem8 = 0;
  mem8++;
 }
}

void copy_memory(dword_t source_memory, dword_t destination_memory, dword_t size) {
 dword_t *source32 = (dword_t *) (source_memory);
 dword_t *destination32 = (dword_t *) (destination_memory);
 
 for(dword_t i=0; i<(size/4); i++) {
  *destination32=*source32;
  destination32++;
  source32++;
 }

 byte_t *source8 = (byte_t *) (source32);
 byte_t *destination8 = (byte_t *) (destination32);

 for(dword_t i=0; i<(size%4); i++) {
  *destination8=*source8;
  destination8++;
  source8++;
 }
}

void copy_memory_back(dword_t source_memory, dword_t destination_memory, dword_t size) {
 byte_t *source8 = (byte_t *) (source_memory);
 byte_t *destination8 = (byte_t *) (destination_memory);

 for(dword_t i=0; i<size; i++) {
  *destination8=*source8;
  destination8--;
  source8--;
 }
}

void *memcpy(void *destination, const void *source, dword_t num) {
 char *dest = (char *) destination;
 const char *src = (const char *) source;

 for(dword_t i=0; i<num; i++) {
  dest[i] = src[i];
 }

 return destination;
}

void *memset(void *ptr, int value, dword_t num) {
 char *p = (char *) ptr;

 for(dword_t i=0; i<num; i++) {
  p[i] = (char) value;
 }

 return ptr;
}

void *memmove(void *destination, const void *source, dword_t num) {
 char *dest = (char *) destination;
 const char *src = (const char *) source;

 if ((dword_t)dest > (dword_t)src && (dword_t)dest < ((dword_t)src + num)) {
  for (dword_t i = num; i > 0; i--) {
   dest[i - 1] = src[i - 1];
  }
 }
 else {
  for (dword_t i = 0; i < num; i++) {
   dest[i] = src[i];
  }
 }

 return destination;
}