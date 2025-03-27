//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void intel_e1000_add_new_pci_device(struct pci_device_info_t device) {
    // check number of already connected cards
    if(components->n_intel_e1000 >= MAX_NUMBER_OF_INTEL_E1000_CARDS) {
        return;
    }

    // log driver
    logf("%s", __FILE__);

    // save basic device informations
    copy_memory((dword_t)&device, (dword_t)&components->intel_e1000[components->n_intel_e1000].pci, sizeof(struct pci_device_info_t));

    // read other device informations and configure PCI
    components->intel_e1000[components->n_intel_e1000].bar_type = pci_get_bar_type(device, PCI_BAR0);
    if(components->intel_e1000[components->n_intel_e1000].bar_type == PCI_IO_BAR) {
        components->intel_e1000[components->n_intel_e1000].base = pci_get_io(device, PCI_BAR0);
        pci_set_bits(device, 0x04, PCI_STATUS_BUSMASTERING | PCI_STATUS_IO);
    }
    else {
        components->intel_e1000[components->n_intel_e1000].base = pci_get_mmio(device, PCI_BAR0);
        pci_set_bits(device, 0x04, PCI_STATUS_BUSMASTERING | PCI_STATUS_MMIO);
    }

    // update number of devices
    components->n_intel_e1000++;
}

dword_t ec_intel_e1000_read(dword_t number_of_card, dword_t port) {
    if(components->intel_e1000[number_of_card].bar_type == PCI_MMIO_BAR) {
        return mmio_ind(components->intel_e1000[number_of_card].base+port);
    }
    else {
        // number of port
        outd(components->intel_e1000[number_of_card].base+0, port);

        // read value
        return ind(components->intel_e1000[number_of_card].base+4);
    }
}

void ec_intel_e1000_write(dword_t number_of_card, dword_t port, dword_t value) {
    if(components->intel_e1000[number_of_card].bar_type==PCI_MMIO_BAR) {
        mmio_outd(components->intel_e1000[number_of_card].base+port, value);
    }
    else {
        // number of port
        outd(components->intel_e1000[number_of_card].base+0, port);

        // send value
        outd(components->intel_e1000[number_of_card].base+4, value);
    }
}

byte_t ec_intel_e1000_read_timeout(dword_t number_of_card, dword_t port, dword_t bits, dword_t value, dword_t wait_in_milliseconds) {
    volatile dword_t timeout = (time_of_system_running+wait_in_milliseconds);

    while((ec_intel_e1000_read(number_of_card, port) & bits) != value) {
        asm("nop");

        if(time_of_system_running >= timeout) {
            return STATUS_ERROR;
        }
    }

    return STATUS_GOOD;
}

word_t ec_intel_e1000_read_eeprom(dword_t number_of_card, dword_t address) {
    // request EEPROM access
    ec_intel_e1000_write(number_of_card, EC_INTEL_E1000_PORT_EEPROM_CONTROL, ec_intel_e1000_read(number_of_card, EC_INTEL_E1000_PORT_EEPROM_CONTROL) | (1 << 6));

    // wait for access to be granted
    if(ec_intel_e1000_read_timeout(number_of_card, EC_INTEL_E1000_PORT_EEPROM_CONTROL, (1 << 7), (1 << 7), 10)==STATUS_ERROR) {
        return STATUS_ERROR;
    }

    // send request to read data from EEPROM
    ec_intel_e1000_write(number_of_card, EC_INTEL_E1000_PORT_EEPROM_READ, (0x1 | (address<<components->intel_e1000[number_of_card].eeprom_shift)));

    // wait for read to be completed and then read data
    dword_t status_bit = (1 << 1);
    if(components->intel_e1000[number_of_card].eeprom_shift == 8) {
        status_bit = (1 << 4);
    }
    word_t data = STATUS_ERROR;
    if(ec_intel_e1000_read_timeout(number_of_card, EC_INTEL_E1000_PORT_EEPROM_READ, status_bit, status_bit, 10)==STATUS_GOOD) {
        data = (ec_intel_e1000_read(number_of_card, EC_INTEL_E1000_PORT_EEPROM_READ) >> 16);
    }

    // disable EEPROM access
    ec_intel_e1000_write(number_of_card, EC_INTEL_E1000_PORT_EEPROM_CONTROL, ec_intel_e1000_read(number_of_card, EC_INTEL_E1000_PORT_EEPROM_CONTROL) & ~(1 << 6));
    
    // return readed EEPROM data
    return data;
}

