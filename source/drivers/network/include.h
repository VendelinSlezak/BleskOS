//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 BleskOS developers
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "main.h"
#include "send_packets.h"
#include "receive_packets.h"
#include "network_transfers.h"

#include "ethernet/main.h"
#include "ethernet/amd_pcnet.h"
#include "ethernet/atheros.h"
#include "ethernet/broadcom_netxtreme.h"
#include "ethernet/intel_e1000.h"
#include "ethernet/realtek_8139.h"
#include "ethernet/realtek_8169.h"

#include "packets/main.h"
#include "packets/ethernet.h"
#include "packets/arp.h"
#include "packets/ipv4.h"
#include "packets/udp.h"
#include "packets/tcp.h"
#include "packets/dhcp.h"
#include "packets/dns.h"
#include "packets/http.h"

#include "protocols/dhcp.h"
#include "protocols/http.h"
#include "protocols/https.h"