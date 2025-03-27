//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void ac97_add_new_pci_device(struct pci_device_info_t device) {
    // check number of already connected cards
    if(components->n_ac97 >= MAX_NUMBER_OF_AC97_SOUND_CARDS) {
        return;
    }

    // log driver
    logf("%s", __FILE__);

    // save basic device informations
    copy_memory((dword_t)&device, (dword_t)&components->ac97[components->n_ac97].pci, sizeof(struct pci_device_info_t));

    // read other device informations
    components->ac97[components->n_ac97].nam_base = pci_get_io(device, PCI_BAR0);
    components->ac97[components->n_ac97].nabm_base = pci_get_io(device, PCI_BAR1);

    // configure PCI
    pci_set_bits(device, 0x04, PCI_STATUS_BUSMASTERING | PCI_STATUS_IO);

    // update number of devices
    components->p_sound_card = 1;
    components->n_ac97++;
}

void initalize_ac97_sound_card(dword_t sound_card_number) {
    selected_ac97_card = sound_card_number;

    //log
    logf("\n\nDriver: AC97 Sound Card\nDevice: PCI bus %d:%d:%d:%d",
        components->ac97[selected_ac97_card].pci.segment,
        components->ac97[selected_ac97_card].pci.bus,
        components->ac97[selected_ac97_card].pci.device,
        components->ac97[selected_ac97_card].pci.function);
    
    //load base address of registers
    word_t ac97_nam_base = components->ac97[selected_ac97_card].nam_base;
    word_t ac97_nabm_base = components->ac97[selected_ac97_card].nabm_base;
    
    //resume from cold reset to normal operation and set 2 channels + 16 bit samples
    outd(ac97_nabm_base + AC97_NABM_IO_GLOBAL_CONTROL, (0b00<<22) | (0b00<<20) | (0<<2) | (1<<1));
    wait(20);
    
    //reset all streams
    outb(ac97_nabm_base + AC97_NABM_IO_PCM_INPUT_CONTROL, 0x2);
    outb(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_CONTROL, 0x2);
    outb(ac97_nabm_base + AC97_NABM_IO_MICROPHONE_INPUT_CONTROL, 0x2);
    wait(20);

    //reset all NAM registers
    outw(ac97_nam_base + AC97_NAM_IO_RESET, 0xFF);

    //detect if there is headphone output
    if((inw(ac97_nam_base + AC97_NAM_IO_CAPABILITES) & AC97_CAPABILITY_HEADPHONE_OUTPUT) == AC97_CAPABILITY_HEADPHONE_OUTPUT
        && inw(ac97_nam_base + AC97_NAM_IO_AUX_OUT_VOLUME) == 0x8000) {
        logf("\nHeadphone output: Present");
        components->ac97[selected_ac97_card].headphone_output_present = STATUS_TRUE;
    }

    //set max PCM output volume
    outw(ac97_nam_base + AC97_NAM_IO_PCM_OUT_VOLUME, 0x0);
    
    //allocate memory for buffer
    components->ac97[selected_ac97_card].buffer_memory_pointer = (struct ac97_buffer_entry_t *) (aligned_malloc(sizeof(struct ac97_buffer_entry_t)*32, 0xF));
    
    //read extended capabilities
    components->ac97[selected_ac97_card].extended_capabilities = inw(ac97_nam_base + AC97_NAM_IO_EXTENDED_CAPABILITIES);
    if((components->ac97[selected_ac97_card].extended_capabilities & AC97_EXTENDED_CAPABILITY_VARIABLE_SAMPLE_RATE) == AC97_EXTENDED_CAPABILITY_VARIABLE_SAMPLE_RATE) {
        outw(ac97_nam_base + AC97_NAM_IO_EXTENDED_FEATURES_CONTROL, AC97_EXTENDED_CAPABILITY_VARIABLE_SAMPLE_RATE); //enable variable sample rate
    }

    //get number of AUX_OUT volume steps
    components->ac97[selected_ac97_card].aux_out_number_of_volume_steps = 31;
    outw(ac97_nam_base + AC97_NAM_IO_AUX_OUT_VOLUME, 0x2020);
    if((inw(ac97_nam_base + AC97_NAM_IO_AUX_OUT_VOLUME) & 0x2020)==0x2020) {
        components->ac97[selected_ac97_card].aux_out_number_of_volume_steps = 63;
    }

    //set output 
    sound_volume = 0;
    if(ac97_is_headphone_connected()==STATUS_TRUE) {
        ac97_set_output(AC97_HEADPHONE_OUTPUT);
    }
    else {
        ac97_set_output(AC97_SPEAKER_OUTPUT);
    }

    //add task for checking headphone connection
    create_task(ac97_check_headphone_connection_change, TASK_TYPE_PERIODIC_INTERRUPT, 50);
    
    //log
    logf("\nCapabilities: 0x%04x", inw(ac97_nam_base + AC97_NAM_IO_CAPABILITES));
    logf("\nExtended capabilities: 0x%04x", components->ac97[selected_ac97_card].extended_capabilities);
}

