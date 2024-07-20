//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define GRAPHIC_EDITOR_FILE_IMAGE_INFO_MEMORY (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+0)
#define GRAPHIC_EDITOR_FILE_IMAGE_PREVIEW_MEMORY (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+1)
#define GRAPHIC_EDITOR_FILE_IMAGE_ZOOM (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+2)

#define GRAPHIC_EDITOR_SIDE_PANEL_WIDTH 176
#define GRAPHIC_EDITOR_BOTTOM_PANEL_HEIGHT 20
#define GRAPHIC_EDITOR_COLOR_AREA_LINE 21
#define GRAPHIC_EDITOR_TOOLS_AREA_LINE 21+20+25+160
#define GRAPHIC_EDITOR_TOOL_STATE_AREA_LINE 21+20+25+160+20+20*5

#define GRAPHIC_EDITOR_CLICK_ZONE_BUTTON_TAKE_COLOR 100
#define GRAPHIC_EDITOR_CLICK_ZONE_COLOR_PALETTE 101
#define GRAPHIC_EDITOR_CLICK_ZONE_TOOL_PEN 110
#define GRAPHIC_EDITOR_CLICK_ZONE_TOOL_LINE 111
#define GRAPHIC_EDITOR_CLICK_ZONE_TOOL_SQUARE 112
#define GRAPHIC_EDITOR_CLICK_ZONE_TOOL_CIRCLE 113
#define GRAPHIC_EDITOR_CLICK_ZONE_TOOL_FILL 114
#define GRAPHIC_EDITOR_CLICK_ZONE_TOOL_STATE_EMPTY 120
#define GRAPHIC_EDITOR_CLICK_ZONE_TOOL_STATE_FULL 121
#define GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_1 200
#define GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_2 201
#define GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_3 202
#define GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_5 204
#define GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_10 210
#define GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_20 220
#define GRAPHIC_EDITOR_CLICK_ZONE_PEN_SIZE_TEXT_AREA 299
#define GRAPHIC_EDITOR_CLICK_ZONE_IMAGE_ZOOM_25 325
#define GRAPHIC_EDITOR_CLICK_ZONE_IMAGE_ZOOM_50 350
#define GRAPHIC_EDITOR_CLICK_ZONE_IMAGE_ZOOM_100 400
#define GRAPHIC_EDITOR_CLICK_ZONE_IMAGE_ZOOM_200 500
#define GRAPHIC_EDITOR_CLICK_ZONE_IMAGE_ZOOM_300 600
#define GRAPHIC_EDITOR_CLICK_ZONE_IMAGE_ZOOM_400 700
#define GRAPHIC_EDITOR_CLICK_ZONE_IMAGE 1000
#define GRAPHIC_EDITOR_CLICK_ZONE_VERTICAL_SCROLLBAR 1001
#define GRAPHIC_EDITOR_CLICK_ZONE_HORIZONTAL_SCROLLBAR 1002
#define GRAPHIC_EDITOR_CLICK_ZONE_TEXT_AREA_WIDTH 1003
#define GRAPHIC_EDITOR_CLICK_ZONE_TEXT_AREA_HEIGHT 1004
#define GRAPHIC_EDITOR_CLICK_ZONE_EDIT_IMAGE 1005

#define GRAPHIC_EDITOR_TOOL_PEN 0
#define GRAPHIC_EDITOR_TOOL_LINE 1
#define GRAPHIC_EDITOR_TOOL_SQUARE 2
#define GRAPHIC_EDITOR_TOOL_CIRCLE 3
#define GRAPHIC_EDITOR_TOOL_FILL 4
#define GRAPHIC_EDITOR_TOOL_STATE_EMPTY 0
#define GRAPHIC_EDITOR_TOOL_STATE_FULL 1