void ec_intel_e1000_initalize(dword_t number_of_card) {
    // log
    logf("\n\nDriver: Intel E1000 Ethernet\nDevice: PCI bus %d:%d:%d:%d",
        components->intel_e1000[number_of_card].pci.segment,
        components->intel_e1000[number_of_card].pci.bus,
        components->intel_e1000[number_of_card].pci.device,
        components->intel_e1000[number_of_card].pci.function);

    // reset card
    ec_intel_e1000_write(number_of_card, EC_INTEL_E1000_PORT_DEVICE_CONTROL, (1 << 26));
    if(ec_intel_e1000_read_timeout(number_of_card, EC_INTEL_E1000_PORT_DEVICE_CONTROL, (1 << 26), 0x00000000, 100)==STATUS_ERROR) {
        logf("\nIntel E1000 ERROR: can not reset card");
        return;
    }

    // read MAC address from EEPROM or ports
    if((ec_intel_e1000_read(number_of_card, EC_INTEL_E1000_PORT_EEPROM_CONTROL) & 0x100)==0x100) { // check if card has EEPROM
        logf("\nEEPROM present with address shift ");

        // read EEPROM address 0x00
        ec_intel_e1000_read_eeprom(number_of_card, 0x00);

        // find which status bit is set, to know where we should place requests for EEPROM data
        components->intel_e1000[number_of_card].eeprom_shift = 2;
        if((ec_intel_e1000_read(number_of_card, EC_INTEL_E1000_PORT_EEPROM_READ) & 0x10)==0x10) {
            components->intel_e1000[number_of_card].eeprom_shift = 8;
        }
        logf("%d", components->intel_e1000[number_of_card].eeprom_shift);

        // read MAC address from EEPROM
        components->intel_e1000[number_of_card].mac_address[0] = (ec_intel_e1000_read_eeprom(number_of_card, 0x00) & 0xFF);
        components->intel_e1000[number_of_card].mac_address[1] = (ec_intel_e1000_read_eeprom(number_of_card, 0x00) >> 8);
        components->intel_e1000[number_of_card].mac_address[2] = (ec_intel_e1000_read_eeprom(number_of_card, 0x01) & 0xFF);
        components->intel_e1000[number_of_card].mac_address[3] = (ec_intel_e1000_read_eeprom(number_of_card, 0x01) >> 8);
        components->intel_e1000[number_of_card].mac_address[4] = (ec_intel_e1000_read_eeprom(number_of_card, 0x02) & 0xFF);
        components->intel_e1000[number_of_card].mac_address[5] = (ec_intel_e1000_read_eeprom(number_of_card, 0x02) >> 8);
    }
    else {
        logf("\nEEPROM not present");

        // read MAC address from ports
        for(dword_t i = 0; i < 6; i++) {
            components->intel_e1000[number_of_card].mac_address[i] = (ec_intel_e1000_read(number_of_card, 0x5400+i) & 0xFF);
        }
    }

    // log MAC address
    logf("\nMAC: ");
    for(dword_t i = 0; i < 6; i++) {
        logf("0x%02x ", components->intel_e1000[number_of_card].mac_address[i]);
    }

    // add card to list
    ethernet_cards[number_of_ethernet_cards].card_number = number_of_card;
    for(dword_t i = 0; i < 6; i++) {
        ethernet_cards[number_of_ethernet_cards].mac_address[i] = components->intel_e1000[number_of_card].mac_address[i];
    }
    ethernet_cards[number_of_ethernet_cards].get_cable_status = ec_intel_e1000_get_cable_status;
    ethernet_cards[number_of_ethernet_cards].send_packet = ec_intel_e1000_send_packet;
    ethernet_cards[number_of_ethernet_cards].process_irq = ec_intel_e1000_process_irq;
    number_of_ethernet_cards++;

    // start card
    ec_intel_e1000_write(number_of_card, EC_INTEL_E1000_PORT_DEVICE_CONTROL, ec_intel_e1000_read(number_of_card, EC_INTEL_E1000_PORT_DEVICE_CONTROL) | (1 << 6));

    // allocate memory of receive descriptors
    struct ec_intel_e1000_rx_descriptor_t *rx_descriptors = (struct ec_intel_e1000_rx_descriptor_t *) aligned_calloc(sizeof(struct ec_intel_e1000_rx_descriptor_t)*EC_INTEL_E1000_NUMBER_OF_RX_DESCRIPTORS, 0xFF);
    byte_t *rx_packets_memory = (byte_t *) malloc(MAX_SIZE_OF_ONE_PACKET*EC_INTEL_E1000_NUMBER_OF_RX_DESCRIPTORS);

    // set variables of receive descriptors
    components->intel_e1000[number_of_card].rx_descriptors_memory = (void *) rx_descriptors;
    components->intel_e1000[number_of_card].rx_packets_memory = rx_packets_memory;

    // initalize receive descriptors
    for(dword_t i = 0; i < EC_INTEL_E1000_NUMBER_OF_RX_DESCRIPTORS; i++) {
        // set memory of packet
        rx_descriptors[i].lower_packet_address = (dword_t) rx_packets_memory;
        rx_packets_memory += MAX_SIZE_OF_ONE_PACKET;

        // all other fields are zero
    }

    // initalize receive mechanism ports
    ec_intel_e1000_write(number_of_card, 0x2800, (dword_t)rx_descriptors); // lower memory of descriptors
    ec_intel_e1000_write(number_of_card, 0x2804, 0); // upper memory of descriptors
    ec_intel_e1000_write(number_of_card, 0x2808, sizeof(struct ec_intel_e1000_rx_descriptor_t)*EC_INTEL_E1000_NUMBER_OF_RX_DESCRIPTORS); // size of descriptors
    ec_intel_e1000_write(number_of_card, 0x2810, 0); // head of rx transfers
    ec_intel_e1000_write(number_of_card, 0x2818, EC_INTEL_E1000_NUMBER_OF_RX_DESCRIPTORS-1); // tail of rx transfers
    
    // allocate memory of transfer descriptors
    struct ec_intel_e1000_tx_descriptor_t *tx_descriptors = (struct ec_intel_e1000_tx_descriptor_t *) aligned_calloc(sizeof(struct ec_intel_e1000_tx_descriptor_t)*EC_INTEL_E1000_NUMBER_OF_TX_DESCRIPTORS, 0xFF);
    byte_t *tx_packets_memory = (byte_t *) malloc(MAX_SIZE_OF_ONE_PACKET*EC_INTEL_E1000_NUMBER_OF_TX_DESCRIPTORS);

    // set variables of receive descriptors
    components->intel_e1000[number_of_card].tx_descriptors_memory = (void *) tx_descriptors;
    components->intel_e1000[number_of_card].tx_packets_memory = tx_packets_memory;

    // initalize receive descriptors
    for(dword_t i = 0; i < EC_INTEL_E1000_NUMBER_OF_TX_DESCRIPTORS; i++) {
        // set memory of packet
        tx_descriptors[i].lower_packet_address = (dword_t) tx_packets_memory;
        tx_packets_memory += MAX_SIZE_OF_ONE_PACKET;

        // all other fields are zero
    }

    // initalize transfer mechanism ports
    ec_intel_e1000_write(number_of_card, 0x3800, (dword_t)tx_descriptors); // lower memory of descriptors
    ec_intel_e1000_write(number_of_card, 0x3804, 0); // upper memory of descriptors
    ec_intel_e1000_write(number_of_card, 0x3808, sizeof(struct ec_intel_e1000_tx_descriptor_t)*EC_INTEL_E1000_NUMBER_OF_TX_DESCRIPTORS); // size of descriptors
    ec_intel_e1000_write(number_of_card, 0x3810, 0); // head of tx transfers
    ec_intel_e1000_write(number_of_card, 0x3818, 0); // tail of tx transfers

    // enable receiving packets
    ec_intel_e1000_write(number_of_card, 0x100, ((1 << 1) | (1 << 3) | (0 << 5) | (1 << 15))); // enable receiver, unicast promiscuous mode, long packets disabled, accept broadcast packets
    
    // enable transmitting packets
    if(pci_is_device_in_list(components->intel_e1000[number_of_card].pci.vendor_id, components->intel_e1000[number_of_card].pci.device_id, intel_e1000e_supported_pci_devices) == STATUS_TRUE) { //e1000e cards
        ec_intel_e1000_write(number_of_card, 0x400, ((1 << 1) | (1 << 3) | (1 << 15) | (0x3F << 12) | (0x3 << 28)));
    }
    else { // e1000 cards
        ec_intel_e1000_write(number_of_card, 0x400, ((1 << 1) | (1 << 3) | (4 << 15) | (0x40 << 12)));
    }

    // enable interrupts
    pci_device_install_interrupt_handler(components->intel_e1000[number_of_card].pci, network_irq);
    ec_intel_e1000_write(number_of_card, 0xD0, 0x84); // link status change and receiving packets
    ec_intel_e1000_read(number_of_card, 0xC0);
}

