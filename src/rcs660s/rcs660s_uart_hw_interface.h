#ifndef RCS660S_UART_HW_INTERFACE_H
#define RCS660S_UART_HW_INTERFACE_H

// ハードレイヤで必要なメソッドのプロトタイプ宣言を書いておく

// オブジェクト指向で言うところのインターフェース
// 使う環境に応じてArduioと同じ振る舞いになるようにこのインターフェースをincludeして実装することで、
// 移植することができる

#include <stdio.h>

//プロトタイプ宣言
void setupSerial(void);

void uart_hw_sendUart(const uint8_t* , const uint16_t);

int uart_hw_available();
uint8_t uart_hw_read();

void uart_wait_ms(uint16_t);

void debugPrintMsg(const char*);
void debugPrintHex(const uint8_t); //charで受けるとFFが論理反転して表示バグる
void debugPrintDec(const int);

#endif // RCS660S_UART_HW_INTERFACE_H