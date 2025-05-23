//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void realtek_8169_add_new_pci_device(struct pci_device_info_t device) {
    // check number of already connected cards
    if(components->n_realtek_8169 >= MAX_NUMBER_OF_REALTEK_8169_CARDS) {
        return;
    }

    // log driver
    logf("%s", __FILE__);

    // save basic device informations
    copy_memory((dword_t)&device, (dword_t)&components->realtek_8169[components->n_realtek_8169].pci, sizeof(struct pci_device_info_t));

    // read other device informations and configure PCI
    components->realtek_8169[components->n_realtek_8169].bar_type = pci_get_bar_type(device, PCI_BAR0);
    if(components->realtek_8169[components->n_realtek_8169].bar_type == PCI_IO_BAR) {
        components->realtek_8169[components->n_realtek_8169].base = pci_get_io(device, PCI_BAR0);
        pci_set_bits(device, 0x04, PCI_STATUS_BUSMASTERING | PCI_STATUS_IO);
    }
    else {
        components->realtek_8169[components->n_realtek_8169].base = pci_get_mmio(device, PCI_BAR0);
        pci_set_bits(device, 0x04, PCI_STATUS_BUSMASTERING | PCI_STATUS_MMIO);
    }

    // update number of devices
    components->n_realtek_8169++;
}

void ec_realtek_8169_outb(dword_t number_of_card, dword_t port, byte_t value) {
    if(components->realtek_8169[number_of_card].bar_type==PCI_IO_BAR) {
        outb(components->realtek_8169[number_of_card].base+port, value);
    }
    else if(components->realtek_8169[number_of_card].bar_type==PCI_MMIO_BAR) {
        mmio_outb(components->realtek_8169[number_of_card].base+port, value);
    }
}

void ec_realtek_8169_outw(dword_t number_of_card, dword_t port, word_t value) {
    if(components->realtek_8169[number_of_card].bar_type==PCI_IO_BAR) {
        outw(components->realtek_8169[number_of_card].base+port, value);
    }
    else if(components->realtek_8169[number_of_card].bar_type==PCI_MMIO_BAR) {
        mmio_outw(components->realtek_8169[number_of_card].base+port, value);
    }
}

void ec_realtek_8169_outd(dword_t number_of_card, dword_t port, dword_t value) {
    if(components->realtek_8169[number_of_card].bar_type==PCI_IO_BAR) {
        outd(components->realtek_8169[number_of_card].base+port, value);
    }
    else if(components->realtek_8169[number_of_card].bar_type==PCI_MMIO_BAR) {
        mmio_outd(components->realtek_8169[number_of_card].base+port, value);
    }
}

byte_t ec_realtek_8169_inb(dword_t number_of_card, dword_t port) {
    if(components->realtek_8169[number_of_card].bar_type==PCI_IO_BAR) {
        return inb(components->realtek_8169[number_of_card].base+port);
    }
    else if(components->realtek_8169[number_of_card].bar_type==PCI_MMIO_BAR) {
        return mmio_inb(components->realtek_8169[number_of_card].base+port);
    }

    return STATUS_ERROR;
}

word_t ec_realtek_8169_inw(dword_t number_of_card, dword_t port) {
    if(components->realtek_8169[number_of_card].bar_type==PCI_IO_BAR) {
        return inw(components->realtek_8169[number_of_card].base+port);
    }
    else if(components->realtek_8169[number_of_card].bar_type==PCI_MMIO_BAR) {
        return mmio_inw(components->realtek_8169[number_of_card].base+port);
    }

    return STATUS_ERROR;
}

dword_t ec_realtek_8169_ind(dword_t number_of_card, dword_t port) {
    if(components->realtek_8169[number_of_card].bar_type==PCI_IO_BAR) {
        return ind(components->realtek_8169[number_of_card].base+port);
    }
    else if(components->realtek_8169[number_of_card].bar_type==PCI_MMIO_BAR) {
        return mmio_ind(components->realtek_8169[number_of_card].base+port);
    }

    return STATUS_ERROR;
}

