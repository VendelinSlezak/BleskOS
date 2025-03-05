//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void initalize_mp3_decoder(void) {
 mp3_decoder_working_area = (mp3dec_t *) (malloc(sizeof(mp3dec_t)));
 mp3_frame_info = (mp3dec_frame_info_t *) (malloc(sizeof(mp3dec_frame_info_t)));
 mp3_frame_pcm_output = (word_t *) (malloc(MINIMP3_MAX_SAMPLES_PER_FRAME*2)); //mp3 file can have max two channels in frame
}

dword_t convert_mp3_to_sound_data(dword_t mp3_memory, dword_t mp3_length) {
 byte_t *mp3 = (byte_t *) (mp3_memory);
 
 //decode first frame to found out if file is not corrupted
 dword_t samples = mp3dec_decode_frame(mp3_decoder_working_area, mp3, mp3_length, mp3_frame_pcm_output, mp3_frame_info);
 if(samples==0) { //not mp3 file / corrupted data
  logf("\nMP3: corrupted file");
  return STATUS_ERROR;
 }

 //decode full file
 struct byte_stream_descriptor_t *mp3_pcm_output = create_byte_stream(BYTE_STREAM_10_MB_BLOCK);
 while(1) {
  //add decoded PCM data to stream
  for(dword_t i=0; i<(samples*mp3_frame_info->channels); i++) {
   add_byte_to_byte_stream(mp3_pcm_output, (mp3_frame_pcm_output[i] & 0xFF));
   add_byte_to_byte_stream(mp3_pcm_output, (mp3_frame_pcm_output[i] >> 8));
  }

  //skip frame
  mp3 = (byte_t *) ((dword_t)mp3+mp3_frame_info->frame_bytes);
  if(mp3_frame_info->frame_bytes>=mp3_length) {
   break;
  }
  else {
   mp3_length -= mp3_frame_info->frame_bytes;
  }

  //decode next frame
  samples = mp3dec_decode_frame(mp3_decoder_working_area, mp3, mp3_length, mp3_frame_pcm_output, mp3_frame_info);

  //test if we are not at end
  if(samples==0) {
   break;
  }
 }

 //create sound structure
 dword_t sound_memory = create_sound(mp3_frame_info->channels, 16, mp3_frame_info->hz, mp3_pcm_output->size_of_stream);

 //copy PCM data
 copy_memory(mp3_pcm_output->start_of_allocated_memory, get_sound_data_memory(sound_memory), mp3_pcm_output->size_of_stream);

 //close stream
 destroy_byte_stream(mp3_pcm_output);

 return sound_memory;
}

struct mp3_info_t *read_mp3_info(byte_t *mp3_memory, dword_t mp3_length) {
 //decode first frame to found out if file is not corrupted and to get sound parameters
 dword_t samples = mp3dec_decode_frame(mp3_decoder_working_area, (byte_t *)mp3_memory, mp3_length, mp3_frame_pcm_output, mp3_frame_info);
 if(samples==0) { //corrupted mp3 file
  logf("\nMP3: corrupted file");
  return STATUS_ERROR;
 }
 dword_t samples_in_one_frame = samples;

 //allocate mp3 info structure
 struct mp3_info_t *mp3_info = (struct mp3_info_t *) (calloc(sizeof(struct mp3_info_t)));
 mp3_info->mp3_file = mp3_memory;
 mp3_info->size_of_mp3_file = mp3_length;

 //save audio format
 mp3_info->pcm_data_number_of_channels = mp3_frame_info->channels;
 mp3_info->pcm_data_sample_rate = mp3_frame_info->hz;

 //recalculate what sample rate we will use
 mp3_info->output_pcm_data_sample_rate = 0;
 if(mp3_info->pcm_data_sample_rate==48000 || mp3_info->pcm_data_sample_rate==24000 || mp3_info->pcm_data_sample_rate==12000) {
  mp3_info->output_pcm_data_sample_rate = 48000;
 }
 else if(mp3_info->pcm_data_sample_rate==44100 || mp3_info->pcm_data_sample_rate==22050 || mp3_info->pcm_data_sample_rate==11025) {
  mp3_info->output_pcm_data_sample_rate = 44100;
 }

 //index mp3 file and read length of PCM data
 struct byte_stream_descriptor_t *mp3_index = create_byte_stream(BYTE_STREAM_100_KB_BLOCK);
 dword_t mp3_end_of_file = ((dword_t)mp3_memory+mp3_length);
 dword_t pcm_data_offset = 0;
 while((dword_t)mp3_memory<mp3_end_of_file) {
  //get mp3 frame info without decoding PCM data
  samples = mp3dec_decode_frame(mp3_decoder_working_area, (byte_t *)mp3_memory, mp3_length, 0, mp3_frame_info);
  if(samples==0) {
   break;
  }

  //save index of mp3 frame
  add_dword_to_byte_stream(mp3_index, (dword_t)mp3_memory);
  add_dword_to_byte_stream(mp3_index, pcm_data_offset);

  //move to next frame
  mp3_memory = (byte_t *) ((dword_t)mp3_memory+mp3_frame_info->frame_bytes);
  mp3_length -= mp3_frame_info->frame_bytes;
  pcm_data_offset += (samples*2*mp3_frame_info->channels);
 }

