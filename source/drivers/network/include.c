//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "main.c"
#include "send_packets.c"
#include "receive_packets.c"
#include "network_transfers.c"

#include "ethernet/main.c"
#include "ethernet/amd_pcnet.c"
#include "ethernet/atheros.c"
#include "ethernet/broadcom_netxtreme.c"
#include "ethernet/intel_e1000.c"
#include "ethernet/realtek_8139.c"
#include "ethernet/realtek_8169.c"

#include "packets/main.c"
#include "packets/ethernet.c"
#include "packets/arp.c"
#include "packets/ipv4.c"
#include "packets/udp.c"
#include "packets/tcp.c"
#include "packets/dhcp.c"
#include "packets/dns.c"
#include "packets/http.c"

#include "protocols/dhcp.c"
#include "protocols/http.c"
#include "protocols/https.c"