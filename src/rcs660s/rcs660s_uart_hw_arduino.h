#ifndef RCS660S_UART_HW_ARDUINO_H
#define RCS660S_UART_HW_ARDUINO_H

//#define UART_HW_LAYER_DEBUG

// 本番用
// RC-S660S への出力　UART通信
// RC-S660/S からの入力　UART通信
// デバッグメッセージ　PC　シリアルポート

#include <Arduino.h>

//UARTの設定
#define UART_PC Serial
#define UART_RCS660S Serial1

//プロトタイプ宣言の読み込み
#include "rcs660s_uart_hw_interface.h"


#endif // RCS660S_UART_HW_ARDUINO_H
