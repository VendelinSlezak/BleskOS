# sound.c

From perspective of higher layers of code, you need to know only about these four methods:

```
process_audio_file()
play_audio_file()
audio_calculate_time_of_sound_data_offset()
destroy_audio_file()
```

When you load audio file into memory, you invoke process_audio_file() that prepare and return struct audio_file_t that contains all useful info, as well as all informations that are needed for playing that file. Currently there is support for MP3, WAV and CDDA files.

```
byte_t *pointer_to_mp3_file_in_memory = (byte_t *) (0xRANDOMPOSITION);
dword_t length_of_mp3_file_in_memory = 0xRANDOMLENGTH;

struct audio_file_t *mp3_audio_file_info = process_audio_file(AUDIO_FILE_TYPE_MP3, pointer_to_mp3_file_in_memory, length_of_mp3_file_in_memory);
```

audio_file_t->output_length contains full length of PCM data in this file.
audio_file_t->length_hours/minutes/seconds contains time of file.

Now you can play this file by method play_audio_file(audio_file_t, offset_to_PCM_data). Offset to PCM data have to be calculated from audio_file_t->output_length what equals end of file. You can use any value, method will round it to valid PCM frame.

```
play_audio_file(mp3_audio_file_info, 0); //play file from start
play_audio_file(mp3_audio_file_info, mp3_audio_file_info->output_length/2); //play file from half
```

File will be played from desired position to end. You can stop playing of file with method stop_sound() from source/drivers/sound/main.c.

If you want to change offset to PCM data manually to have calculated hours/minutes/seconds for that position, use method audio_calculate_time_of_sound_data_offset(audio_file_t, offset_to_PCM_data).

```
audio_calculate_time_of_sound_data_offset(mp3_audio_file_info, mp3_audio_file_info->output_length/2);

//now you have time for half of this audio file in mp3_audio_file_info->played_length_hours/minutes/seconds
```

When you do not need audio file, you can free all allocated memory with destroy_audio_file(audio_file_t).

```
destroy_audio_file(mp3_audio_file_info);
```