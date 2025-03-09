#ifndef RCS660S_UART_HW_WINDOWSPC_H
#define RCS660S_UART_HW_WINDOWSPC_H

// PCシミュレーション用
// RC-S660S への出力　標準出力
// RC-S660/S からの入力　配列指定 ファイル入力にしたい
// デバッグメッセージ　PC　標準出力

#include <stdio.h>

//UARTの設定
#define UART_PC Serial
#define UART_RCS660S Serial1

//プロトタイプ宣言の読み込み
#include "rcs660s_uart_hw_interface.h"


#endif // RCS660S_UART_HW_ARDUINO_H
