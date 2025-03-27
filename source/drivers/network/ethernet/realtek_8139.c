//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void realtek_8139_add_new_pci_device(struct pci_device_info_t device) {
    // check number of already connected cards
    if(components->n_realtek_8139 >= MAX_NUMBER_OF_REALTEK_8139_CARDS) {
        return;
    }

    // log driver
    logf("%s", __FILE__);

    // save basic device informations
    copy_memory((dword_t)&device, (dword_t)&components->realtek_8139[components->n_realtek_8139].pci, sizeof(struct pci_device_info_t));

    // read other device informations
    components->realtek_8139[components->n_realtek_8139].base = pci_get_io(device, PCI_BAR0);

    // configure PCI
    pci_set_bits(device, 0x04, PCI_STATUS_BUSMASTERING | PCI_STATUS_IO);

    // update number of devices
    components->n_realtek_8139++;
}

void ec_realtek_8139_initalize(dword_t number_of_card) {
    // log
    logf("\n\nDriver: Realtek 8139 Ethernet\nDevice: PCI bus %d:%d:%d:%d",
        components->realtek_8139[number_of_card].pci.segment,
        components->realtek_8139[number_of_card].pci.bus,
        components->realtek_8139[number_of_card].pci.device,
        components->realtek_8139[number_of_card].pci.function);
    logf("\nHardware Version ID: 0x%02x", (ind(components->realtek_8139[number_of_card].base+0x40) >> 26));

    //turn card on
    outb(components->realtek_8139[number_of_card].base+0x52, 0x00);
    
    //reset card
    outb(components->realtek_8139[number_of_card].base+0x37, 0x10);
    volatile dword_t timeout = (time_of_system_running+50);
    while((inb(components->realtek_8139[number_of_card].base+0x37) & (1 << 4))==(1 << 4)) {
        asm("nop");
        if(time_of_system_running >= timeout) {
            logf("\nERROR: Reset was not successful");
            return;
        }
    }

    //read MAC address
    for(dword_t i = 0; i < 6; i++) {
        components->realtek_8139[number_of_card].mac_address[i] = inb(components->realtek_8139[number_of_card].base+i);
    }

    //log MAC address
    logf("\nMAC: ");
    for(dword_t i = 0; i < 6; i++) {
        logf("0x%02x ",components->realtek_8139[number_of_card].mac_address[i]);
    }

    // add card to list
    ethernet_cards[number_of_ethernet_cards].card_number = number_of_card;
    for(dword_t i = 0; i < 6; i++) {
        ethernet_cards[number_of_ethernet_cards].mac_address[i] = components->realtek_8139[number_of_card].mac_address[i];
    }
    ethernet_cards[number_of_ethernet_cards].get_cable_status = ec_realtek_8139_get_cable_status;
    ethernet_cards[number_of_ethernet_cards].send_packet = ec_realtek_8139_send_packet;
    ethernet_cards[number_of_ethernet_cards].process_irq = ec_realtek_8139_process_irq;
    number_of_ethernet_cards++;

    //enable receiving and transmitting - we *must* enable them before configuring them, otherwise some cards will not work correctly
    outb(components->realtek_8139[number_of_card].base+0x37, 0x0C);
    
    //initalize receiving mechanism
    components->realtek_8139[number_of_card].rx_packets_memory = (byte_t *) aligned_calloc(64*1024 + 16, 0xFF);
    components->realtek_8139[number_of_card].rx_descriptor_pointer = 0;
    outd(components->realtek_8139[number_of_card].base+0x30, (dword_t)components->realtek_8139[number_of_card].rx_packets_memory);
    outd(components->realtek_8139[number_of_card].base+0x44, (0x1F) | (0b11 << 11) | (0x7 << 13)); //all packets accepted, no rx treshold to immediately process received packet
    
    //initalize transmitting mechanism
    components->realtek_8139[number_of_card].tx_descriptors_memory = (void *) calloc(sizeof(struct ec_realtek_8139_bleskos_tx_descriptor_t)*EC_REALTEK_8139_NUMBER_OF_TX_DESCRIPTORS); //card do not use descriptors, this is for driver
    components->realtek_8139[number_of_card].tx_descriptor_pointer = 0;
    components->realtek_8139[number_of_card].tx_packets_memory = (byte_t *) malloc(MAX_SIZE_OF_ONE_PACKET*256);
    outd(components->realtek_8139[number_of_card].base+0x40, (0x7 << 8) | (0x3 << 24)); //max packet size is 2048 bytes, interframe gap time (0x3 is only allowed value)

    //enable interrupts
    pci_device_install_interrupt_handler(components->amd_pcnet[number_of_card].pci, network_irq);
    outw(components->realtek_8139[number_of_card].base+0x3C, 0xFFFF); //enable all interrupts
}

