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

  //EXPERIMENT
  if(qoi32[0]==0x62696F71) {
   return convert_qoib_to_image_data(qoi_memory);
  }

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
 clear_memory((dword_t)qoi_index_array, 256);
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
   else { //run of pixels
    repeat = (chunk_type & 0x3F);
   }

   //save pixel to index array
   qoi_index_array[((pixel8[0]*3 + pixel8[1]*5 + pixel8[2]*7 + pixel8[3]*11) % 64)] = pixel;
  }
  
  //write pixel to output
  image_data[0] = pixel8[2];
  image_data[1] = pixel8[1];
  image_data[2] = pixel8[0];
  image_data[3] = pixel8[3];
  image_data+=4;
 }

 return image_memory;
}

dword_t convert_qoib_to_image_data(dword_t qoi_memory) {
 //test signature 'qoib'
 dword_t *qoi32 = (dword_t *) (qoi_memory);
 if(qoi32[0]!=0x62696F71) {
  log("\nQOIB: invalid signature");
  return STATUS_ERROR;
 }

 //read informations about image
 byte_t *qoi8 = (byte_t *) (qoi_memory+4);
 dword_t width = ((qoi8[0]<<24) | (qoi8[1]<<16) | (qoi8[2]<<8) | (qoi8[3]));
 dword_t height = ((qoi8[4]<<24) | (qoi8[5]<<16) | (qoi8[6]<<8) | (qoi8[7]));
 byte_t channels = (qoi8[8] & 0xFF);
 if(width==0 || height==0 || channels<3 || channels>4 || (width*height)>4096*4096) {
  log("QOIB: not fitting\n");
  return STATUS_ERROR;
 }

 //convert image
 dword_t image_memory = create_image(width, height);
 byte_t *image_data = (byte_t *) (get_image_data_memory(image_memory));
 byte_t *qoi_data = (byte_t *) (qoi_memory+14);
 dword_t *qoi_data_32 = (dword_t *) (qoi_memory+14);
 dword_t pixel = 0xFF000000;
 byte_t *pixel8 = (byte_t *) ((dword_t)&pixel);
 dword_t qoi_index_array_pointer = 1;
 clear_memory((dword_t)qoi_index_array, 256);
 for(dword_t i=0, repeat=0; i<(width*height); i++) {
  if(repeat>0) {
   repeat--; //run of pixels
  }
  else {
   byte_t chunk_type = *qoi_data;
   qoi_data++;

   if(chunk_type==0xFC) { //long run of pixels
    repeat = (qoi_data[0] | qoi_data[1]<<8 | qoi_data[2]<<16);
    qoi_data+=3;
   }
   else if(chunk_type==0xFD) { //BIG DIFF chunk
    pixel8[0] += (((qoi_data[0] >> 4) & 0xF)-8);
    pixel8[1] += ((qoi_data[0] & 0xF)-8);
    pixel8[2] += (((qoi_data[1] >> 4) & 0xF)-8);
    pixel8[3] += ((qoi_data[1] & 0xF)-8);
    qoi_data+=2;
   }
   else if(chunk_type==0xFE) { //RGB pixel
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
   else { //run of pixels
    repeat = (chunk_type & 0x3F);
   }

   //save pixel to index array
   value8 = STATUS_TRUE;
   for(dword_t j=0; j<64; j++) {
    if(qoi_index_array[j]==pixel) {
     value8 = STATUS_FALSE;
     break;
    }
   }
   if(value8==STATUS_TRUE) {
    qoi_index_array[qoi_index_array_pointer] = pixel;
    qoi_index_array_pointer++;
    if(qoi_index_array_pointer==64) {
     qoi_index_array_pointer = 1;
    }
   }
  }
  
  //write pixel to output
  image_data[0] = pixel8[2];
  image_data[1] = pixel8[1];
  image_data[2] = pixel8[0];
  image_data[3] = pixel8[3];
  image_data+=4;
 }

 return image_memory;
}

void convert_image_data_to_qoi(dword_t image_info_memory) {
 dword_t *image_info = (dword_t *) (image_info_memory);
 dword_t *image_data = (dword_t *) (get_image_data_memory(image_info_memory));
 dword_t qoi_file_memory = calloc(14+image_info[IMAGE_INFO_REAL_WIDTH]*image_info[IMAGE_INFO_REAL_HEIGTH]*4+8);
 dword_t qoi_file_length = 22; //header + ending
 dword_t *qoi_image_header = (dword_t *) (qoi_file_memory);

 //create header
 qoi_image_header[0] = 0x66696F71; //'qoif'
 qoi_image_header[1] = ((image_info[IMAGE_INFO_REAL_WIDTH] & 0xFF)<<24 | ((image_info[IMAGE_INFO_REAL_WIDTH]>>8) & 0xFF)<<16  | ((image_info[IMAGE_INFO_REAL_WIDTH]>>16) & 0xFF)<<8  | (image_info[IMAGE_INFO_REAL_WIDTH]>>24));
 qoi_image_header[2] = ((image_info[IMAGE_INFO_REAL_HEIGTH] & 0xFF)<<24 | ((image_info[IMAGE_INFO_REAL_HEIGTH]>>8) & 0xFF)<<16  | ((image_info[IMAGE_INFO_REAL_HEIGTH]>>16) & 0xFF)<<8  | (image_info[IMAGE_INFO_REAL_HEIGTH]>>24));
 qoi_image_header[3] = 0x0004; //sRGB, four channels

 //convert image data
 byte_t *qoi_image_data = (byte_t *) (qoi_file_memory+14);
 dword_t image_pixel = 0, previous_image_pixel = 0xFF000000, run = 0;
 byte_t *image_pixel8 = (byte_t *) ((dword_t)&image_pixel);
 byte_t *previous_image_pixel8 = (byte_t *) ((dword_t)&previous_image_pixel);
 clear_memory((dword_t)qoi_index_array, 256);
 for(dword_t i=0; i<(image_info[IMAGE_INFO_REAL_WIDTH]*image_info[IMAGE_INFO_REAL_HEIGTH]); i++) {
  //convert 0x0RGB pixel to 0x0BGR
  image_pixel = *image_data;
  image_data++;
  image_pixel = (image_pixel8[3]<<24 | image_pixel8[0]<<16 | image_pixel8[1]<<8 | image_pixel8[2]); //TODO: alpha color

  //RUN chunk
  if(image_pixel==previous_image_pixel) { //same pixels afterwards are comprimed
   run++;
   if(run==62) {
    //add RUN chunk
    qoi_image_data[0] = (0xC0 | (run-1));
    qoi_image_data++;
    qoi_file_length++;

    run = 0;
   }
   continue;
  }
  else {
   if(run!=0) { //there is end of pixel line that is represented in RUN chunk
    //add RUN chunk
    qoi_image_data[0] = (0xC0 | (run-1));
    qoi_image_data++;
    qoi_file_length++;

    run = 0;
   }
  }

  //select most efficent representation of pixel
  for(dword_t j=0; j<64; j++) {
   if(qoi_index_array[j]==image_pixel) {
    //add INDEX chunk
    qoi_image_data[0] = j;
    qoi_image_data++;
    qoi_file_length++;

    goto add_pixel_to_index_array;
   }
  }

  //alpha channel changed
  if(image_pixel8[3]!=previous_image_pixel8[3]) {
   //add RGBA chunk
   qoi_image_data[0] = 0xFF;
   qoi_image_data[1] = image_pixel8[0];
   qoi_image_data[2] = image_pixel8[1];
   qoi_image_data[3] = image_pixel8[2];
   qoi_image_data[4] = image_pixel8[3];
   qoi_image_data+=5;
   qoi_file_length+=5;

   goto add_pixel_to_index_array;
  }

  byte_t red_difference = (image_pixel8[0]-previous_image_pixel8[0]);
  byte_t green_difference = (image_pixel8[1]-previous_image_pixel8[1]);
  byte_t blue_difference = (image_pixel8[2]-previous_image_pixel8[2]);
  if((red_difference<=1 || red_difference>=0xFE) && (green_difference<=1 || green_difference>=0xFE) && (blue_difference<=1 || blue_difference>=0xFE)) {
   //add DIFF chunk
   qoi_image_data[0] = (0x40 | (red_difference+2)<<4 | (green_difference+2)<<2 | (blue_difference+2));
   qoi_image_data++;
   qoi_file_length++;
  }
  else {
   byte_t green_red_difference = (red_difference-green_difference);
   byte_t green_blue_difference = (blue_difference-green_difference);

   if((green_red_difference<=7 || green_red_difference>=0xF8) && (green_blue_difference<=7 || green_blue_difference>=0xF8) && (green_difference<=31 || green_difference>=0xE0)) {
    //add LUMA chunk
    qoi_image_data[0] = (0x80 | (green_difference+32));
    qoi_image_data[1] = ((green_red_difference+8)<<4 | (green_blue_difference+8));
    qoi_image_data+=2;
    qoi_file_length+=2;
   }
   else {
    //add RGB chunk
    qoi_image_data[0] = 0xFE;
    qoi_image_data[1] = image_pixel8[0];
    qoi_image_data[2] = image_pixel8[1];
    qoi_image_data[3] = image_pixel8[2];
    qoi_image_data+=4;
    qoi_file_length+=4;
   }
  }

  //add pixel to index array
  add_pixel_to_index_array:
  qoi_index_array[((image_pixel8[0]*3 + image_pixel8[1]*5 + image_pixel8[2]*7 + image_pixel8[3]*11) % 64)] = image_pixel;
  previous_image_pixel = image_pixel;
 }
 if(run!=0) {
  //add last RUN chunk
  qoi_image_data[0] = (0xC0 | (run-1));
  qoi_image_data++;
  qoi_file_length++;
 }

 //ending
 qoi_image_header = (dword_t *) ((dword_t)qoi_image_data);
 qoi_image_header[0] = 0x00000000;
 qoi_image_header[1] = 0x01000000;

 converted_file_memory = qoi_file_memory;
 converted_file_size = qoi_file_length;
}

void convert_image_data_to_qoib(dword_t image_info_memory) {
 dword_t *image_info = (dword_t *) (image_info_memory);
 dword_t *image_data = (dword_t *) (get_image_data_memory(image_info_memory));
 dword_t qoi_file_memory = calloc(14+image_info[IMAGE_INFO_REAL_WIDTH]*image_info[IMAGE_INFO_REAL_HEIGTH]*4+8);
 dword_t qoi_file_length = 22; //header + ending
 dword_t *qoi_image_header = (dword_t *) (qoi_file_memory);

 //create header
 qoi_image_header[0] = 0x62696F71; //'qoib'
 qoi_image_header[1] = ((image_info[IMAGE_INFO_REAL_WIDTH] & 0xFF)<<24 | ((image_info[IMAGE_INFO_REAL_WIDTH]>>8) & 0xFF)<<16  | ((image_info[IMAGE_INFO_REAL_WIDTH]>>16) & 0xFF)<<8  | (image_info[IMAGE_INFO_REAL_WIDTH]>>24));
 qoi_image_header[2] = ((image_info[IMAGE_INFO_REAL_HEIGTH] & 0xFF)<<24 | ((image_info[IMAGE_INFO_REAL_HEIGTH]>>8) & 0xFF)<<16  | ((image_info[IMAGE_INFO_REAL_HEIGTH]>>16) & 0xFF)<<8  | (image_info[IMAGE_INFO_REAL_HEIGTH]>>24));
 qoi_image_header[3] = 0x0004; //sRGB, four channels

 //convert image data
 byte_t *qoi_image_data = (byte_t *) (qoi_file_memory+14);
 dword_t image_pixel = 0, previous_image_pixel = 0xFF000000, run = 0;
 byte_t *image_pixel8 = (byte_t *) ((dword_t)&image_pixel);
 byte_t *previous_image_pixel8 = (byte_t *) ((dword_t)&previous_image_pixel);
 dword_t qoi_index_array_pointer = 1;
 clear_memory((dword_t)qoi_index_array, 256);
 if(*image_data==0xFF000000) { //handle special case when image starts with black color
  qoi_index_array[1] = 0xFF000000; //add color to index
  qoi_index_array_pointer++;
 }
 for(dword_t i=0; i<(image_info[IMAGE_INFO_REAL_WIDTH]*image_info[IMAGE_INFO_REAL_HEIGTH]); i++) {
  //convert 0x0RGB pixel to 0x0BGR
  image_pixel = *image_data;
  image_data++;
  image_pixel = (image_pixel8[3]<<24 | image_pixel8[0]<<16 | image_pixel8[1]<<8 | image_pixel8[2]);

  //RUN chunk
  if(image_pixel==previous_image_pixel) { //same pixels afterwards are comprimed
   run++;
   continue;
  }
  else {
   if(run!=0) { //there is end of pixel line that is represented in RUN chunk
    if(run<=60) {
     //add RUN chunk
     qoi_image_data[0] = (0xC0 | (run-1));
     qoi_image_data++;
     qoi_file_length++;
    }
    else {
     //add LONG RUN chunk
     qoi_image_data[0] = 0xFC;
     qoi_image_data[1] = ((run-1) & 0xFF);
     qoi_image_data[2] = (((run-1)>>8) & 0xFF);
     qoi_image_data[3] = (((run-1)>>16) & 0xFF);
     qoi_image_data+=4;
     qoi_file_length+=4;
    }

    run = 0;
   }
  }

  //select most efficent representation of pixel
  for(dword_t j=0; j<64; j++) {
   if(qoi_index_array[j]==image_pixel) {
    //add INDEX chunk
    qoi_image_data[0] = j;
    qoi_image_data++;
    qoi_file_length++;

    goto add_pixel_to_index_array;
   }
  }

  byte_t red_difference = (image_pixel8[0]-previous_image_pixel8[0]);
  byte_t green_difference = (image_pixel8[1]-previous_image_pixel8[1]);
  byte_t blue_difference = (image_pixel8[2]-previous_image_pixel8[2]);
  byte_t alpha_difference = (image_pixel8[3]-previous_image_pixel8[3]);
  if(alpha_difference==0) { //alpha channel did not changed, use chunks for changing RGB channels
   if((red_difference<=1 || red_difference>=0xFE) && (green_difference<=1 || green_difference>=0xFE) && (blue_difference<=1 || blue_difference>=0xFE)) {
    //add DIFF chunk
    qoi_image_data[0] = (0x40 | (red_difference+2)<<4 | (green_difference+2)<<2 | (blue_difference+2));
    qoi_image_data++;
    qoi_file_length++;

    goto add_pixel_to_index_array;
   }
   else {
    byte_t green_red_difference = (red_difference-green_difference);
    byte_t green_blue_difference = (blue_difference-green_difference);

    if((green_red_difference<=7 || green_red_difference>=0xF8) && (green_blue_difference<=7 || green_blue_difference>=0xF8) && (green_difference<=31 || green_difference>=0xE0)) {
     //add LUMA chunk
     qoi_image_data[0] = (0x80 | (green_difference+32));
     qoi_image_data[1] = ((green_red_difference+8)<<4 | (green_blue_difference+8));
     qoi_image_data+=2;
     qoi_file_length+=2;

     goto add_pixel_to_index_array;
    }
    else if((red_difference<=7 || red_difference>=0xF8) && (green_difference<=7 || green_difference>=0xF8) && (blue_difference<=7 || blue_difference>=0xF8) && (alpha_difference<=7 || alpha_difference>=0xF8)) {
     //add BIG DIFF chunk
     qoi_image_data[0] = 0xFD;
     qoi_image_data[1] = ((red_difference+8)<<4 | (green_difference+8));
     qoi_image_data[2] = ((blue_difference+8)<<4 | (alpha_difference+8));
     qoi_image_data+=3;
     qoi_file_length+=3;

     goto add_pixel_to_index_array;
    }
    else {
     //add RGB chunk
     qoi_image_data[0] = 0xFE;
     qoi_image_data[1] = image_pixel8[0];
     qoi_image_data[2] = image_pixel8[1];
     qoi_image_data[3] = image_pixel8[2];
     qoi_image_data+=4;
     qoi_file_length+=4;

     goto add_pixel_to_index_array;
    }
   }
  }
  else {
   if((red_difference<=7 || red_difference>=0xF8) && (green_difference<=7 || green_difference>=0xF8) && (blue_difference<=7 || blue_difference>=0xF8) && (alpha_difference<=7 || alpha_difference>=0xF8)) {
    //add BIG DIFF chunk
    qoi_image_data[0] = 0xFD;
    qoi_image_data[1] = ((red_difference+8)<<4 | (green_difference+8));
    qoi_image_data[2] = ((blue_difference+8)<<4 | (alpha_difference+8));
    qoi_image_data+=3;
    qoi_file_length+=3;

    goto add_pixel_to_index_array;
   }
   else {
    //add RGBA chunk
    qoi_image_data[0] = 0xFF;
    qoi_image_data[1] = image_pixel8[0];
    qoi_image_data[2] = image_pixel8[1];
    qoi_image_data[3] = image_pixel8[2];
    qoi_image_data[4] = image_pixel8[3];
    qoi_image_data+=5;
    qoi_file_length+=5;
   }
  }

  //add pixel to index array
  add_pixel_to_index_array:
  value8 = STATUS_TRUE;
  for(dword_t j=0; j<64; j++) {
   if(qoi_index_array[j]==image_pixel) {
    value8 = STATUS_FALSE;
    break;
   }
  }
  if(value8==STATUS_TRUE) {
   qoi_index_array[qoi_index_array_pointer] = image_pixel;
   qoi_index_array_pointer++;
   if(qoi_index_array_pointer==64) {
    qoi_index_array_pointer = 1;
   }
  }
  previous_image_pixel = image_pixel;
 }
 if(run!=0) {
  if(run<=60) {
   //add RUN chunk
   qoi_image_data[0] = (0xC0 | (run-1));
   qoi_image_data++;
   qoi_file_length++;
  }
  else {
   //add LONG RUN chunk
   qoi_image_data[0] = 0xFC;
   qoi_image_data[1] = ((run-1) & 0xFF);
   qoi_image_data[2] = (((run-1)>>8) & 0xFF);
   qoi_image_data[3] = (((run-1)>>16) & 0xFF);
   qoi_image_data+=4;
   qoi_file_length+=4;
  }
 }

 //ending
 qoi_image_header = (dword_t *) ((dword_t)qoi_image_data);
 qoi_image_header[0] = 0x00000000;
 qoi_image_header[1] = 0x01000000;

 converted_file_memory = qoi_file_memory;
 converted_file_size = qoi_file_length;
}