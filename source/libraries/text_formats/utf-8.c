//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void convert_utf_8_to_unicode(dword_t utf_8_mem, dword_t unicode_mem, dword_t max_chars) {
 byte_t *utf_8_text = (byte_t *) utf_8_mem;
 word_t *unicode_text = (word_t *) unicode_mem;
 
 for(int i=0; i<max_chars; i++) {
  if(*utf_8_text==0) {
   *unicode_text = 0;
   return;
  }
  else if(*utf_8_text<0x80) {
   *unicode_text=*utf_8_text;
   unicode_text++;
   utf_8_text++;
  }
  else if((*utf_8_text & 0xE0)==0xC0) {
   *unicode_text = ((utf_8_text[1] & 0x3F) | ((utf_8_text[0] & 0x1F)<<6));
   unicode_text++;
   utf_8_text += 2;
  }
  else if((*utf_8_text & 0xF0)==0xE0){
   *unicode_text = ((utf_8_text[2] & 0x3F) | ((utf_8_text[1] & 0x3F)<<6) | ((utf_8_text[0] & 0x0F)<<12));
   unicode_text++;
   utf_8_text += 3;
  }
  else if((*utf_8_text & 0xF8)==0xF0){
   //TODO:
   utf_8_text += 4;
  }
  else {
   utf_8_text++;
  }
 }
 
 *unicode_text = 0;
}

void convert_unicode_to_utf_8(dword_t unicode_mem, dword_t max_chars) {
 word_t *unicode_text = (word_t *) unicode_mem;
 
 //count length of output file
 converted_file_size = 0;
 for(int i=0; i<max_chars; i++) {
  if(*unicode_text==0) {
   break;
  }
  else if(*unicode_text<0x80) {
   converted_file_size++;
  }
  else if(*unicode_text<0x800) {
   converted_file_size+=2;
  }
  else {
   converted_file_size+=3;
  }
  
  unicode_text++;
 }
 converted_file_size++; //zero char at end
 
 //allocate memory
 converted_file_memory = malloc(converted_file_size);
 
 //convert text
 byte_t *utf_8_text = (byte_t *) converted_file_memory;
 unicode_text = (word_t *) unicode_mem;
 for(int i=0; i<max_chars; i++) {
  if(*unicode_text==0) {
   break;
  }
  else if(*unicode_text<0x80) {
   *utf_8_text = *unicode_text;
   utf_8_text++;
  }
  else if(*unicode_text<0x800) {
   utf_8_text[0] = (((*unicode_text>>6) & 0x1F) | 0xC0);
   utf_8_text[1] = ((*unicode_text & 0x3F) | 0x80);
   utf_8_text+=2;
  }
  else {
   utf_8_text[0] = (((*unicode_text>>12) & 0x0F) | 0xE0);
   utf_8_text[1] = (((*unicode_text>>6) & 0x1F) | 0x80);
   utf_8_text[2] = ((*unicode_text & 0x3F) | 0x80);
   utf_8_text+=3;
  }
  
  unicode_text++;
 }
 
 *utf_8_text = 0; //zero char at end
}