byte_t ec_realtek_8139_get_cable_status(dword_t number_of_card) {
    if((inw(components->realtek_8139[number_of_card].base+0x64) & 0x4)==0x4) {
        return STATUS_TRUE;
    }
    else {
        return STATUS_FALSE;
    }
}

void ec_realtek_8139_set_transmit_port(dword_t number_of_card, byte_t transmit_port_number, dword_t packet_memory, dword_t packet_size) {
    //calculate base port
    dword_t base_transmit_port = (components->realtek_8139[number_of_card].base+0x10+(transmit_port_number*0x04));

    //set packet memory
    outd(base_transmit_port+0x10, packet_memory);

    //send packet with no early transmit
    outd(base_transmit_port, (packet_size) | (0x3F << 16));
}

byte_t ec_realtek_8139_send_packet(dword_t number_of_card, byte_t *packet_memory, dword_t packet_size) {
    // Realted 8139 has only 4 transmit ports, that cyclically transfer packets. This driver allocates ring that has EC_REALTEK_8139_NUMBER_OF_TX_DESCRIPTORS
    // descriptors, from which are packets transferred one by one through card transmit ports. tx_descriptors_memory contains pointer to start of this ring and
    // tx_descriptor_pointer points to descriptor that should be transferred to card. This uses fact, that after every transfer from trasmit port, IRQ is fired,
    // so descriptor that is pointed to by tx_descriptor_pointer is marked as transferred and tx_descriptor_pointer is moved to next descriptor. If it contain
    // untransferred packet, IRQ routine will immediately send it to next transmit port.

    // send_to_card == 1 mean that this descriptor from ring is waiting to be send to card

    //skip all descriptors that are waiting for transfer and find free descriptor
    struct ec_realtek_8139_bleskos_tx_descriptor_t *tx_descriptors = (struct ec_realtek_8139_bleskos_tx_descriptor_t *) components->realtek_8139[number_of_card].tx_descriptors_memory;
    dword_t actual_descriptor = components->realtek_8139[number_of_card].tx_descriptor_pointer;
    for(dword_t i=0; i<EC_REALTEK_8139_NUMBER_OF_TX_DESCRIPTORS; i++) {
        //this descriptor is free
        if(tx_descriptors[actual_descriptor].send_to_card == 0) {
            break;
        }
        else {
            //move to next descriptor
            actual_descriptor++;
            if(actual_descriptor == EC_REALTEK_8139_NUMBER_OF_TX_DESCRIPTORS) {
                actual_descriptor = 0;
            }
        }
    }

    //check if descriptors are not full
    if(tx_descriptors[actual_descriptor].send_to_card == 1) { //we check all descriptors, so we are back and first descriptor and it is still not free
        logf("\nERROR: Ethernet tx buffer is full");
        return STATUS_ERROR;
    }

    //copy packet to transmit buffer
    copy_memory((dword_t)packet_memory, (dword_t)((dword_t)components->realtek_8139[number_of_card].tx_packets_memory+(actual_descriptor*MAX_SIZE_OF_ONE_PACKET)), packet_size);

    //add padding to too small packets
    if(packet_size < 60) {
        clear_memory((dword_t)((dword_t)components->realtek_8139[number_of_card].tx_packets_memory+(actual_descriptor*MAX_SIZE_OF_ONE_PACKET))+packet_size, 60-packet_size);
        packet_size = 60;
    }

    //update descriptor
    tx_descriptors[actual_descriptor].packet_size = packet_size;
    tx_descriptors[actual_descriptor].send_to_card = 1;

    //if all descriptors were already sended, we need to initiate transfer here
    if(actual_descriptor == components->realtek_8139[number_of_card].tx_descriptor_pointer) { //tx_descriptor_pointer points to descriptor that should be transferred to card, so if it points to our descriptor, we need to send it to transmit port
        ec_realtek_8139_set_transmit_port(number_of_card,
                                            (components->realtek_8139[number_of_card].tx_descriptor_pointer % 4),
                                            (dword_t)((dword_t)components->realtek_8139[number_of_card].tx_packets_memory+(components->realtek_8139[number_of_card].tx_descriptor_pointer*MAX_SIZE_OF_ONE_PACKET)),
                                            packet_size);
    }

    return STATUS_GOOD;
}