 //save index to mp3 info
 mp3_info->mp3_index = (struct mp3_index_t *) (close_byte_stream(mp3_index));

 //save length of PCM data
 mp3_info->length_of_pcm_data = pcm_data_offset;

 //calculate length of output PCM data
 mp3_info->length_of_output_pcm_data = ((mp3_info->length_of_pcm_data/mp3_info->pcm_data_number_of_channels)*2);
 mp3_info->length_of_output_pcm_data = (mp3_info->length_of_output_pcm_data*(mp3_info->output_pcm_data_sample_rate/mp3_info->pcm_data_sample_rate));

 //calculate buffer size
 if(samples_in_one_frame==1152) {
  mp3_info->number_of_frames_in_buffer = 40;
 }
 else {
  mp3_info->number_of_frames_in_buffer = 80;
 }
 mp3_info->output_pcm_data_buffer_size = samples_in_one_frame*2*2*mp3_info->number_of_frames_in_buffer;

 return mp3_info;
}

dword_t play_mp3(struct mp3_info_t *mp3_info, dword_t offset_to_first_byte_to_play) {
 //calculate mp3 PCM data offset
 dword_t offset_to_first_byte_to_pcm_data_in_original_mp3 = ((offset_to_first_byte_to_play/2/2)/(mp3_info->output_pcm_data_sample_rate/mp3_info->pcm_data_sample_rate)*(mp3_info->pcm_data_number_of_channels*2));

 //calculate by index to frame
 dword_t frame_number = (offset_to_first_byte_to_pcm_data_in_original_mp3/mp3_info->mp3_index[1].pcm_data_offset);
 actually_played_mp3_info->mp3_first_frame_pcm_data_offset = (offset_to_first_byte_to_pcm_data_in_original_mp3%mp3_info->mp3_index[1].pcm_data_offset);

 //save mp3 info pointer
 actually_played_mp3_info = mp3_info;

 //play mp3 file
 play_sound_with_refilling_buffer(mp3_info->mp3_index[frame_number].mp3_frame_pointer, ((dword_t)mp3_info->mp3_file+mp3_info->size_of_mp3_file)-(dword_t)mp3_info->mp3_index[frame_number].mp3_frame_pointer, mp3_info->length_of_output_pcm_data-offset_to_first_byte_to_play, mp3_info->output_pcm_data_sample_rate, mp3_info->output_pcm_data_buffer_size, mp3_refill_buffer);
}

void mp3_refill_buffer(byte_t *buffer) {
 //clear buffer memory
 clear_memory((dword_t)buffer, sound_buffer_refilling_info->buffer_size);

 //fill buffer with data
 for(dword_t i=0; i<actually_played_mp3_info->number_of_frames_in_buffer; i++) {
  //decode frame
  dword_t samples = mp3dec_decode_frame(mp3_decoder_working_area, sound_buffer_refilling_info->source_data_pointer, sound_buffer_refilling_info->source_data_length, mp3_frame_pcm_output, mp3_frame_info);
  
  if(samples!=0) {
   //copy PCM data
   if(actually_played_mp3_info->pcm_data_number_of_channels==2) {
    copy_memory((dword_t)mp3_frame_pcm_output, (dword_t)buffer+actually_played_mp3_info->mp3_first_frame_pcm_data_offset, samples*2*2-actually_played_mp3_info->mp3_first_frame_pcm_data_offset);
    buffer += samples*2*2-actually_played_mp3_info->mp3_first_frame_pcm_data_offset;
   }
   else { //1 channel
    word_t *buffer_word_pointer = (word_t *) (buffer+actually_played_mp3_info->mp3_first_frame_pcm_data_offset);

    for(dword_t buffer_pointer=0, mp3_frame_pcm_output_pointer=0; mp3_frame_pcm_output_pointer<(samples-(actually_played_mp3_info->mp3_first_frame_pcm_data_offset/4)); buffer_pointer+=2, mp3_frame_pcm_output_pointer++) {
     buffer_word_pointer[buffer_pointer] = mp3_frame_pcm_output[mp3_frame_pcm_output_pointer];
     buffer_word_pointer[buffer_pointer+1] = mp3_frame_pcm_output[mp3_frame_pcm_output_pointer];
     buffer += 4;
    }
   }
  }

  //this variable is relevant only for first frame
  actually_played_mp3_info->mp3_first_frame_pcm_data_offset = 0; 
  
  //move variables
  sound_buffer_refilling_info->source_data_pointer = (byte_t *) ((dword_t)sound_buffer_refilling_info->source_data_pointer+mp3_frame_info->frame_bytes);
  sound_buffer_refilling_info->source_data_length -= mp3_frame_info->frame_bytes;
 }
}