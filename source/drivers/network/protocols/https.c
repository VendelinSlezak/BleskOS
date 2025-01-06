//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

byte_t https_download_file_from_url(byte_t *url) {
 //check if URL is not too long
 if((get_number_of_chars_in_ascii_string(url)+22)>MAX_LENGTH_OF_URL) {
  return NETWORK_TRANSFER_INVALID_URL; //TODO:
 }

 //workaround: this site returns HTTPS pages through HTTP protocol
 byte_t workaround_url[MAX_LENGTH_OF_URL];
 dword_t pointer = 43;
 copy_memory((dword_t)"bleskos.com/https_to_http.php?site=https://", (dword_t)&workaround_url, 43);
 while(pointer<MAX_LENGTH_OF_URL && *url!=0) {
  workaround_url[pointer] = *url;
  pointer++;
  url++;
 }
 workaround_url[pointer] = 0;

 //now we can from this URL download site through HTTP protocol
 return http_download_file_from_url(workaround_url);
}