byte_t ac97_is_headphone_connected(void) {
    //JACK_SENSE port is not part of official specification, but it seems to be supported by cards
    if(components->ac97[selected_ac97_card].headphone_output_present==STATUS_TRUE
        && (inw(components->ac97[selected_ac97_card].nam_base + AC97_NAM_IO_JACK_SENSE) & 0x8)==0x8) {
        return STATUS_TRUE;
    }
    else {
        return STATUS_FALSE;
    }
}

void ac97_set_volume_in_register(dword_t offset, byte_t number_of_volume_steps, byte_t volume) {
    if(volume == 0) {
        outw(components->ac97[selected_ac97_card].nam_base + offset, 0x8000); //mute
    }
    else {
        volume = ((100-volume)*number_of_volume_steps/100); //recalculate 0-100 scale to register volume scale
        outw(components->ac97[selected_ac97_card].nam_base + offset, ((volume) | (volume<<8))); //set same volume for left and right
    }
}

void ac97_set_output(byte_t output) {
    if(output == AC97_SPEAKER_OUTPUT) {
        ac97_set_volume_in_register(AC97_NAM_IO_AUX_OUT_VOLUME, components->ac97[selected_ac97_card].aux_out_number_of_volume_steps, 0);
        ac97_set_volume_in_register(AC97_NAM_IO_MASTER_VOLUME, AC97_SPEAKER_OUTPUT_NUMBER_OF_VOLUME_STEPS, sound_volume);
    }
    else if(output == AC97_HEADPHONE_OUTPUT) {
        ac97_set_volume_in_register(AC97_NAM_IO_MASTER_VOLUME, components->ac97[selected_ac97_card].aux_out_number_of_volume_steps, 0);
        ac97_set_volume_in_register(AC97_NAM_IO_AUX_OUT_VOLUME, AC97_SPEAKER_OUTPUT_NUMBER_OF_VOLUME_STEPS, sound_volume);
    }
    else {
        return;
    }

    components->ac97[selected_ac97_card].selected_output = output;
}

void ac97_set_volume(byte_t volume) {
    if(components->ac97[selected_ac97_card].selected_output == AC97_SPEAKER_OUTPUT) {
        ac97_set_volume_in_register(AC97_NAM_IO_MASTER_VOLUME, components->ac97[selected_ac97_card].aux_out_number_of_volume_steps, volume);
    }
    else if(components->ac97[selected_ac97_card].selected_output == AC97_HEADPHONE_OUTPUT) {
        ac97_set_volume_in_register(AC97_NAM_IO_AUX_OUT_VOLUME, AC97_SPEAKER_OUTPUT_NUMBER_OF_VOLUME_STEPS, volume);
    }
}

byte_t ac97_is_supported_sample_rate(word_t sample_rate) {
    // 48000 sample rate is always supported
    if(sample_rate == 48000) {
        return STATUS_GOOD;
    }
    // check if variable sound rate feature is present
    else if((components->ac97[selected_ac97_card].extended_capabilities & AC97_EXTENDED_CAPABILITY_VARIABLE_SAMPLE_RATE) == AC97_EXTENDED_CAPABILITY_VARIABLE_SAMPLE_RATE) {
        // 44100 sample rate is always supported by variable sound rate feature
        if(sample_rate == 44100) {
            return STATUS_GOOD;
        }
        // other may or may not be supported, so we will check it
        else {
            //TODO: does this method always work?
            outw(components->ac97[selected_ac97_card].nam_base + AC97_NAM_IO_VARIABLE_SAMPLE_RATE_FRONT_DAC, sample_rate);
            if(inw(components->ac97[selected_ac97_card].nam_base + AC97_NAM_IO_VARIABLE_SAMPLE_RATE_FRONT_DAC) == sample_rate) {
                return STATUS_GOOD;
            }
        }
    }
    // this sample rate is not supported
    else {
        return STATUS_ERROR; 
    }
}

void ac97_set_sample_rate(word_t sample_rate) {
    // check if variable sample rate feature is present
    if((components->ac97[selected_ac97_card].extended_capabilities & AC97_EXTENDED_CAPABILITY_VARIABLE_SAMPLE_RATE) == AC97_EXTENDED_CAPABILITY_VARIABLE_SAMPLE_RATE) {
        // set same variable rate on all outputs
        outw(components->ac97[selected_ac97_card].nam_base + AC97_NAM_IO_VARIABLE_SAMPLE_RATE_FRONT_DAC, sample_rate);
        outw(components->ac97[selected_ac97_card].nam_base + AC97_NAM_IO_VARIABLE_SAMPLE_RATE_SURR_DAC, sample_rate);
        outw(components->ac97[selected_ac97_card].nam_base + AC97_NAM_IO_VARIABLE_SAMPLE_RATE_LFE_DAC, sample_rate);
        outw(components->ac97[selected_ac97_card].nam_base + AC97_NAM_IO_VARIABLE_SAMPLE_RATE_LR_ADC, sample_rate);
    }
}

