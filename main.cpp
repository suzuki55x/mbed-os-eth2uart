/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
#include "DigitalOut.h"
#include "PinNames.h"
#include <cstring>
#define DEBUG

#include "mbed.h"
#include "EthernetInterface.h"
#include <cstdio>

// GPIO state
#define HIGH 1
#define LOW 0

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

// SPI interface
SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK);
DigitalOut cs(SPI_CS);

// Network Interface
EthernetInterface eth;

// Serial Setting
BufferedSerial uart0(CONSOLE_TX, CONSOLE_RX);// debug
BufferedSerial uart1(D2, D3, 9600);// M5Atom

#define UART_RX_BUF_SIZE (64)


static void dbgmsg(char *msg, size_t len = 0) {
#ifdef DEBUG
    if (len < 1) {
        len = strlen(msg);
    }
    uart0.write(msg, strlen(msg));
#endif
}

int main()
{
    dbgmsg((char *)"start!\n");

    // Initialise the digital pin LED1 as an output
    DigitalOut led(LED1);// UDP RX WAIT
    DigitalOut led2(LED2);// error
    DigitalOut led3(LED3);// UART RX WAIT
    DigitalOut led4(LED4);// power
    DigitalOut uart_valid(D0);// uart
    DigitalIn m5_sleep(D1);// sleep

    led = LED_OFF;
    led2 = LED_OFF;
    led3 = LED_OFF;
    led4 = LED_ON;
    uart_valid = 1;

    spi.format(8, 3);
    spi.frequency(1000000);// 1MHz

    cs = LOW;// selected

    spi.write(0x8F);// read command(for example)

    int reg = spi.write(0x00);// write dummy bite and read register result

    cs = HIGH;// deselect

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
 
    // UDPソケットを開く
    UDPSocket sock;
    sock.open(&eth);

    // 接続先のIPアドレスとポートを設定
    //eth.gethostbyname("host", &sockAddr);
    sockAddr.set_ip_address(DST_IP);
    sockAddr.set_port(DST_PORT);

  

    // UDP send test
    char out_buffer[] = "test data";
    if (0 > sock.sendto(sockAddr, out_buffer, sizeof(out_buffer))) {
        led2 = LED_ON;
        dbgmsg((char *)"Error sending data\n");
        return -1;
    }
    dbgmsg((char *)"UDP SEND DONE: ");
    dbgmsg(out_buffer);
    dbgmsg((char *)"\n");

    char* sense_interval;
    int sense_cnt = 1;// 測定回数
    char* sense_cnt_char;
    int data_cnt = 0;// 受信データ数
    char uart_rx_buffer[UART_RX_BUF_SIZE];
    char buf[1] = {'\0'};
    int cnt;
    char in_data[256];
    char in_data_cpy[256];

    // 以下無限ループ
    while (true) {
        // UDP文字列が正しくなるまでループ
        while(true) {
 
            dbgmsg((char *)"UDP receive waiting...\n");

            // UDP受信。受信するまで待つ(ブロッキング)
            led = LED_ON; 
            sock.recvfrom(&sockAddr, &in_data, sizeof(in_data));
            led = LED_OFF;
            dbgmsg((char *)"UDP RECV DONE!: ");
            dbgmsg((char *)in_data);
            dbgmsg((char *)"\n");

            strcpy(in_data_cpy, in_data);

            // 測定回数を取得
            sense_interval = strtok(in_data_cpy, " ");
            dbgmsg(sense_interval);
            if (sense_interval!=NULL) {
 
                sense_cnt_char = strtok(NULL, " ");
                if (sense_cnt_char!=NULL) {
                    sense_cnt = atoi(sense_cnt_char);

                    if(sense_cnt < 1) {
                        sense_cnt = 1;
                    }else if(sense_cnt > 120) {
                        sense_cnt = 120;
                    }
                    break;
                }
            }
        }

        // UDP受信結果をM5に送信
        if (uart1.writable()) {
            uart1.write(in_data, strlen(in_data));
            dbgmsg((char *)"uart to m5 send\n");
        } else {
            led2 = LED_ON;
            dbgmsg((char *)"ERROR: M5Atom Not Found\n");
            return -1;
        }
        uart_valid = 0;
        wait_us(1000);
        dbgmsg((char *)"Sleep...\n");
        while(m5_sleep == 0) {
            sleep();
        }
        dbgmsg((char *)"WakeUp!!!\n");
        // M5から受信
        led3 = LED_ON;
        cnt = 0;
        uart_valid = 1;
        wait_us(10000);
        while(true) {

            if (uart1.readable()) {
                // uart flush delay
                //ThisThread::sleep_for(100);
                
                uart1.read(buf, 1);

                //uart_valid = 0;

                //dbgmsg((char *)"read: ");
                //if(*buf!='\0') {
                // 力技で固定文字しかとらなくしてしまう
                if(
                    *buf == 't' ||
                    *buf == 'e' ||
                    *buf == 'm' ||
                    *buf == 'p' ||
                    *buf == 'h' ||
                    *buf == 'u' ||
                    *buf == 'm' ||
                    *buf == 'i' ||
                    *buf == 'o' ||
                    *buf == 's' ||
                    *buf == ':' ||
                    *buf == ' ' ||
                    *buf == ',' ||
                    *buf == '.' ||
                    *buf == '0' ||
                    *buf == '1' ||
                    *buf == '2' ||
                    *buf == '3' ||
                    *buf == '4' ||
                    *buf == '5' ||
                    *buf == '6' ||
                    *buf == '7' ||
                    *buf == '8' ||
                    *buf == '9'
                ) {
                    // 1文字目が化けることがあるので対策
                    if ((*buf == 't' && cnt==0) || (cnt>0)) {
                        uart_rx_buffer[cnt] = *buf;
                        //dbgmsg((char *)uart_rx_buffer);
                        //dbgmsg((char *)"\n");
                        cnt++;
                    } else if(cnt>0){

                    }
                }
                // 改行を受けたら1データ
                if((char)*buf=='\n'||(char)*buf=='\r') {
                    dbgmsg((char *)"Rx Done!!\n");
                    uart_rx_buffer[cnt] = '\0';
                    cnt++;
                    // M5からの受信が来たらUDP送信
                    if (0 > sock.sendto(sockAddr, uart_rx_buffer, sizeof(uart_rx_buffer))) {
                        led2 = LED_ON;
                        dbgmsg((char *)"Error sending data\n");
                        return -1;
                    }
                    dbgmsg((char *)"UDP SEND DONE: ");
                    dbgmsg(uart_rx_buffer);
                    dbgmsg((char *)"\n");

                    cnt = 0;
                    data_cnt++;
                    if(data_cnt >= sense_cnt) {
                        led3 = LED_OFF;
                        uart_valid = 1;
                        data_cnt = 0;
                        wait_us(10000);
                        break;
                    }
                }
                buf[0] = '\0';
            }
        }


        // buffer clear
        //for(int i=0; i<UART_RX_BUF_SIZE; i++) {
        //    uart_rx_buffer[i] = '\0';
        //}
    }

    sock.close();
    eth.disconnect();
    dbgmsg((char *)"finish!");
    return 0;
}