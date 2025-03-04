//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/* initalization of xHCI controller at boot */

void initalize_xhci_controller(dword_t number_of_controller) {
 //log
 l("\n\nXHCI controller ");

 //disable BIOS ownership
 dword_t xhci_bios_register_offset = ((mmio_ind(xhci_controllers[number_of_xhci_controllers].base+0x10)>>16)*4);
 if(xhci_bios_register_offset != 0 && (mmio_ind(xhci_controllers[number_of_xhci_controllers].base+xhci_bios_register_offset) & 0xFF)==0x01) {
  //check if BIOS released ownership
  if(((mmio_ind(xhci_controllers[number_of_xhci_controllers].base+xhci_bios_register_offset) & ((1 << 16) | (1 << 24))) == (1 << 24))) {
   //BIOS did not released ownership
   logf("\nERROR: xHCI controller is still in BIOS ownership %x", mmio_ind(xhci_controllers[number_of_xhci_controllers].base+xhci_bios_register_offset));
   return;
  }
  else {
   l("\nBIOS ownership released");
  }
 }
}