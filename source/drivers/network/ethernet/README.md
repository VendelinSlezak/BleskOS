# Ethernet cards

This folder contains all drivers for ethernet cards.

During booting in method `scan_pci()` from source/drivers/system/pci.c are all ethernet cards saved to structure `ethernet_cards[]` from main.h. If there is driver for some card, its initalization method is saved to `ethernet_cards[].initalize()`. Then later during booting is called method `initalize_network_connection()` from source/drivers/network/main.c that calls method `initalize_ethernet_cards()` from main.c. This method sets every detected card info to default state, and if initalize() method is connected, it calls it. After this method are all ethernet cards we have driver for initalized.

From perspective of higher layers of code you need to know only about variable `number_of_ethernet_cards` that contains number of cards in `ethernet_cards[]` and following parts of `ethernet_cards[]` structure:

```
ethernet_cards[number_of_card].cable_status //ETHERNET_CARD_CABLE_DISCONNECTED or ETHERNET_CARD_CABLE_CONNECTED
ethernet_cards[number_of_card].get_cable_status(dword_t number_of_card)
ethernet_cards[number_of_card].send_packet(dword_t number_of_card, byte_t *memory_of_packet, dword_t length_of_packet)
```

Example:

```
// this sends packet of size 200 bytes from memory packet_mem from ethernet card that is on first entry of ethernet_cards[]
ethernet_cards[0].send_packet(0, packet_mem, 200);

// this code is executed, if ethernet card in third entry of ethernet_cards[] has connected cable to it
if(ethernet_cards[2].get_cable_status(2) == STATUS_TRUE) {
 // ...
}

// ERROR: number_of_card has to be same number, because ethernet_cards[number_of_card].get_cable_status calls
// driver specific method and get_cable_status(number_of_card) passes number of card to that method, so it knows from
// which ethernet_cards[] entry it should read card data such as port base to be able to work
ethernet_cards[1].get_cable_status(5);
```

## Drivers

Every driver for ethernet card needs to have following four methods:

```
void ec_nameofcard_initalize(dword_t number_of_card);
byte_t ec_nameofcard_get_cable_status(dword_t number_of_card);
byte_t ec_nameofcard_send_packet(dword_t number_of_card, byte_t *packet_memory, dword_t packet_size);
void ec_nameofcard_process_irq(dword_t number_of_card);
```

Variable number_of_card is used to access structure ethernet_cards: `ethernet_cards[number_of_card].something`. Every driver for card that uses descriptors should also contain definitions for number of rx and tx descriptors:

```
#define EC_NAMEOFCARD_NUMBER_OF_RX_DESCRIPTORS 256
#define EC_NAMEOFCARD_NUMBER_OF_TX_DESCRIPTORS 256
```

If you want to write driver for new card, do not forget that it is detected and added to ethernet_cards[] in `scan_pci()` from source/drivers/system/pci.c. 

### void ec_nameofcard_initalize(dword_t number_of_card)

This method is called only once during boot process. It needs to initalize card to state that is is capable of sending and receiving packets, and reporting status of cable. As first, it should log its name with device ID:

```
logf("\n\nNameOfCard driver\nDevice ID: 0x%02x", (ethernet_cards[number_of_card].id >> 16));
```

It needs to read MAC address of card and save it to `ethernet_cards[number_of_card].mac_address`.

It has to connect `ethernet_cards[number_of_card]` methods to driver specific methods:

```
//set card methods
ethernet_cards[number_of_card].get_cable_status = ec_nameofcard_get_cable_status;
ethernet_cards[number_of_card].send_packet = ec_nameofcard_send_packet;
ethernet_cards[number_of_card].process_irq = ec_nameofcard_process_irq;
```

It also needs to connect irq of card to method `network_irq()` from source/drivers/network/main.c.

```
//enable interrupts
set_irq_handler(ethernet_cards[number_of_card].irq, (dword_t)network_irq);
```

When `network_irq()` is be fired, it will call `ec_nameofcard_process_irq()`. Because `network_irq()` calls all irq handlers, if status of card interrupt is clear, you have to immediately return, because it mean that this interrupt was not fired by this card.

Rest of this method is up to implementation of specific card.

### byte_t ec_nameofcard_get_cable_status(dword_t number_of_card)

This method returns STATUS_TRUE is cable is connected to this card and STATUS_FALSE, if it is disconnected.

### byte_t ec_nameofcard_send_packet(dword_t number_of_card, byte_t *packet_memory, dword_t packet_size)

This method sends packet through ethernet card. It returns STATUS_GOOD on sending packet/adding it to queue or STATUS_ERROR if it is impossible to send packet or queue is full. This method can not wait by relying on any interrupt, because it is called during interrupt handler, so such approach would block OS. Returning STATUS_GOOD from this method do not that packet was already sended by card, but that it was passed to card and it will be sended when card will found it.

### void ec_nameofcard_process_irq(dword_t number_of_card)

This method processes IRQ produced by card. It can be called even if this card did not produced interrupt, so it is important to return if interrupt status is clear.

## main.c

### void initalize_ethernet_cards(void)

This method initalizes all detected ethernet cards.

### ethernet_void_*

These methods are connected to cards that do not have driver.

### void ethernet_cards_update_cable_status(void)

This method updates all `ethernet_cards[].cable_status` variables to actual state.

### void select_ethernet_card_as_internet_connection_device(dword_t number_of_card)

If we want to use ethernet card as connection to internet, this method initalizes `internet` structure from source/drivers/network/main.h for concrete card to be used. number_of_card is number of card entry in `ethernet_cards[]`

### byte_t send_packet_to_internet_through_ethernet(byte_t *packet_memory, dword_t packet_size)

If we use ethernet card as connection to internet, this method is connected to `internet.send_packet()` from source/drivers/network/main.h. It sends packet to card that was selected by `select_ethernet_card_as_internet_connection_device()`.

### void ethernet_cards_process_irq(void)

This method calls irq handles of all ethernet cards. It is called from `network_irq()` from source/drivers/network/main.c when interrupt is fired.

## TODO

* Write drivers for Qualcomm Atheros and Broadcom ethernet cards