void ec_realtek_8169_initalize(dword_t number_of_card) {
    // log
    logf("\n\nDriver: Realtek 8169 Ethernet\nDevice: PCI bus %d:%d:%d:%d",
        components->realtek_8169[number_of_card].pci.segment,
        components->realtek_8169[number_of_card].pci.bus,
        components->realtek_8169[number_of_card].pci.device,
        components->realtek_8169[number_of_card].pci.function);

    //enable PCI multiple read/write
    ec_realtek_8169_outw(number_of_card, 0xE0, (1<<3));

    //reset card
    ec_realtek_8169_outb(number_of_card, 0x37, (1 << 4));
    volatile dword_t timeout = (time_of_system_running+50);
    while((ec_realtek_8169_inb(number_of_card, 0x37) & (1 << 4)) == (1 << 4)) {
        asm("nop");
        if(time_of_system_running >= timeout) {
            logf("\nERROR: Reset was not successful");
            return;
        }
    }

    //read MAC address
    for(dword_t i = 0; i < 6; i++) {
        components->realtek_8169[number_of_card].mac_address[i] = ec_realtek_8169_inb(number_of_card, i);
    }

    //log MAC address
    logf("\nMAC: ");
    for(dword_t i = 0; i < 6; i++) {
        logf("0x%02x ", components->realtek_8169[number_of_card].mac_address[i]);
    }

    // add card to list
    ethernet_cards[number_of_ethernet_cards].card_number = number_of_card;
    for(dword_t i = 0; i < 6; i++) {
        ethernet_cards[number_of_ethernet_cards].mac_address[i] = components->realtek_8169[number_of_card].mac_address[i];
    }
    ethernet_cards[number_of_ethernet_cards].get_cable_status = ec_realtek_8169_get_cable_status;
    ethernet_cards[number_of_ethernet_cards].send_packet = ec_realtek_8169_send_packet;
    ethernet_cards[number_of_ethernet_cards].process_irq = ec_realtek_8169_process_irq;
    number_of_ethernet_cards++;

    //unlock registers
    ec_realtek_8169_outb(number_of_card, 0x50, 0xC0);

    //enable interrupts
    pci_device_install_interrupt_handler(components->realtek_8169[number_of_card].pci, network_irq);
    ec_realtek_8169_outw(number_of_card, 0x3C, (1 << 0) | (1 << 4) | (1 << 5)); //packet received, rx buffer is full, link status change

    //allocate memory of receive descriptors
    struct ec_realtek_8169_rx_descriptor_t *rx_descriptors = (struct ec_realtek_8169_rx_descriptor_t *) aligned_calloc(sizeof(struct ec_realtek_8169_rx_descriptor_t)*EC_REALTEK_8169_NUMBER_OF_RX_DESCRIPTORS, 0xFF);
    byte_t *rx_packets_memory = (byte_t *) malloc(MAX_SIZE_OF_ONE_PACKET*EC_REALTEK_8169_NUMBER_OF_RX_DESCRIPTORS);

    //set variables of receive descriptors
    components->realtek_8169[number_of_card].rx_descriptors_memory = (void *) rx_descriptors;
    components->realtek_8169[number_of_card].rx_packets_memory = rx_packets_memory;
    components->realtek_8169[number_of_card].rx_descriptor_pointer = 0;

    //initalize receive descriptors
    for(dword_t i = 0; i < EC_REALTEK_8169_NUMBER_OF_RX_DESCRIPTORS; i++) {
        //set memory of packet
        rx_descriptors[i].lower_memory = (dword_t) rx_packets_memory;
        rx_packets_memory += MAX_SIZE_OF_ONE_PACKET;

        //set max length of one packet
        rx_descriptors[i].packet_size = MAX_SIZE_OF_ONE_PACKET;

        //card owns this descriptor
        rx_descriptors[i].owned_by_card = 1;

        //all other fields are zero
    }
    rx_descriptors[EC_REALTEK_8169_NUMBER_OF_RX_DESCRIPTORS-1].end_of_ring = 1; //set this bit at last descriptor

    //initialize receiving mechanism
    ec_realtek_8169_outd(number_of_card, 0x44, (0x1F | (0x7 << 8) | (0x7 << 13))); //accept all packets, max DMS burst, no rx treshold
    ec_realtek_8169_outw(number_of_card, 0xDA, 0x1FFF); //max packet size
    ec_realtek_8169_outd(number_of_card, 0xE4, (dword_t)components->realtek_8169[number_of_card].rx_descriptors_memory); //lower address of rx descriptors
    ec_realtek_8169_outd(number_of_card, 0xE8, 0); //upper address of rx descriptors

    //allocate memory of transmit descriptors
    struct ec_realtek_8169_tx_descriptor_t *tx_descriptors = (struct ec_realtek_8169_tx_descriptor_t *) aligned_calloc(sizeof(struct ec_realtek_8169_tx_descriptor_t)*EC_REALTEK_8169_NUMBER_OF_TX_DESCRIPTORS, 0xFF);
    byte_t *tx_packets_memory = (byte_t *) malloc(MAX_SIZE_OF_ONE_PACKET*EC_REALTEK_8169_NUMBER_OF_TX_DESCRIPTORS);

    //set variables of transmit descriptors
    components->realtek_8169[number_of_card].tx_descriptors_memory = (void *) tx_descriptors;
    components->realtek_8169[number_of_card].tx_packets_memory = tx_packets_memory;
    components->realtek_8169[number_of_card].tx_descriptor_pointer = 0;

    //initalize transmit descriptors
    for(dword_t i = 0; i < EC_REALTEK_8169_NUMBER_OF_TX_DESCRIPTORS; i++) {
        //set memory of packet
        tx_descriptors[i].lower_memory = (dword_t) tx_packets_memory;
        tx_packets_memory += MAX_SIZE_OF_ONE_PACKET;

        //all other fields are zero
    }
    tx_descriptors[EC_REALTEK_8169_NUMBER_OF_TX_DESCRIPTORS-1].end_of_ring = 1; //set this bit at last descriptor

    //initialize transmitting mechanism
    ec_realtek_8169_outb(number_of_card, 0x37, 0x04); //enable transmitting packets, this must be set before configuring transmitting
    ec_realtek_8169_outb(number_of_card, 0xEC, 0x3F); //no early transmit allowed
    ec_realtek_8169_outd(number_of_card, 0x40, (0x7 << 8) | (0x3 << 24)); //unlimited dma burst, interframe gap
    ec_realtek_8169_outd(number_of_card, 0x20, (dword_t)components->realtek_8169[number_of_card].tx_descriptors_memory); //lower address of normal priority tx descriptors
    ec_realtek_8169_outd(number_of_card, 0x24, 0); //upper address of normal priority tx descriptors
    ec_realtek_8169_outd(number_of_card, 0x28, 0); //lower address of high priority tx descriptors
    ec_realtek_8169_outd(number_of_card, 0x2C, 0); //upper address of high priority tx descriptors

    //enable transmitting and receiving packets
    ec_realtek_8169_outb(number_of_card, 0x37, 0x0C);

    //lock registers
    ec_realtek_8169_outb(number_of_card, 0x50, 0x00);
}

