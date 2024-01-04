//BleskOS

/*
* MIT License
* Copyright (c) 2023-2024 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define MEDIA_VIEWER_FILE_TYPE (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+0)
#define MEDIA_VIEWER_FILE_IMAGE_INFO_MEMORY (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+1)
#define MEDIA_VIEWER_FILE_IMAGE_ZOOM (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+2)
#define MEDIA_VIEWER_FILE_SOUND_INFO_MEMORY (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+1)
#define MEDIA_VIEWER_FILE_SOUND_ORIGINAL_FILE_MEMORY (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+2)
#define MEDIA_VIEWER_FILE_SOUND_ORIGINAL_FILE_SIZE (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+3)
#define MEDIA_VIEWER_FILE_SOUND_LENGTH_IN_MS (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+4)
#define MEDIA_VIEWER_FILE_SOUND_ACTUAL_MS (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+5)
#define MEDIA_VIEWER_FILE_SOUND_NEXT_UPDATE_MS (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+6)
#define MEDIA_VIEWER_FILE_SOUND_ORIGINAL_FILE_TYPE (PROGRAM_INTERFACE_FILE_PROGRAM_AREA+7)

#define MEDIA_VIEWER_FILE_IMAGE 1
#define MEDIA_VIEWER_FILE_SOUND 2

#define MEDIA_VIEWER_SOUND_WAV 1
#define MEDIA_VIEWER_SOUND_CDDA 2

#define MEDIA_VIEWER_SOUND_NO_FILE 0
#define MEDIA_VIEWER_SOUND_STATE_STOPPED 1
#define MEDIA_VIEWER_SOUND_STATE_PLAYING 2
#define MEDIA_VIEWER_SOUND_STATE_PAUSED 3

#define MEDIA_VIEWER_CLICK_ZONE_IMAGE 100
#define MEDIA_VIEWER_CLICK_ZONE_IMAGE_VERTICAL_SCROLLBAR 101
#define MEDIA_VIEWER_CLICK_ZONE_IMAGE_HORIZONTAL_SCROLLBAR 102
#define MEDIA_VIEWER_CLICK_ZONE_IMAGE_REVERSE_HORIZONTALLY 103
#define MEDIA_VIEWER_CLICK_ZONE_IMAGE_REVERSE_VERTICALLY 104
#define MEDIA_VIEWER_CLICK_ZONE_IMAGE_TURN_LEFT 105
#define MEDIA_VIEWER_CLICK_ZONE_IMAGE_TURN_RIGHT 106

#define MEDIA_VIEWER_CLICK_ZONE_SOUND_PROGRESS 200
#define MEDIA_VIEWER_CLICK_ZONE_SOUND_VOLUME_MINUS_10 201
#define MEDIA_VIEWER_CLICK_ZONE_SOUND_VOLUME_PLUS_10 202
#define MEDIA_VIEWER_CLICK_ZONE_SOUND_PLAY_STOP 203

dword_t media_viewer_program_interface_memory = 0, media_viewer_sound_state = 0, media_viewer_showed_square_length_of_skipped_data = 0;

void initalize_media_viewer(void);
void media_viewer(void);
void draw_media_viewer(void);
void media_viewer_draw_image(void);
void media_viewer_change_between_files(void);
void media_viewer_open_file(void);
void media_viewer_save_file(void);
void media_viewer_close_file(void);
void media_viewer_image_vertical_scrollbar_event(void);
void media_viewer_image_horizontal_scrollbar_event(void);
void media_viewer_image_reverse_horizontally(void);
void media_viewer_image_reverse_vertically(void);
void media_viewer_image_turn_left(void);
void media_viewer_image_turn_right(void);
void media_viewer_key_up_event(void);
void media_viewer_key_down_event(void);
void media_viewer_key_left_event(void);
void media_viewer_key_right_event(void);
void media_viewer_key_space_event(void);
void media_viewer_image_recalculate_scrollbars(void);
void media_viewer_image_recalculate_zoom(void);
void media_viewer_pause_sound(void);
void media_viewer_click_on_sound_progress_square(void);