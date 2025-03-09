#ifndef RCS660S_UART_H
#define RCS660S_UART_H

//#define UART_LAYER_DEBUG


/* グローバル変数をヘッダに書かないこと！ */

/*****************************/
/* 仕様ハードに合わせてinclude */
/*****************************/

//#include "rcs660s_uart_stub_visualstudio.h"
#include "rcs660s_uart_hw_arduino.h"

/*****************************/

#include <stdlib.h>
#include <stdint.h>

/*******************/
/* タイムアウト設定 */
/*******************/

#define RECEIVE_ACK_TIMEOUT        20 //ACK受信タイムアウト時間(ms) > 10ms (マニュアル Ver1.0 p.15 図2-4)
#define RECEIVE_DATA_TIMEOUT    60000 //データ受信タイムアウト時間(ms) ※十分に長くする リーダのタイムアウトより短いと途中でカード外すと領域外アイクセス発生

/*********************/
/* マニュアル指定定数 */
/*********************/

//マニュアル Ver1.0 p.11 表2-2 時点ではデータ長MAX = 0x0115バイト(277バイト)
#define RECEIVE_DATA_BUFF_SIZE 1 + 2 + 2 + 1 + 277 + 1 + 1
#define RCS660S_UART_MAX_RES_LEN    0x0115 //受信パケットデータの最長(マニュアル表2-2)


/*******************/
/* プロトタイプ宣言 */
/*******************/

void assemblyRcs660sUartCommandFrame(const uint8_t* , const uint16_t);

void uart_sendAck(void);

// detail ResetDevice と Power Down で ACK 送信が必要
// APDUコマンドの制約だがACKは2章(UART)で定義されているためuart層に関数を設ける 

void uart_receiver_init(void);
bool uart_receiver_checkACK(void);
bool uart_receiver_receiveData(uint8_t[], uint16_t*);


#endif // RCS660S_UART_H