void ac97_check_headphone_connection_change(void) {
    if(components->ac97[selected_ac97_card].selected_output==AC97_SPEAKER_OUTPUT && ac97_is_headphone_connected()==STATUS_TRUE) { //headphone was connected
        ac97_set_output(AC97_HEADPHONE_OUTPUT);
    }
    else if(components->ac97[selected_ac97_card].selected_output==AC97_HEADPHONE_OUTPUT && ac97_is_headphone_connected()==STATUS_FALSE) { //headphone was disconnected
        ac97_set_output(AC97_SPEAKER_OUTPUT);
    }
}

void ac97_play_pcm_data_in_loop(dword_t sample_rate) {
    word_t ac97_nabm_base = components->ac97[selected_ac97_card].nabm_base;

    //stop sound
    outb(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_CONTROL, 0x0);

    //set sample rate
    ac97_set_sample_rate(sample_rate);

    //reset stream
    outb(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_CONTROL, 0x2);
    ticks = 0;
    while((inb(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_CONTROL) & 0x2)==0x2) {
        asm("nop");
        if(ticks>50) { //stream was not reseted after 100 ms
            return;
        }
    }
    outb(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_CONTROL, 0x0);

    //clear buffer
    clear_memory((dword_t)components->ac97[selected_ac97_card].buffer_memory_pointer, (sizeof(struct ac97_buffer_entry_t)*32));

    //set buffer address
    outd(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_BUFFER_BASE_ADDRESS, (dword_t)components->ac97[selected_ac97_card].buffer_memory_pointer);

    //fill buffer entries
    dword_t sound_memory = (dword_t) pcm_data;
    dword_t sound_length = (sound_buffer_refilling_info->buffer_size*2);
    for(dword_t i = 0; i < 32; i++) {
        if(sound_length > 0x2000*2) {
            components->ac97[selected_ac97_card].buffer_memory_pointer[i].sample_memory = sound_memory;
            components->ac97[selected_ac97_card].buffer_memory_pointer[i].number_of_samples = 0x2000;
            sound_memory += 0x2000*2;
            sound_length -= 0x2000*2;
        }
        else {
            components->ac97[selected_ac97_card].buffer_memory_pointer[i].sample_memory = sound_memory;
            components->ac97[selected_ac97_card].buffer_memory_pointer[i].number_of_samples = ((sound_length/2) & 0xFFFE);
            break;
        }
    }

    //clear status
    outw(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_STATUS, 0x1C);

    //start streaming
    outb(ac97_nabm_base + AC97_NABM_IO_PCM_OUTPUT_CONTROL, 0x1);

    //add task for playing buffer in loop
    create_task(task_ac97_play_buffer_in_loop, TASK_TYPE_PERIODIC_INTERRUPT, 1);
}

void task_ac97_play_buffer_in_loop(void) {
    //update Last Valid Entry register for all entries to be valid
    outb(components->ac97[selected_ac97_card].nabm_base + AC97_NABM_IO_PCM_OUTPUT_LAST_VALID_ENTRY, (inb(components->ac97[selected_ac97_card].nabm_base + AC97_NABM_IO_PCM_OUTPUT_CURRENTLY_PROCESSED_ENTRY)-1) & 0x1F);
}

dword_t ac97_get_actual_stream_position(void) {
    dword_t number_of_processed_bytes = 0;

    //add already played buffers
    for(dword_t i = 0; i < inb(components->ac97[selected_ac97_card].nabm_base + AC97_NABM_IO_PCM_OUTPUT_CURRENTLY_PROCESSED_ENTRY); i++) {
        number_of_processed_bytes += components->ac97[selected_ac97_card].buffer_memory_pointer[i].number_of_samples*2;
    }

    //add actual entry position
    number_of_processed_bytes += (components->ac97[selected_ac97_card].buffer_memory_pointer[inb(components->ac97[selected_ac97_card].nabm_base + AC97_NABM_IO_PCM_OUTPUT_CURRENTLY_PROCESSED_ENTRY)].number_of_samples*2 - inw(components->ac97[selected_ac97_card].nabm_base + AC97_NABM_IO_PCM_OUTPUT_CURRENT_ENTRY_POSITION)*2);

    return number_of_processed_bytes;
}

void ac97_stop_sound(void) {
    //stop streaming
    outb(components->ac97[selected_ac97_card].nabm_base + AC97_NABM_IO_PCM_OUTPUT_CONTROL, 0x0);

    //clear status
    outw(components->ac97[selected_ac97_card].nabm_base + AC97_NABM_IO_PCM_OUTPUT_STATUS, 0x1C);

    //destroy task for looping
    destroy_task(task_ac97_play_buffer_in_loop);
}