void ec_realtek_8139_process_irq(dword_t number_of_card) {
    //read irq status to know what caused interrupt
    word_t irq_status = inw(components->realtek_8139[number_of_card].base+0x3E);
    if(irq_status == 0x00000000) {
        return;
    }

    //acknowledge interrupt - we *must* do it here otherwise we can not read packets from buffer
    outw(components->realtek_8139[number_of_card].base+0x3E, irq_status);

    //packet received
    if((irq_status & 0x1) == 0x1) {
        //for cycle will prevent infinite cycle if there is some error
        for(dword_t i = 0; i < 10; i++) {
            //check if we processed everything
            if((inb(components->realtek_8139[number_of_card].base+0x37) & 0x1)==0x1) {
                break;
            }

            //process packet
            struct ec_realtek_8139_received_packet_header_t *packet_header = (struct ec_realtek_8139_received_packet_header_t *) ((dword_t)components->realtek_8139[number_of_card].rx_packets_memory + components->realtek_8139[number_of_card].rx_descriptor_pointer);
            if(packet_header->packet_ok == 1 && packet_header->packet_size < MAX_SIZE_OF_ONE_PACKET) {
                //copy packet from cyclic buffer
                byte_t packet[MAX_SIZE_OF_ONE_PACKET];
                if((components->realtek_8139[number_of_card].rx_descriptor_pointer+4+packet_header->packet_size) > 64*1024) { //packet is splitted
                    word_t size_of_first_half_of_packet = (64*1024-4-components->realtek_8139[number_of_card].rx_descriptor_pointer);
                    copy_memory((dword_t)packet_header + 4, (dword_t)&packet, size_of_first_half_of_packet);
                    copy_memory((dword_t)components->realtek_8139[number_of_card].rx_packets_memory, ((dword_t)&packet)+size_of_first_half_of_packet, packet_header->packet_size-size_of_first_half_of_packet);
                }
                else { //packet is not splitted
                    copy_memory((dword_t)packet_header + 4, (dword_t)&packet, packet_header->packet_size);
                }

                //process packet
                process_packet(packet, packet_header->packet_size);
            }

            //move pointer through header, packet data and CRC at end
            components->realtek_8139[number_of_card].rx_descriptor_pointer = ((components->realtek_8139[number_of_card].rx_descriptor_pointer+sizeof(struct ec_realtek_8139_received_packet_header_t)+packet_header->packet_size+3) & 0xFFFFFFFC);
            if(components->realtek_8139[number_of_card].rx_descriptor_pointer >= 64*1024) {
                components->realtek_8139[number_of_card].rx_descriptor_pointer -= 64*1024;
            }

            //tell card that we processed this packet
            outw(components->realtek_8139[number_of_card].base+0x38, components->realtek_8139[number_of_card].rx_descriptor_pointer-16);
        }
    }

    //packet was transmitted
    if((irq_status & 0x0C) != 0x00) {
        //update descriptor
        struct ec_realtek_8139_bleskos_tx_descriptor_t *tx_descriptor = (struct ec_realtek_8139_bleskos_tx_descriptor_t *) (((dword_t)components->realtek_8139[number_of_card].tx_descriptors_memory)+(sizeof(struct ec_realtek_8139_bleskos_tx_descriptor_t)*components->realtek_8139[number_of_card].tx_descriptor_pointer));
        tx_descriptor->send_to_card = 0;

        //move to next descriptor
        components->realtek_8139[number_of_card].tx_descriptor_pointer++;
        if(components->realtek_8139[number_of_card].tx_descriptor_pointer >= EC_REALTEK_8139_NUMBER_OF_TX_DESCRIPTORS) {
            components->realtek_8139[number_of_card].tx_descriptor_pointer = 0;
        }

        //check if we do not need to send this descriptor
        tx_descriptor = (struct ec_realtek_8139_bleskos_tx_descriptor_t *) (((dword_t)components->realtek_8139[number_of_card].tx_descriptors_memory)+(sizeof(struct ec_realtek_8139_bleskos_tx_descriptor_t)*components->realtek_8139[number_of_card].tx_descriptor_pointer));
        if(tx_descriptor->send_to_card == 1) {
            ec_realtek_8139_set_transmit_port(number_of_card,
                                                (components->realtek_8139[number_of_card].tx_descriptor_pointer % 4),
                                                (dword_t)((dword_t)components->realtek_8139[number_of_card].tx_packets_memory+(components->realtek_8139[number_of_card].tx_descriptor_pointer*MAX_SIZE_OF_ONE_PACKET)),
                                                tx_descriptor->packet_size);
        }
    }
    
    //link status change
    if((irq_status & 0x20)==0x20) {
        ethernet_cards_update_cable_status();
    }
}