byte_t ec_intel_e1000_get_cable_status(dword_t number_of_card) {
    if((ec_intel_e1000_read(number_of_card, 0x08) & 0x2)==0x2) {
        return ETHERNET_CARD_CABLE_CONNECTED;
    }
    else {
        return ETHERNET_CARD_CABLE_DISCONNECTED;
    }
}

byte_t ec_intel_e1000_send_packet(dword_t number_of_card, byte_t *packet_memory, dword_t packet_size) {
    // read last free descriptor number and one descriptor ahead
    dword_t free_descriptor_number = ec_intel_e1000_read(number_of_card, 0x3818); // read tail
    dword_t one_descriptor_ahead = (free_descriptor_number + 1);
    if(one_descriptor_ahead >= EC_INTEL_E1000_NUMBER_OF_TX_DESCRIPTORS) {
        one_descriptor_ahead = 0;
    }

    // check if descriptors are not full
    if(one_descriptor_ahead == ec_intel_e1000_read(number_of_card, 0x3810)) { // compare with head
        logf("\nERROR: Ethernet tx buffer is full");
        return STATUS_ERROR;
    }

    // copy packet to transfer packet memory
    copy_memory((dword_t)packet_memory, (dword_t)&components->intel_e1000[number_of_card].tx_packets_memory[MAX_SIZE_OF_ONE_PACKET*free_descriptor_number], packet_size);
    
    // add padding to too small packets
    if(packet_size < 60) {
        clear_memory((dword_t)&components->intel_e1000[number_of_card].tx_packets_memory[MAX_SIZE_OF_ONE_PACKET*free_descriptor_number]+packet_size, 60-packet_size);
        packet_size = 60;
    }

    // fill tx descriptor
    struct ec_intel_e1000_tx_descriptor_t *tx_descriptor = (struct ec_intel_e1000_tx_descriptor_t *) (((dword_t)components->intel_e1000[number_of_card].tx_descriptors_memory)+(sizeof(struct ec_intel_e1000_tx_descriptor_t)*free_descriptor_number));
    tx_descriptor->packet_size = packet_size;
    tx_descriptor->end_of_packet = 1;
    tx_descriptor->status_descriptor_done = 0;
    
    // move tail to tell card to send packet
    ec_intel_e1000_write(number_of_card, 0x3818, one_descriptor_ahead);

    return STATUS_GOOD;
}