dword_t graphic_editor_pallette[64] = {
0xFFFFFF, 0xE0E0E0, 0xC0C0C0, 0xA0A0A0, 0x808080, 0x606060, 0x404040, 0x000000,
0xFFCCCC, 0xFF9999, 0xFF6666, 0xFF3333, 0xFF0000, 0xCC0000, 0x990000, 0x660000,
0xFFE5CC, 0xFFCC99, 0xFFB266, 0xFF9933, 0xFF8000, 0xCC6600, 0x994C00, 0x663300,
0xFFFFCC, 0xFFFF99, 0xFFFF66, 0xFFFF33, 0xFFFF00, 0xCCCC00, 0x999900, 0x333300,
0xE5FFCC, 0xCCFF99, 0xB2FF66, 0x99FF33, 0x80FF00, 0x00CC00, 0x009900, 0x006600,
0xCCFFFF, 0x99FFFF, 0x66FFFF, 0x33FFFF, 0x00FFFF, 0x0088FF, 0x0000FF, 0x000099,
0xE5CCFF, 0xCC99FF, 0xB266FF, 0x9933FF, 0x7F00FF, 0x6600CC, 0x4C0099, 0x330066,
0xFFCCFF, 0xFF99FF, 0xFF66FF, 0xFF33FF, 0xFF00FF, 0xCC00CC, 0x990099, 0x660066 };

dword_t graphic_editor_image_area_width = 0, graphic_editor_image_area_width_center = 0, graphic_editor_image_area_height = 0, graphic_editor_image_area_height_center = 0;
dword_t graphic_editor_pen_size_text_area_info_memory = 0, graphic_editor_width_text_area_info_memory = 0, graphic_editor_height_text_area_info_memory = 0;
dword_t graphic_editor_color = 0, graphic_editor_tool = 0, graphic_editor_tool_state = 0, graphic_editor_pen_size = 0;
dword_t graphic_editor_program_interface_memory = 0;
dword_t graphic_editor_mouse_cursor_previous_image_x = 0, graphic_editor_mouse_cursor_previous_image_y = 0;
dword_t graphic_editor_image_saved_to_preview = 0;

void initalize_graphic_editor(void);
void graphic_editor(void);
void draw_graphic_editor(void);
void graphic_editor_redraw_image(void);
void graphic_editor_open_file(void);
void graphic_editor_save_file(void);
void graphic_editor_new_file(void);
void graphic_editor_close_file(void);
void graphic_editor_key_f7_event(void);
void graphic_editor_key_up_event(void);
void graphic_editor_key_down_event(void);
void graphic_editor_key_left_event(void);
void graphic_editor_key_right_event(void);
void graphic_editor_key_a_event(void);
void graphic_editor_key_s_event(void);
void graphic_editor_key_d_event(void);
void graphic_editor_key_c_event(void);
void graphic_editor_key_g_event(void);
void graphic_editor_key_e_event(void);
void graphic_editor_key_f_event(void);
void graphic_editor_key_t_event(void);
void graphic_editor_key_1_event(void);
void graphic_editor_key_2_event(void);
void graphic_editor_key_3_event(void);
void graphic_editor_key_5_event(void);
void graphic_editor_key_0_event(void);
void graphic_editor_key_enter_event(void);
void graphic_editor_click_zone_event_palette(void);
void graphic_editor_click_zone_event_pen_size_20(void);
void graphic_editor_click_zone_event_zoom_25(void);
void graphic_editor_click_zone_event_zoom_50(void);
void graphic_editor_click_zone_event_zoom_100(void);
void graphic_editor_click_zone_event_zoom_200(void);
void graphic_editor_click_zone_event_zoom_300(void);
void graphic_editor_click_zone_event_zoom_400(void);
void graphic_editor_image_vertical_scrollbar_event(void);
void graphic_editor_image_horizontal_scrollbar_event(void);
void graphic_editor_set_image_zoom(dword_t zoom);
void graphic_editor_image_recalculate_scrollbars(void);
void graphic_editor_image_recalculate_zoom(void);
void graphic_editor_copy_image_to_preview(void);
void graphic_editor_copy_preview_to_image(void);
void graphic_editor_prepare_drawing_by_tool_without_copying_preview(void);
void graphic_editor_prepare_drawing_by_tool(void);
void graphic_editor_finish_drawing_by_tool(void);