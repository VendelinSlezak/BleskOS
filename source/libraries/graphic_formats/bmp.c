//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t convert_bmp_to_image_data(dword_t bmp_memory) {
 byte_t *bmp8 = (byte_t *) bmp_memory;
 word_t *bmp16 = (word_t *) (bmp_memory+28);
 dword_t *bmp32 = (dword_t *) (bmp_memory+18);
 
 //check signature
 if(bmp8[0]!='B' || bmp8[1]!='M') {
  log("BMP: invalid signature\n");
  return STATUS_ERROR;
 }
 
 //TODO: check if no compression
 
 //read informations about image
 dword_t width = bmp32[0];
 dword_t heigth = bmp32[1];
 dword_t bpp = (bmp16[0]/8);
 
 if(width==0 || heigth==0 || bpp<1 || bpp>4 || (width*heigth)>4096*4096) {
  log("BMP: not fitting\n");
  return STATUS_ERROR;
 }
 
 //convert image
 dword_t image_memory = create_image(width, heigth);
 dword_t *image_data = (dword_t *) (get_image_data_memory(image_memory)+(width*heigth*4)-(width*4));
 bmp32 = (dword_t *) (bmp_memory+10); //offset to image data
 if(bpp==4) {
  dword_t *bmp_data = (dword_t *) (bmp_memory+bmp32[0]); //bmp data
  
  for(int line=0; line<heigth; line++) {
   for(int i=0; i<width; i++) {
    *image_data=*bmp_data;
    image_data++;
    bmp_data++;
   }
   
   image_data-=(width*2);
  }
 }
 else if(bpp==3) {
  byte_t *bmp_data = (byte_t *) (bmp_memory+bmp32[0]); //bmp data
  
  for(int line=0; line<heigth; line++) {
   for(int i=0; i<width; i++) {
    *image_data=(bmp_data[0] | bmp_data[1]<<8 | bmp_data[2]<<16);
    image_data++;
    bmp_data+=3;
   }
   
   image_data-=(width*2);
  }
 }
 else if(bpp==2) {
  word_t *bmp_data = (word_t *) (bmp_memory+bmp32[0]); //bmp data
  
  for(int line=0; line<heigth; line++) {
   for(int i=0; i<width; i++) {
    *image_data=((*bmp_data & 0x3F) | (((*bmp_data>>5) & 0x3F)<<8) | ((*bmp_data>>11)<<16));
    image_data++;
    bmp_data++;
   }
   
   image_data-=(width*2);
  }
 }
 else if(bpp==1) {
  byte_t *bmp_data = (byte_t *) (bmp_memory+bmp32[0]); //bmp data
  
  for(int line=0; line<heigth; line++) {
   for(int i=0; i<width; i++) {
    *image_data=((*bmp_data << 16) | (*bmp_data << 8) | (*bmp_data));
    image_data++;
    bmp_data++;
   }
   
   image_data-=(width*2);
  }
 }
 
 return image_memory;
}

void convert_image_data_to_bmp(dword_t image_info_memory) {
 dword_t *image_info = (dword_t *) image_info_memory;
 dword_t *image_data = (dword_t *) (get_image_data_memory(image_info_memory));
 dword_t bmp_file_size = (54+(image_info[IMAGE_INFO_REAL_WIDTH]*image_info[IMAGE_INFO_REAL_HEIGTH]*4));
 dword_t bmp_file_memory = malloc(bmp_file_size);
 byte_t *bmp_file8 = (byte_t *) bmp_file_memory;
 word_t *bmp_file16 = (word_t *) (bmp_file_memory+26);
 dword_t *bmp_file32 = (dword_t *) (bmp_file_memory+2);
 
 //create file header
 bmp_file8[0]='B'; //signature
 bmp_file8[1]='M'; //signature
 bmp_file32[0]=bmp_file_size; //size of file
 bmp_file32[1]=0; //reserved
 bmp_file32[2]=54; //offset to image data
 bmp_file32[3]=40; //size of extended header
 bmp_file32[4]=image_info[IMAGE_INFO_REAL_WIDTH]; //width of image
 bmp_file32[5]=image_info[IMAGE_INFO_REAL_HEIGTH]; //heigth of image
 bmp_file16[0]=1; //number of color planes
 bmp_file16[1]=32; //4 bytes per pixel
 bmp_file32[7]=0; //no compression
 bmp_file32[8]=(bmp_file_size-54); //size of image
 bmp_file32[9]=image_info[IMAGE_INFO_REAL_WIDTH]; //width of image
 bmp_file32[10]=image_info[IMAGE_INFO_REAL_HEIGTH]; //heigth of image
 bmp_file32[11]=0; //number of colors in pallete
 bmp_file32[12]=0; //number of important colors
 
 //convert file data
 bmp_file32 = (dword_t *) (bmp_file_memory+bmp_file_size-(image_info[IMAGE_INFO_REAL_WIDTH]*4));
 for(int line=0; line<image_info[IMAGE_INFO_REAL_HEIGTH]; line++) {
  for(int column=0; column<image_info[IMAGE_INFO_REAL_WIDTH]; column++) {
   *bmp_file32 = *image_data;
   bmp_file32++;
   image_data++;
  }
  
  bmp_file32 -= (image_info[IMAGE_INFO_REAL_WIDTH]*2);
 }
 
 converted_file_memory = bmp_file_memory;
 converted_file_size = bmp_file_size;
}
