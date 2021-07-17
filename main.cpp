/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "EthernetInterface.h"
#include <cstdio>

// Blinking rate in milliseconds
#define BLINKING_RATE     500ms
#define LED_ON 0
#define LED_OFF 1


// UDP Setting
#define SRC_IP "172.10.0.10"
#define DST_IP "172.10.0.11"
#define SRC_MASK "255.255.255.0"
#define SRC_GATEWAY "172.10.0.0"
#define SRC_PORT 59452
#define DST_PORT 50001

// Network Interface
EthernetInterface eth;

// Serial
BufferedSerial uart0(P2_0, P2_1);

static void dbgmsg(char *msg, size_t len = 0) {
    if (len < 1) {
        len = strlen(msg);
    }
    uart0.write(msg, strlen(msg));
}

int main()
{
    dbgmsg((char *)"start!\n");

    // Initialise the digital pin LED1 as an output
    DigitalOut led(LED1);// end
    DigitalOut led2(LED2);// error
    DigitalOut led3(LED3);// udp done
    DigitalOut led4(LED4);// start

    led = LED_OFF;
    led2 = LED_OFF;
    led3 = LED_OFF;
    led4 = LED_ON;

    //uart0.write("test\n", strlen("test\n"));

    eth.set_network(SRC_IP, SRC_MASK, SRC_GATEWAY);
    SocketAddress sockAddr;
    dbgmsg((char*)"UDP Socket test\n");
    if (0 != eth.connect()) {
        led2 = LED_ON;
        dbgmsg((char *)"Eth connecting ERROR\n");
        return -1;
    }

    dbgmsg((char *)"Eth connecting done\n");
    dbgmsg((char *)eth.get_mac_address());
    dbgmsg((char *)"\n");

    // Show the network address
    eth.get_ip_address(&sockAddr);
    dbgmsg((char *)sockAddr.get_ip_address());
    dbgmsg((char *)"\n");
 
    led3 = LED_ON;

    // UDPソケットを開く
    UDPSocket sock;
    sock.open(&eth);

    // 接続先のIPアドレスとポートを設定
    //eth.gethostbyname("host", &sockAddr);
    sockAddr.set_ip_address(DST_IP);
    sockAddr.set_port(DST_PORT);
    led = LED_ON;   
    char out_buffer[] = "test data";
    if (0 > sock.sendto(sockAddr, out_buffer, sizeof(out_buffer))) {
        led2 = LED_ON;
        dbgmsg((char *)"Error sending data\n");
        return -1;
    }
    dbgmsg((char *)"send done\n");

    char in_data[256];
    sock.recvfrom(&sockAddr, &in_data, sizeof(in_data));
    dbgmsg((char *)"UDP RECV---\n");
    dbgmsg((char *)in_data);

    while (true) {
        led = !led;
        ThisThread::sleep_for(BLINKING_RATE);
    }

    sock.close();
    eth.disconnect();
    dbgmsg((char *)"finish!");
    return 0;
}
