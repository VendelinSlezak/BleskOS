//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t wait_for_event(dword_t *event_interface) {
 void (*method)(void);
 dword_t (*method_with_return)(void);
 void (*scrollbar_change_method)(dword_t value);

 //wait for some event to happen
 wait_for_user_input();

 //move mouse cursor
 move_mouse_cursor();

 //find out how to react to this event
 if(usb_new_device_detected==STATUS_TRUE) {
  return EVENT_USB_DEVICE_CHANGE;
 }

 //get mouse click zone
 dword_t click_zone = get_mouse_cursor_click_board_value();

 //get through all events in array
 dword_t *focus = event_interface;
 *event_interface++; //first dword is for focus
 while(1) {
  //end of array of events
  if(event_interface[0]==END_OF_EVENTS) {
   break;
  }

  //pressed key event
  if(event_interface[0]==KEYBOARD_EVENT_PRESSED_KEY) {
   if(keyboard_code_of_pressed_key==event_interface[1]) {
    //call method
    if(event_interface[2]!=0) {
     if(event_interface[3]==RETURN_EVENT_FROM_METHOD) {
      method_with_return = (dword_t (*)(void)) event_interface[2];
      return (*method_with_return)();
     }
     else {
      method = (void (*)(void)) event_interface[2];
      (*method)();
     }
    }

    //return
    return event_interface[3];
   }

   //go to next event
   event_interface += 4;
  }

  //click to mouse zone event
  if(event_interface[0]==MOUSE_EVENT_CLICK_ON_ZONE) {
   if(mouse_click_button_state==MOUSE_CLICK && click_zone==event_interface[1]) {
    //call method
    if(event_interface[2]!=0) {
     if(event_interface[3]==RETURN_EVENT_FROM_METHOD) {
      method_with_return = (dword_t (*)(void)) event_interface[2];
      return (*method_with_return)();
     }
     else {
      method = (void (*)(void)) event_interface[2];
      (*method)();
     }
    }

    //return
    return event_interface[3];
   }

   //go to next event
   event_interface += 4;
  }

  //click to mouse zones event
  if(event_interface[0]==MOUSE_EVENT_CLICK_ON_ZONES) {
   if(mouse_click_button_state==MOUSE_CLICK && click_zone>=event_interface[1] && click_zone<=event_interface[2]) {
    //call method
    if(event_interface[3]!=0) {
     if(event_interface[4]==RETURN_EVENT_FROM_METHOD) {
      method_with_return = (dword_t (*)(void)) event_interface[3];
      return (*method_with_return)();
     }
     else {
      method = (void (*)(void)) event_interface[3];
      (*method)();
     }
    }

    //return
    return event_interface[4];
   }

   //go to next event
   event_interface += 5;
  }

  //scrollbar
  if(event_interface[0]==VERTICAL_SCROLLBAR_EVENT || event_interface[0]==HORIZONTAL_SCROLLBAR_EVENT) {
   //scrollbar is focused
   if(*focus==(dword_t)event_interface) {
    //move scrollbar
    if(mouse_click_button_state==MOUSE_DRAG) {
     struct scrollbar_info_t *scrollbar_info = (struct scrollbar_info_t *) (event_interface[2]);

     dword_t movement = 0;
     if(event_interface[0]==VERTICAL_SCROLLBAR_EVENT) {
      movement = (mouse_cursor_y_dnd-mouse_cursor_y_previous_dnd);
     }
     else {
      movement = (mouse_cursor_x_dnd-mouse_cursor_x_previous_dnd);
     }
     scrollbar_info->rider_position += movement;
     if(movement<0x80000000 && scrollbar_info->rider_position>(scrollbar_info->size-scrollbar_info->rider_size)) {
      scrollbar_info->rider_position = (scrollbar_info->size-scrollbar_info->rider_size);
     }
     if(movement>0x80000000 && scrollbar_info->rider_position>0x80000000) {
      scrollbar_info->rider_position = 0;
     }

     //call method
     scrollbar_change_method = (void (*)(dword_t value)) event_interface[3];
     (*scrollbar_change_method)(get_scrollbar_rider_value(scrollbar_info->size, scrollbar_info->rider_size, scrollbar_info->rider_position, scrollbar_info->full_document_size, scrollbar_info->showed_document_size));
     return NO_EVENT;
    }
    else {
     *focus = 0; //scrollbar is not focused
    }
   }
   else { //scrollbar is not focused
    //click on scrollbar
    if(click_zone==event_interface[1] && mouse_click_button_state==MOUSE_CLICK) {
     *focus = (dword_t)event_interface; //set focus on this scrollbar
    }
   }

   //go to next event
   event_interface += 4;
  }

  //text area
  if(event_interface[0]==TEXT_AREA_WITH_PERMANENT_FOCUS_EVENT) {
   text_area_keyboard_event(event_interface[1]);
   text_area_mouse_event(event_interface[1]);
   draw_text_area(event_interface[1]);
   redraw_text_area(event_interface[1]);

   //go to next event
   event_interface += 2;
  }
 }

 return NO_EVENT;
}

//TODO: later when text area code will be rewritten, delete this
void event_interface_set_text_area(dword_t *event_interface, dword_t text_area_pointer) {
 dword_t *focus = event_interface;
 *event_interface++; //first dword is for focus

 while(1) {
  //end of array of events
  if(event_interface[0]==END_OF_EVENTS) {
   return;
  }

  //pressed key event
  if(event_interface[0]==KEYBOARD_EVENT_PRESSED_KEY) {
   //go to next event
   event_interface += 4;
  }

  //click to mouse zone event
  if(event_interface[0]==MOUSE_EVENT_CLICK_ON_ZONE) {
   //go to next event
   event_interface += 4;
  }

  //click to mouse zones event
  if(event_interface[0]==MOUSE_EVENT_CLICK_ON_ZONES) {
   //go to next event
   event_interface += 5;
  }

  //scrollbar
  if(event_interface[0]==VERTICAL_SCROLLBAR_EVENT || event_interface[0]==HORIZONTAL_SCROLLBAR_EVENT) {
   //go to next event
   event_interface += 4;
  }

  //text area
  if(event_interface[0]==TEXT_AREA_WITH_PERMANENT_FOCUS_EVENT) {
   event_interface[1] = text_area_pointer;
   return;
  }
 }
}