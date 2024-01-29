//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t convert_qoi_to_image_data(dword_t qoi_memory) {
 //test signature 'qoif'
 dword_t *qoi32 = (dword_t *) (qoi_memory);
 if(qoi32[0]!=0x66696F71) {
  log("\nQOI: invalid signature");
  return STATUS_ERROR;
 }

 //read informations about image
 byte_t *qoi8 = (byte_t *) (qoi_memory+4);
 dword_t width = ((qoi8[0]<<24) | (qoi8[1]<<16) | (qoi8[2]<<8) | (qoi8[3]));
 dword_t height = ((qoi8[4]<<24) | (qoi8[5]<<16) | (qoi8[6]<<8) | (qoi8[7]));
 byte_t channels = (qoi8[8] & 0xFF);
 if(width==0 || height==0 || channels<3 || channels>4 || (width*height)>4096*4096) {
  log("QOI: not fitting\n");
  return STATUS_ERROR;
 }

 //convert image
 dword_t image_memory = create_image(width, height);
 byte_t *image_data = (byte_t *) (get_image_data_memory(image_memory));
 byte_t *qoi_data = (byte_t *) (qoi_memory+14);
 dword_t *qoi_data_32 = (dword_t *) (qoi_memory+14);
 dword_t pixel = 0xFF000000;
 byte_t *pixel8 = (byte_t *) ((dword_t)&pixel);
 clear_memory((dword_t)qoi_index_array, 64);
 for(dword_t i=0, repeat=0; i<(width*height); i++) {
  if(repeat>0) {
   repeat--; //run of pixels
  }
  else {
   byte_t chunk_type = *qoi_data;
   qoi_data++;

   if(chunk_type==0xFE) { //RGB pixel
    qoi_data_32 = (dword_t *) ((dword_t)qoi_data);
    pixel = ((pixel & 0xFF000000) | (*qoi_data_32 & 0x00FFFFFF));
    qoi_data+=3;
   }
   else if(chunk_type==0xFF) { //RGBA pixel
    qoi_data_32 = (dword_t *) ((dword_t)qoi_data);
    pixel = (*qoi_data_32);
    qoi_data+=4;
   }
   else if((chunk_type & 0xC0)==0x00) { //pixel from index array
    pixel = qoi_index_array[(chunk_type & 0x3F)];
   }
   else if((chunk_type & 0xC0)==0x40) { //pixel calculated by small difference
    pixel8[0] += (((chunk_type >> 4) & 0x3)-2);
    pixel8[1] += (((chunk_type >> 2) & 0x3)-2);
    pixel8[2] += ((chunk_type & 0x3)-2);
   }
   else if((chunk_type & 0xC0)==0x80) { //pixel calculated by green difference
    dword_t green_difference = ((chunk_type & 0x3F) - 32);
    pixel8[1] += green_difference;
    green_difference -= 8;
    pixel8[0] += (green_difference + ((*qoi_data >> 4) & 0xF));
    pixel8[2] += (green_difference + (*qoi_data & 0xF));
    qoi_data++;
   }
   else if((chunk_type & 0xC0)==0xC0) { //run of pixels
    repeat = (chunk_type & 0x3F);
   }

   //save pixel to index array
   qoi_index_array[((pixel8[0]*3 + pixel8[1]*5 + pixel8[2]*7 + pixel8[3]*11) % 64)] = pixel;
  }
  
  //write pixel to output
  image_data[0] = pixel8[2];
  image_data[1] = pixel8[1];
  image_data[2] = pixel8[0];
  image_data+=4;
 }

 return image_memory;
}