//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void draw_vertical_scrollbar(dword_t x, dword_t y, dword_t height, dword_t rider_position, dword_t rider_size) {
 //draw background
 draw_full_square(x, y, SCROLLBAR_SIZE, height, 0xDDDDDD);
 
 //draw rider
 draw_full_square(x, y+rider_position, SCROLLBAR_SIZE, rider_size, 0x888888);
}

void draw_horizontal_scrollbar(dword_t x, dword_t y, dword_t width, dword_t rider_position, dword_t rider_size) {
 //draw background
 draw_full_square(x, y, width, SCROLLBAR_SIZE, 0xDDDDDD);
 
 //draw rider
 draw_full_square(x+rider_position, y, rider_size, SCROLLBAR_SIZE, 0x888888);
}

dword_t calculate_scrollbar_rider_position(dword_t scrollbar_size, dword_t rider_size, dword_t full_document_size, dword_t showed_document_size, dword_t showed_document_line) {
 if((full_document_size-showed_document_size)==0) {
  return 0;
 }
 return ((scrollbar_size-rider_size)*showed_document_line/(full_document_size-showed_document_size));
}

dword_t calculate_scrollbar_rider_size(dword_t scrollbar_size, dword_t full_document_size, dword_t showed_document_size) {
 if(full_document_size<=showed_document_size) {
  return 0;
 }
 dword_t rider_size = (scrollbar_size*showed_document_size/full_document_size);
 if(rider_size==0) {
  rider_size = 1;
 }
 return rider_size;
}

dword_t get_scrollbar_rider_value(dword_t scrollbar_size, dword_t rider_size, dword_t rider_position, dword_t full_document_size, dword_t showed_document_size) {
 return ((full_document_size-showed_document_size)*rider_position/(scrollbar_size-rider_size));
}

void scrollbar_struct_calculate_rider_position(struct scrollbar_info_t *scrollbar_info) {
 scrollbar_info->rider_position = calculate_scrollbar_rider_position(scrollbar_info->size, scrollbar_info->rider_size, scrollbar_info->full_document_size, scrollbar_info->showed_document_size, scrollbar_info->showed_document_line);
}

void scrollbar_struct_calculate_rider_size(struct scrollbar_info_t *scrollbar_info) {
 scrollbar_info->rider_size = calculate_scrollbar_rider_size(scrollbar_info->size, scrollbar_info->full_document_size, scrollbar_info->showed_document_size);
}

void scrollbar_struct_calculate_rider(struct scrollbar_info_t *scrollbar_info) {
 scrollbar_info->rider_size = calculate_scrollbar_rider_size(scrollbar_info->size, scrollbar_info->full_document_size, scrollbar_info->showed_document_size);
 scrollbar_info->rider_position = calculate_scrollbar_rider_position(scrollbar_info->size, scrollbar_info->rider_size, scrollbar_info->full_document_size, scrollbar_info->showed_document_size, scrollbar_info->showed_document_line);
}

void scrollbar_struct_draw_vertical_scrollbar(struct scrollbar_info_t *scrollbar_info, dword_t click_zone) {
 draw_vertical_scrollbar(scrollbar_info->x, scrollbar_info->y, scrollbar_info->size, scrollbar_info->rider_position, scrollbar_info->rider_size);
 add_zone_to_click_board(scrollbar_info->x, scrollbar_info->y, SCROLLBAR_SIZE, scrollbar_info->size, click_zone);
}

void scrollbar_struct_draw_horizontal_scrollbar(struct scrollbar_info_t *scrollbar_info, dword_t click_zone) {
 draw_horizontal_scrollbar(scrollbar_info->x, scrollbar_info->y, scrollbar_info->size, scrollbar_info->rider_position, scrollbar_info->rider_size);
 add_zone_to_click_board(scrollbar_info->x, scrollbar_info->y, scrollbar_info->size, SCROLLBAR_SIZE, click_zone);
}