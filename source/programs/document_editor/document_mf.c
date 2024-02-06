//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 Document Memory Format

 This format is layer between formats like DOC or ODT and DLLMF.
 It consists of word chars. When there is DMF_SECTION_FORMAT_CHANGE_SIGNATURE it mean that follows this block
 that specifies what changes:

 word = DMF_SECTION_FORMAT_CHANGE_SIGNATURE
 word = length of section in bytes with signatures
 word = what changes in this block, set bit mean present
  bit 0 = new page
  bit 1 = new paragraph
  bit 2 = inline change

 word = width of new page
 word = height of new page
 word = page top border size in mm
 word = page bottom border size in mm
 word = page left border size in mm
 word = page right border size in mm

 word = paragraph description, set bit mean present
  bit 0 = paragraph top border collapse
  bit 1 = paragraph bottom border collapse
  bit 2 = this paragraph is list entry
  bit 3+4 = alignment 0b00 = left 0b01 = center 0b10 = right
 word = paragraph top border size
 word = paragraph bottom border size
 word = paragraph left border size
 word = paragraph right border size
 word = list entry char, DMF_SFCH_PARAGRAPH_LIST_ENTRY_DOT = dot, other = number to print

 word = char size
 word = char emphasis
 dword = char color
 dword = char background color

 word = length of section in bytes with signatures
 word = DMF_SECTION_FORMAT_CHANGE_SIGNATURE
*/

void convert_dmf_to_dllmf(dword_t dmf_memory, dword_t dllmf_memory) {
 word_t *dmf = (word_t *) (dmf_memory);
 dword_t *dllmf = (dword_t *) (dllmf_memory);

 while(*dmf!=0) {
  
 }
}