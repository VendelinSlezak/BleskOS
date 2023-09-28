//BleskOS

/*
* MIT License
* Copyright (c) 2023 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t convert_gif_to_image_data(dword_t gif_memory, dword_t gif_file_length) {
 byte_t *gif8 = (byte_t *) gif_memory;
 word_t *gif16 = (word_t *) gif_memory;
 dword_t *gif32 = (dword_t *) gif_memory;
 dword_t image_memory = 0;
 dword_t *image_data = (dword_t *) image_memory;
 dword_t width, heigth, num_of_colors, palette_memory;

 //check signature
 if(gif8[0]!='G' || gif8[1]!='I' || gif8[2]!='F' || gif8[3]!='8' || !(gif8[4]=='7' || gif8[4]=='9') || gif8[5]!='a') {
  return STATUS_ERROR;
 }
  
 //read base info
 width = gif16[3];
 heigth = gif16[4];
 
 if(width==0 || heigth==0 || (width*heigth)>4096*4096) {
  log("GIF: not fitting\n");
  return STATUS_ERROR;
 }
 
 //create image
 image_memory = create_image(width, heigth);
 image_data = (dword_t *) (get_image_data_memory(image_memory));
 
 //read palette
 palette_memory = 0;
 if((gif8[10] & 0x80)==0x80) {
  palette_memory = (gif_memory+13);
 }
 
 //read number of colors
 num_of_colors = 0xFFFFFFFF;
 num_of_colors <<= ((gif8[10] & 0x7)+1);
 num_of_colors = (~(num_of_colors))+1;
 
 //parse chunks
 gif8 = (byte_t *) (gif_memory+13+num_of_colors*3);
 while(*gif8!=0x3B && *gif8!=0 && (dword_t)gif8<(gif_memory+gif_file_length)) { 
  if(gif8[0]==0x21) { //extensions
   if(gif8[1]==0xFF) {
    gif8 = (byte_t *) ((dword_t)gif8+gif8[2]+8);
    continue;
   }
   else if(gif8[1]==0xFE) {
    gif8 = (byte_t *) ((dword_t)gif8+gif8[2]+4);
    continue;
   }
   else if(gif8[1]==0x01) {
    gif8 = (byte_t *) ((dword_t)gif8+gif8[2]+3);
    while(*gif8!=0) {
     if((dword_t)gif8<(gif_memory+gif_file_length)) {
      return STATUS_ERROR;
     }
     gif8 = (byte_t *) ((dword_t)gif8+gif8[0]+1);
    }
    continue;
   }
   else if(gif8[1]==0xF9) {
    gif8 = (byte_t *) ((dword_t)gif8+gif8[2]+4);
    continue;
   }
   else {
    log("GIF: unknown extension ");
    log_hex(gif8[1]);
    log("\n");
    delete_image(image_memory);
    return STATUS_ERROR;
   }
  }
  else if(gif8[0]==0x2C) { //image descriptor   
   //TODO: other size of image
   gif16 = (word_t *) ((dword_t)gif8+1);
   if(gif16[2]!=width || gif16[3]!=heigth || gif16[0]!=0 || gif16[0]!=0) {
    log("GIF: image not fitting\n");
    delete_image(image_memory);
    return STATUS_ERROR;
   }
   
   //local palette
   if((gif8[9] & 0x80)!=0x80 && palette_memory==0) {
    log("GIF: no palette\n");
    delete_image(image_memory);
    return STATUS_ERROR;
   }
   else if((gif8[9] & 0x80)==0x80) {
    palette_memory = ((dword_t)gif8+10);
    gif8 = (byte_t *) ((dword_t)gif8+10+num_of_colors*3-11);
   }
   
   //read length of image data
   gif8 = (byte_t *) ((dword_t)gif8+11);
   dword_t gif_data_memory = ((dword_t)gif8);
   dword_t length_of_image_data = 0;
   while(*gif8!=0 && (dword_t)gif8<(gif_memory+gif_file_length)) {
    length_of_image_data += *gif8;
    gif8 = (byte_t *) ((dword_t)gif8+gif8[0]+1);
   }
   
   //copy image data from chunks
   dword_t gif_image_data = malloc(length_of_image_data);
   dword_t gif_data_copy_memory = gif_image_data;
   gif8 = (byte_t *) gif_data_memory;
   while(*gif8!=0 && (dword_t)gif8<(gif_memory+gif_file_length)) {
    gif8 = (byte_t *) gif_data_memory;
    copy_memory(gif_data_memory+1, gif_data_copy_memory, gif8[0]);
    gif_data_memory += (*gif8+1);
    gif_data_copy_memory += *gif8;
   }
   
   //decode image data from LZW
   dword_t gif_image_decoded_data = malloc(width*heigth);
   if(decode_lzw(num_of_colors, gif_image_data, length_of_image_data, gif_image_decoded_data, (width*heigth))==STATUS_ERROR) {
    log("GIF: LZW error\n");
    delete_image(image_memory);
    free(gif_image_data);
    free(gif_image_decoded_data);
    return STATUS_ERROR;
   }
   
   //convert to image data
   byte_t *palette = (byte_t *) palette_memory;
   byte_t *image_decoded_data = (byte_t *) gif_image_decoded_data;
   for(int i=0; i<(width*heigth); i++) {    
    *image_data = (palette[*image_decoded_data*3+0]<<16 | palette[*image_decoded_data*3+1]<<8 | palette[*image_decoded_data*3+2]);
    image_data++;
    image_decoded_data++;
   }
   
   free(gif_image_data);
   free(gif_image_decoded_data);
   
   break;
  }
  else {
   log("GIF: unknown value ");
   log_hex(gif8[0] | gif8[1]<<8);
   log("\n");
   delete_image(image_memory);
   return STATUS_ERROR;
  }
 }
 
 return image_memory;
}