void ec_intel_e1000_process_irq(dword_t number_of_card) {
    // read irq status to know what caused interrupt, mask only possible bits
    dword_t irq_status = (ec_intel_e1000_read(number_of_card, 0xC0) & 0xFFFFF);
    if(irq_status == 0x00000000) {
        return;
    }

    // packet was received
    if((irq_status & 0x80) == 0x80) {
        // for cycle will prevent infinite cycle if there is some error with moving head and tail
        for(dword_t i = 0; i < 2048; i++) {
            // get next descriptor after tail, because tail points to last descriptor that we processed
            dword_t tail = (ec_intel_e1000_read(number_of_card, 0x2818) + 1);
            if(tail >= EC_INTEL_E1000_NUMBER_OF_RX_DESCRIPTORS) {
                tail = 0;
            }

            // if this next descriptor equals head, we processed all packets
            if(tail == ec_intel_e1000_read(number_of_card, 0x2810)) {
                break;
            }

            // process packet
            struct ec_intel_e1000_rx_descriptor_t *rx_descriptors = (struct ec_intel_e1000_rx_descriptor_t *) (((dword_t)components->intel_e1000[number_of_card].rx_descriptors_memory)+(sizeof(struct ec_intel_e1000_rx_descriptor_t)*tail));
            if((rx_descriptors->rx_data_error | rx_descriptors->ipv4_checksum_error | rx_descriptors->tcp_udp_checksum_error)==0) {
                process_packet((byte_t *)rx_descriptors->lower_packet_address, rx_descriptors->packet_size);
            }

            // reset descriptor
            rx_descriptors->descriptor_done = 0;

            // move tail to tell card, that we processed this descriptor
            ec_intel_e1000_write(number_of_card, 0x2818, tail);
        }
    }

    // link status change
    if((irq_status & 0x04) == 0x04) {
        ethernet_cards_update_cable_status();
    }

    // acknowledge interrupt
    ec_intel_e1000_write(number_of_card, 0xC0, irq_status);
}