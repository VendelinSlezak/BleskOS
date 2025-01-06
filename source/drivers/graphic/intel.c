//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_intel_graphic_card(byte_t graphic_card_number) {
 selected_graphic_card = graphic_card_number;
 is_driver_for_graphic_card = STATUS_TRUE;

 //find if this driver can change backlight
 word_t max_backlight_value = (mmio_ind(graphic_cards_info[graphic_card_number].mmio_base+GRAPHIC_CARD_INTEL_MMIO_BACKLIGHT_PWM_CONTROL) >> 16);
 if(max_backlight_value==0 || max_backlight_value==0xFFFF) {
  can_graphic_card_driver_change_backlight = STATUS_FALSE;
 }
 else {
  can_graphic_card_driver_change_backlight = STATUS_TRUE;
  graphic_card_driver_monitor_change_backlight = (&graphic_card_intel_change_backlight);
  graphic_card_intel_change_backlight(100);
 }

 //log info
 log("\n\nINTEL graphic card");
 log("\nDevice ID: "); log_hex_specific_size(graphic_cards_info[graphic_card_number].device_id, 4);
 log("\nLinear frame buffer: "); log_hex((dword_t)graphic_cards_info[graphic_card_number].linear_frame_buffer);
 log("\nPipe A mode: "); log_var((mmio_ind(graphic_cards_info[graphic_card_number].mmio_base+0x60000) & 0xFFFF)+1); log("x"); log_var((mmio_ind(graphic_cards_info[graphic_card_number].mmio_base+0x6000C) & 0xFFFF)+1);
 log("\nPipe B mode: "); log_var((mmio_ind(graphic_cards_info[graphic_card_number].mmio_base+0x61000) & 0xFFFF)+1); log("x"); log_var((mmio_ind(graphic_cards_info[graphic_card_number].mmio_base+0x6100C) & 0xFFFF)+1);
 log("\nBacklight port: "); log_hex(mmio_ind(graphic_cards_info[graphic_card_number].mmio_base+GRAPHIC_CARD_INTEL_MMIO_BACKLIGHT_PWM_CONTROL));
}

void graphic_card_intel_change_backlight(byte_t value) {
 if(is_driver_for_graphic_card==STATUS_FALSE || can_graphic_card_driver_change_backlight==STATUS_FALSE) {
  return;
 }

 word_t max_backlight_value = ((mmio_ind(graphic_cards_info[selected_graphic_card].mmio_base+GRAPHIC_CARD_INTEL_MMIO_BACKLIGHT_PWM_CONTROL) >> 16)-0x20); //too low values will shutdown display, so we are avoiding them
 mmio_outd(graphic_cards_info[selected_graphic_card].mmio_base+GRAPHIC_CARD_INTEL_MMIO_BACKLIGHT_PWM_CONTROL, ((max_backlight_value << 16) | (0x20+((max_backlight_value*value/100) & 0xFFFE))));

 percent_of_backlight = value;
}

void intel_try_read_edid(void) {
 dword_t *edid_memory_pointer = (dword_t *)calloc(128);

 //type of device
 mmio_outd(graphic_cards_info[selected_graphic_card].mmio_base+GRAPHIC_CARD_INTEL_MMIO_GMBUS0, 0b011);

 //read 128 bytes from offset 0x50
 mmio_outd(graphic_cards_info[selected_graphic_card].mmio_base+GRAPHIC_CARD_INTEL_MMIO_GMBUS1, ((1 << 30) | (0b011 << 25) | (128 << 16) | (0x50 << 1) | 0x1));
 
 for(dword_t i=0; i<32; i++) {
  //wait
  ticks = 0;
  while(ticks<10) {
   asm("nop");
   if((mmio_ind(graphic_cards_info[selected_graphic_card].mmio_base+GRAPHIC_CARD_INTEL_MMIO_GMBUS2) & (1<<11))==(1<<11)) {
    break;
   }
   if((mmio_ind(graphic_cards_info[selected_graphic_card].mmio_base+GRAPHIC_CARD_INTEL_MMIO_GMBUS2) & (1<<10))==(1<<10)) {
    log("\nNAK error");
    break;
   }
  }

  //read 4 bytes
  edid_memory_pointer[i] = mmio_ind(graphic_cards_info[selected_graphic_card].mmio_base+GRAPHIC_CARD_INTEL_MMIO_GMBUS3);
 }

 //stop transfer
 mmio_outd(graphic_cards_info[selected_graphic_card].mmio_base+GRAPHIC_CARD_INTEL_MMIO_GMBUS1, ((1 << 30) | (0b100 << 27)));

 parse_edid_data((dword_t)edid_memory_pointer);
 free((dword_t)edid_memory_pointer);
}