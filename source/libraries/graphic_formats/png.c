//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t convert_png_to_image_data(dword_t png_memory, dword_t png_file_length) {
 byte_t *png8 = (byte_t *) png_memory;
 word_t *png16 = (word_t *) png_memory;
 dword_t *png32 = (dword_t *) (png_memory+8);
 dword_t width, height, bpp, bits_per_channel;
 dword_t image_memory = 0, palette_memory = 0;
 dword_t *image_data = (dword_t *) image_memory;
 dword_t deflate_compressed_data_memory = 0, deflate_compressed_data_memory_length = 0, deflate_decompressed_data_memory = 0;
 
 //check signature
 if(png8[0]!=0x89 || png8[1]!='P' || png8[2]!='N' || png8[3]!='G' || png8[4]!=0x0D || png8[5]!=0x0A || png8[6]!=0x1A || png8[7]!=0x0A) {
  return STATUS_ERROR;
 }
 
 //read chunks
 while((dword_t)png32<(png_memory+png_file_length)) {
  if(png32[1]==0x52444849) { //IHDR   
   //size of image
   png8 = (byte_t *) ((dword_t)png32+8);
   width = (png8[0]<<24 | png8[1]<<16 | png8[2]<<8 | png8[3]);
   height = (png8[4]<<24 | png8[5]<<16 | png8[6]<<8 | png8[7]);
   
   //bits per channel 
   //TODO: other values for bits per channel than 8
   png8 = (byte_t *) ((dword_t)png32+16);
   bits_per_channel = png8[0];
   if(bits_per_channel!=8) {
    log("PNG: unsupported bits per channel ");
    log_var(bits_per_channel);
    log("\n");
    return STATUS_ERROR;
   }
  
   //color type
   if(png8[1]==PNG_COLOR_TYPE_TRUECOLOR) {
    bpp = 3;
   }
   else if(png8[1]==PNG_COLOR_TYPE_TRUECOLOR_ALPHA) {
    bpp = 4;
   }
   else if(png8[1]==PNG_COLOR_TYPE_GRAYSCALE) {
    bpp = 1;
   }
   else if(png8[1]==PNG_COLOR_TYPE_GRAYSCALE_ALPHA) {
    bpp = 2;
   }
   else if(png8[1]==PNG_COLOR_TYPE_PALETTE) {
    bpp = PNG_BPP_PALETTE_8_BITS;
   }
   else {
    log("PNG: invalid color type ");
    log_var(png8[1]);
    log("\n");
    return STATUS_ERROR;
   }
   
   //compression method
   if(png8[3]!=0) {
    log("PNG: unknown compression method\n");
    return STATUS_ERROR;
   }
   
   //interlacing TODO: support for interlacing
   if(png8[4]!=0) {
    log("PNG: interlaced image\n");
    return STATUS_ERROR;
   }
   
   //test limits for image
   if(width==0 || height==0 || bpp<1 || bpp>5 || (width*height)>4096*4096) {
    log("PNG: not fitting size\n");
    return STATUS_ERROR;
   }
   
   //create image
   image_memory = create_image(width, height);
   image_data = (dword_t *) (get_image_data_memory(image_memory));
   deflate_compressed_data_memory = (dword_t) (calloc(png_file_length));
   deflate_compressed_data_memory_length = 0;
   deflate_decompressed_data_memory = (dword_t) calloc((width*height*bpp+height));
  }
  else if(png32[1]==0x45544C50) { //PLTE
   palette_memory = ((dword_t)png32+8);
  }
  else if(png32[1]==0x54414449) { //IDAT  
   if(image_memory==0) {
    return STATUS_ERROR;
   }
   
   //copy deflate data
   png8 = (byte_t *) ((dword_t)png32);
   copy_memory(((dword_t)png32+8), (deflate_compressed_data_memory+deflate_compressed_data_memory_length), (png8[0]<<24 | png8[1]<<16 | png8[2]<<8 | png8[3]));
   deflate_compressed_data_memory_length += (png8[0]<<24 | png8[1]<<16 | png8[2]<<8 | png8[3]);
  }
  else if(png32[1]==0x444E4549) { //IEND
   if(bpp==PNG_BPP_PALETTE_8_BITS && palette_memory==0) {
    log("PNG: no palette\n");
    
    delete_image(image_memory);
    free((void *)deflate_compressed_data_memory);
    free((void *)deflate_decompressed_data_memory);
    return STATUS_ERROR;
   }
   else if(bpp==PNG_BPP_PALETTE_8_BITS && bits_per_channel==16) {
    log("PNG: palette with 16 bit channel\n");
    
    delete_image(image_memory);
    free((void *)deflate_compressed_data_memory);
    free((void *)deflate_decompressed_data_memory);
    return STATUS_ERROR;
   }
     
   //decompress deflate
   png8 = (byte_t *) ((dword_t)png32);
   dword_t expected_size_of_file = (width*height*bpp*(bits_per_channel/8)+height);
   if(bpp==PNG_BPP_PALETTE_8_BITS) {
    expected_size_of_file = (width*height*(bits_per_channel/8)+height);
   }
   if(decode_deflate(deflate_compressed_data_memory+2, deflate_compressed_data_memory_length-6, deflate_decompressed_data_memory, expected_size_of_file)==STATUS_ERROR) {
    log("PNG: DEFLATE decompression error\n");
    
    delete_image(image_memory);
    free((void *)deflate_compressed_data_memory);
    free((void *)deflate_decompressed_data_memory);
    return STATUS_ERROR;
   }
   if(decoded_stream_length!=expected_size_of_file) {
    log("PNG: DEFLATE did not decoded succesfully\n");
    log_var_with_space(width);
    log_var_with_space(height);
    log_var_with_space(bpp);
    log_var_with_space(bits_per_channel);
    log_var_with_space(expected_size_of_file);
    log_var(decoded_stream_length);
    
    delete_image(image_memory);
    free((void *)deflate_compressed_data_memory);
    free((void *)deflate_decompressed_data_memory);
    return STATUS_ERROR;
   }
   
   //decompress filtering
   byte_t *png_data = (byte_t *) deflate_decompressed_data_memory;
   byte_t *png_palette_data = (byte_t *) palette_memory;
   dword_t filtering_previous_line_r_memory = (dword_t) calloc(width);
   dword_t filtering_previous_line_g_memory = (dword_t) calloc(width);
   dword_t filtering_previous_line_b_memory = (dword_t) calloc(width);
   byte_t pixel_r, pixel_g, pixel_b, previous_pixel_r, previous_pixel_g, previous_pixel_b, previous_up_pixel_r, previous_up_pixel_g, previous_up_pixel_b;
   
   for(int line=0; line<height; line++) {
    byte_t *filtering_previous_line_r = (byte_t *) filtering_previous_line_r_memory;
    byte_t *filtering_previous_line_g = (byte_t *) filtering_previous_line_g_memory;
    byte_t *filtering_previous_line_b = (byte_t *) filtering_previous_line_b_memory;
    pixel_r = 0; pixel_g = 0; pixel_b = 0;
    previous_pixel_r = 0; previous_pixel_g = 0; previous_pixel_b = 0;
    previous_up_pixel_r = 0; previous_up_pixel_g = 0; previous_up_pixel_b = 0;
    
    if(*png_data==PNG_FILTERING_NONE) {
     //skip filtering type byte
     png_data++;
     
     for(int column=0; column<width; column++) {
      //get pixel data
      if(bpp==3) {
       pixel_r = png_data[0];
       pixel_g = png_data[1];
       pixel_b = png_data[2];
       png_data += 3;
      }
      else if(bpp==4) {
       pixel_r = png_data[0];
       pixel_g = png_data[1];
       pixel_b = png_data[2];
       png_data += 4;
      }
      else if(bpp==PNG_BPP_PALETTE_8_BITS) {
       pixel_r = png_palette_data[png_data[0]*3+0];
       pixel_g = png_palette_data[png_data[0]*3+1];
       pixel_b = png_palette_data[png_data[0]*3+2];
       png_data += 1;
      }
      else if(bpp==1) {
       pixel_r = png_data[0];
       pixel_g = png_data[0];
       pixel_b = png_data[0];
       png_data += 1;
      }
      else if(bpp==2) {
       pixel_r = png_data[0];
       pixel_g = png_data[0];
       pixel_b = png_data[0];
       png_data += 2;
      }
     
      //write pixel      
      *image_data = (0xFF<<24 | pixel_r<<16 | pixel_g<<8 | pixel_b);
      
      //update values
      *filtering_previous_line_r = pixel_r;
      filtering_previous_line_r++;
      *filtering_previous_line_g = pixel_g;
      filtering_previous_line_g++; 
      *filtering_previous_line_b = pixel_b;
      filtering_previous_line_b++;     
      image_data++;
     }
    }
    else if(*png_data==PNG_FILTERING_SUB) {          
     //skip filtering type byte
     png_data++;
     
     for(int column=0; column<width; column++) {
      //get pixel data
      if(bpp==3) {
       pixel_r = png_data[0];
       pixel_g = png_data[1];
       pixel_b = png_data[2];
       png_data += 3;
      }
      else if(bpp==4) {
       pixel_r = png_data[0];
       pixel_g = png_data[1];
       pixel_b = png_data[2];
       png_data += 4;
      }
      else if(bpp==PNG_BPP_PALETTE_8_BITS) {
       pixel_r = png_palette_data[png_data[0]*3+0];
       pixel_g = png_palette_data[png_data[0]*3+1];
       pixel_b = png_palette_data[png_data[0]*3+2];
       png_data += 1;
      }
      else if(bpp==1) {
       pixel_r = png_data[0];
       pixel_g = png_data[0];
       pixel_b = png_data[0];
       png_data += 1;
      }
      else if(bpp==2) {
       pixel_r = png_data[0];
       pixel_g = png_data[0];
       pixel_b = png_data[0];
       png_data += 2;
      }
      
      //reverse SUB filtering
      pixel_r += previous_pixel_r;
      pixel_g += previous_pixel_g;
      pixel_b += previous_pixel_b;
      
      //write pixel      
      *image_data = (0xFF<<24 | pixel_r<<16 | pixel_g<<8 | pixel_b);
      
      //update values
      previous_pixel_r = pixel_r;
      previous_pixel_g = pixel_g;
      previous_pixel_b = pixel_b;
      *filtering_previous_line_r = pixel_r;
      filtering_previous_line_r++;
      *filtering_previous_line_g = pixel_g;
      filtering_previous_line_g++; 
      *filtering_previous_line_b = pixel_b;
      filtering_previous_line_b++;     
      image_data++;
     }
    }
    else if(*png_data==PNG_FILTERING_UP) {
     //skip filtering type byte
     png_data++;
     
     for(int column=0; column<width; column++) {
      //get pixel data
      if(bpp==3) {
       pixel_r = png_data[0];
       pixel_g = png_data[1];
       pixel_b = png_data[2];
       png_data += 3;
      }
      else if(bpp==4) {
       pixel_r = png_data[0];
       pixel_g = png_data[1];
       pixel_b = png_data[2];
       png_data += 4;
      }
      else if(bpp==PNG_BPP_PALETTE_8_BITS) {
       pixel_r = png_palette_data[png_data[0]*3+0];
       pixel_g = png_palette_data[png_data[0]*3+1];
       pixel_b = png_palette_data[png_data[0]*3+2];
       png_data += 1;
      }
      else if(bpp==1) {
       pixel_r = png_data[0];
       pixel_g = png_data[0];
       pixel_b = png_data[0];
       png_data += 1;
      }
      else if(bpp==2) {
       pixel_r = png_data[0];
       pixel_g = png_data[0];
       pixel_b = png_data[0];
       png_data += 2;
      }
      
      //reverse UP filtering
      pixel_r += *filtering_previous_line_r;
      pixel_g += *filtering_previous_line_g;
      pixel_b += *filtering_previous_line_b;
            
      //write pixel      
      *image_data = (0xFF<<24 | pixel_r<<16 | pixel_g<<8 | pixel_b);
      
      //update values
      *filtering_previous_line_r = pixel_r;
      filtering_previous_line_r++;
      *filtering_previous_line_g = pixel_g;
      filtering_previous_line_g++; 
      *filtering_previous_line_b = pixel_b;
      filtering_previous_line_b++;     
      image_data++;
     }
    }
    else if(*png_data==PNG_FILTERING_AVERAGE) {      
     //skip filtering type byte
     png_data++;
     
     for(int column=0; column<width; column++) {
      //get pixel data
      if(bpp==3) {
       pixel_r = png_data[0];
       pixel_g = png_data[1];
       pixel_b = png_data[2];
       png_data += 3;
      }
      else if(bpp==4) {
       pixel_r = png_data[0];
       pixel_g = png_data[1];
       pixel_b = png_data[2];
       png_data += 4;
      }
      else if(bpp==PNG_BPP_PALETTE_8_BITS) {
       pixel_r = png_palette_data[png_data[0]*3+0];
       pixel_g = png_palette_data[png_data[0]*3+1];
       pixel_b = png_palette_data[png_data[0]*3+2];
       png_data += 1;
      }
      else if(bpp==1) {
       pixel_r = png_data[0];
       pixel_g = png_data[0];
       pixel_b = png_data[0];
       png_data += 1;
      }
      else if(bpp==2) {
       pixel_r = png_data[0];
       pixel_g = png_data[0];
       pixel_b = png_data[0];
       png_data += 2;
      }
      
      //reverse AVERAGE filtering
      pixel_r += floor((previous_pixel_r+*filtering_previous_line_r)/2);
      pixel_g += floor((previous_pixel_g+*filtering_previous_line_g)/2);
      pixel_b += floor((previous_pixel_b+*filtering_previous_line_b)/2);
      
      //write pixel
      *image_data = (0xFF<<24 | pixel_r<<16 | pixel_g<<8 | pixel_b);
      
      //update values
      previous_pixel_r = pixel_r;
      previous_pixel_g = pixel_g;
      previous_pixel_b = pixel_b;
      *filtering_previous_line_r = pixel_r;
      filtering_previous_line_r++;
      *filtering_previous_line_g = pixel_g;
      filtering_previous_line_g++;
      *filtering_previous_line_b = pixel_b;
      filtering_previous_line_b++;
      image_data++;
     }
    }
    else if(*png_data==PNG_FILTERING_PAETH) {
     //skip filtering type byte
     png_data++;
     
     for(int column=0; column<width; column++) {
      //get pixel data
      if(bpp==3) {
       pixel_r = png_data[0];
       pixel_g = png_data[1];
       pixel_b = png_data[2];
       png_data += 3;
      }
      else if(bpp==4) {
       pixel_r = png_data[0];
       pixel_g = png_data[1];
       pixel_b = png_data[2];
       png_data += 4;
      }
      else if(bpp==PNG_BPP_PALETTE_8_BITS) {
       pixel_r = png_palette_data[png_data[0]*3+0];
       pixel_g = png_palette_data[png_data[0]*3+1];
       pixel_b = png_palette_data[png_data[0]*3+2];
       png_data += 1;
      }
      else if(bpp==1) {
       pixel_r = png_data[0];
       pixel_g = png_data[0];
       pixel_b = png_data[0];
       png_data += 1;
      }
      else if(bpp==2) {
       pixel_r = png_data[0];
       pixel_g = png_data[0];
       pixel_b = png_data[0];
       png_data += 2;
      }
      
      //reverse PAETH filtering
      dword_t paeth_middle = (previous_pixel_r+*filtering_previous_line_r-previous_up_pixel_r);
      dword_t paeth_previous_pixel_difference = abs(paeth_middle-previous_pixel_r);
      dword_t paeth_previous_line_difference = abs(paeth_middle-*filtering_previous_line_r);
      dword_t paeth_previous_up_pixel_difference = abs(paeth_middle-previous_up_pixel_r);
      if(paeth_previous_pixel_difference<=paeth_previous_line_difference && paeth_previous_pixel_difference<=paeth_previous_up_pixel_difference) {
       pixel_r += previous_pixel_r;
      }
      else if(paeth_previous_line_difference<=paeth_previous_up_pixel_difference) {
       pixel_r += *filtering_previous_line_r;
      }
      else {
       pixel_r += previous_up_pixel_r;
      }
      paeth_middle = (previous_pixel_g+*filtering_previous_line_g-previous_up_pixel_g);
      paeth_previous_pixel_difference = abs(paeth_middle-previous_pixel_g);
      paeth_previous_line_difference = abs(paeth_middle-*filtering_previous_line_g);
      paeth_previous_up_pixel_difference = abs(paeth_middle-previous_up_pixel_g);
      if(paeth_previous_pixel_difference<=paeth_previous_line_difference && paeth_previous_pixel_difference<=paeth_previous_up_pixel_difference) {
       pixel_g += previous_pixel_g;
      }
      else if(paeth_previous_line_difference<=paeth_previous_up_pixel_difference) {
       pixel_g += *filtering_previous_line_g;
      }
      else {
       pixel_g += previous_up_pixel_g;
      }
      paeth_middle = (previous_pixel_b+*filtering_previous_line_b-previous_up_pixel_b);
      paeth_previous_pixel_difference = abs(paeth_middle-previous_pixel_b);
      paeth_previous_line_difference = abs(paeth_middle-*filtering_previous_line_b);
      paeth_previous_up_pixel_difference = abs(paeth_middle-previous_up_pixel_b);
      if(paeth_previous_pixel_difference<=paeth_previous_line_difference && paeth_previous_pixel_difference<=paeth_previous_up_pixel_difference) {
       pixel_b += previous_pixel_b;
      }
      else if(paeth_previous_line_difference<=paeth_previous_up_pixel_difference) {
       pixel_b += *filtering_previous_line_b;
      }
      else {
       pixel_b += previous_up_pixel_b;
      }
      
      //write pixel
      *image_data = (0xFF<<24 | pixel_r<<16 | pixel_g<<8 | pixel_b);
      
      //update values
      previous_pixel_r = pixel_r;
      previous_pixel_g = pixel_g;
      previous_pixel_b = pixel_b;
      previous_up_pixel_r = *filtering_previous_line_r;
      previous_up_pixel_g = *filtering_previous_line_g;
      previous_up_pixel_b = *filtering_previous_line_b;
      *filtering_previous_line_r = pixel_r;
      filtering_previous_line_r++;
      *filtering_previous_line_g = pixel_g;
      filtering_previous_line_g++;
      *filtering_previous_line_b = pixel_b;
      filtering_previous_line_b++;
      image_data++;
     }
    }
    else {
     log("PNG: unknown filtering ");
     log_var(*png_data);
     log("\n");
     
     free((void *)filtering_previous_line_r_memory);
     free((void *)filtering_previous_line_g_memory);
     free((void *)filtering_previous_line_b_memory);
     delete_image(image_memory);
     free((void *)deflate_compressed_data_memory);
     free((void *)deflate_decompressed_data_memory);
     return STATUS_ERROR;
    }
   }
   free((void *)filtering_previous_line_r_memory);
   free((void *)filtering_previous_line_g_memory);
   free((void *)filtering_previous_line_b_memory);
   free((void *)deflate_compressed_data_memory);
   free((void *)deflate_decompressed_data_memory);
 
   return image_memory;
  }
  else {
   //unknown chunk
  }
  
  //skip chunk
  png8 = (byte_t *) ((dword_t)png32);
  png32 = (dword_t *) ((dword_t)png32+12+(png8[0]<<24 | png8[1]<<16 | png8[2]<<8 | png8[3]));
 }
 
 if(image_memory!=0) {
  delete_image(image_memory);
  free((void *)deflate_compressed_data_memory);
  free((void *)deflate_decompressed_data_memory);
 }
 
 return STATUS_ERROR;
}