byte_t ec_realtek_8169_get_cable_status(dword_t number_of_card) {
    if((ec_realtek_8169_inb(number_of_card, 0x6C) & 0x2)==0x2) {
        return STATUS_TRUE;
    }
    else {
        return STATUS_FALSE;
    }
}

byte_t ec_realtek_8169_send_packet(dword_t number_of_card, byte_t *packet_memory, dword_t packet_size) {
    //check if descriptors are not full
    struct ec_realtek_8169_tx_descriptor_t *tx_descriptor = (struct ec_realtek_8169_tx_descriptor_t *) (((dword_t)components->realtek_8169[number_of_card].tx_descriptors_memory)+(sizeof(struct ec_realtek_8169_tx_descriptor_t)*components->realtek_8169[number_of_card].tx_descriptor_pointer));
    if(tx_descriptor->owned_by_card == 1) {
        logf("\nERROR: Ethernet tx buffer is full");
        return STATUS_ERROR;
    }

    //copy packet to transfer packet memory
    copy_memory((dword_t)packet_memory, (dword_t)&components->realtek_8169[number_of_card].tx_packets_memory[MAX_SIZE_OF_ONE_PACKET*components->realtek_8169[number_of_card].tx_descriptor_pointer], packet_size);
    
    //card automatically add padding to too small packets

    //fill tx descriptor
    tx_descriptor->packet_size = packet_size;
    tx_descriptor->first_segment_descriptor = 1;
    tx_descriptor->last_segment_descriptor = 1;
    tx_descriptor->owned_by_card = 1;

    //tell card to send packet from normal priority descriptors
    ec_realtek_8169_outb(number_of_card, 0x38, (1 << 6));

    //TODO: why is packet sent, but bit "TX descriptor unavailable" is also always set in interrupt?
    
    //move free descriptor number
    components->realtek_8169[number_of_card].tx_descriptor_pointer++;
    if(components->realtek_8169[number_of_card].tx_descriptor_pointer >= EC_REALTEK_8169_NUMBER_OF_TX_DESCRIPTORS) {
        components->realtek_8169[number_of_card].tx_descriptor_pointer = 0;
    }

    return STATUS_GOOD;
}

void ec_realtek_8169_process_irq(dword_t number_of_card) {
    //read irq status to know what caused interrupt
    word_t irq_status = ec_realtek_8169_inw(number_of_card, 0x3E);
    if(irq_status == 0x00000000) {
        return;
    }

    //packet received
    if((irq_status & 0x11)!=0x00) {
        //for cycle will prevent infinite cycle if there is some error
        for(dword_t i=0; i<2048; i++) {
            //set pointer to rx descriptor
            struct ec_realtek_8169_rx_descriptor_t *rx_descriptor = (struct ec_realtek_8169_rx_descriptor_t *) (((dword_t)components->realtek_8169[number_of_card].rx_descriptors_memory)+(sizeof(struct ec_realtek_8169_rx_descriptor_t)*components->realtek_8169[number_of_card].rx_descriptor_pointer));
            
            //check if we processed everything
            if(rx_descriptor->owned_by_card == 1) {
                break;
            }

            //process packet
            process_packet((byte_t *)rx_descriptor->lower_memory, rx_descriptor->packet_size);

            //reset descriptor
            rx_descriptor->packet_size = MAX_SIZE_OF_ONE_PACKET;
            rx_descriptor->vlan = 0;
            rx_descriptor->owned_by_card = 1;

            //move to next descriptor
            components->realtek_8169[number_of_card].rx_descriptor_pointer++;
            if(components->realtek_8169[number_of_card].rx_descriptor_pointer >= EC_REALTEK_8169_NUMBER_OF_RX_DESCRIPTORS) {
                components->realtek_8169[number_of_card].rx_descriptor_pointer = 0;
            }
        }
    }

    //link status change
    if((irq_status & 0x20)==0x20) {
        ethernet_cards_update_cable_status();
    }
    
    //acknowledge interrupt
    ec_realtek_8169_outw(number_of_card, 0x3E, irq_status);
}