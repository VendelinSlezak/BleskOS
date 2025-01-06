//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t convert_jpg_to_image_data(dword_t jpg_memory, dword_t jpg_size) {
 //initalize nanojpeg
 njInit();

 //decode JPEG image by nanojpeg
 byte_t status = njDecode((void *)jpg_memory, jpg_size);
 if(status!=NJ_OK) {
  log("\nNANOJPEG error: ");
  if(status==NJ_NO_JPEG) {
   log("not JPG file");
  }
  else if(status==NJ_UNSUPPORTED) {
   log("unsupported format");
  }
  else if(status==NJ_OUT_OF_MEM) {
   log("uout of memory");
  }
  else if(status==NJ_INTERNAL_ERR) {
   log("internal error");
  }
  else if(status==NJ_SYNTAX_ERROR) {
   log("syntax error");
  }
  njDone();
  return STATUS_ERROR;
 }

 //copy RGB data to BleskOS image
 dword_t image_memory = create_image(njGetWidth(), njGetHeight());
 dword_t *image_data = (dword_t *) (get_image_data_memory(image_memory));
 byte_t *raw_rgb_data = (byte_t *) (njGetImage());
 for(dword_t i=0; i<(njGetWidth()*njGetHeight()); i++) {
  *image_data = (0xFF<<24 | raw_rgb_data[0]<<16 | raw_rgb_data[1]<<8 | raw_rgb_data[2]);
  image_data++;
  raw_rgb_data+=3;
 }

 //free memory used by nanojpeg
 njDone();

 return image_memory;
}

void convert_image_data_to_jpg(dword_t image_info_memory, byte_t quality) {
 dword_t *image_info = (dword_t *) (image_info_memory);

 //reverse 0xARGB to 0xABGR for encoder
 byte_t *image_data = (byte_t *) (get_image_data_memory(image_info_memory));
 for(dword_t i=0; i<image_info[IMAGE_INFO_REAL_WIDTH]*image_info[IMAGE_INFO_REAL_HEIGHT]; i++) {
  byte_t blue = image_data[0];
  image_data[0] = image_data[2]; //move red
  image_data[2] = blue; //move blue
  image_data += 4;
 }

 //create JPG file
 struct byte_stream_descriptor_t *jpeg = jo_write_jpg((byte_t *)get_image_data_memory(image_info_memory), image_info[IMAGE_INFO_REAL_WIDTH], image_info[IMAGE_INFO_REAL_HEIGHT], 4, quality);
 converted_file_size = jpeg->size_of_stream;
 converted_file_memory = (dword_t) close_byte_stream(jpeg);

 //reverse 0xABGR to 0xARGB
 image_data = (byte_t *) (get_image_data_memory(image_info_memory));
 for(dword_t i=0; i<image_info[IMAGE_INFO_REAL_WIDTH]*image_info[IMAGE_INFO_REAL_HEIGHT]; i++) {
  byte_t red = image_data[0];
  image_data[0] = image_data[2]; //move blue
  image_data[2] = red; //move red
  image_data += 4;
 }
}