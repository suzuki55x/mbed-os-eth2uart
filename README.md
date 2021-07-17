# eth2uart

## Environment

- [mbed studio](https://os.mbed.com/studio/)
- m5-ulp-uart v2以上 & M5Atom Lite
- udp送受信を行えるツール
- シリアル通信クライアント(デバッグ用)

## Target

[ARCH PRO](https://wiki.seeedstudio.com/Arch_Pro/)

## Pin Assign

| 信号名     | M5Atom ピン | ARCH PRO ピン | 
| ------------------ | ----------- | ----------- |
| UART TX(M5->ARCH)  | G21         | P0_1        |
| UART RX(M5<-ARCH)  | G25         | P0_0        |
| 5V                 | 5V         | 5V            |
| GND                | GND         | GND        |
| 予約                | G22         | |
| 予約                | G19         | |
| 予約                | G23         | |
| ARCH PRO Sleep     | G33         | P0_28 |
| ARCH PRO DEBUG Serial Tx |          | P2_0 |
| ARCH PRO DEBUG Serial Rx |          | P2_1 |

## ARCH PRO LED

| LED番号 | 点灯 | 消灯 |
| --- | --- | ---|
|LED1 | UDP受信待機中 | UDP受信未待機 |
|LED2 | エラー発生 | 正常動作 |
|LED3 | UART受信待機中 | UART受信未待機|
|LED4 | Power ON | Power OFF |
