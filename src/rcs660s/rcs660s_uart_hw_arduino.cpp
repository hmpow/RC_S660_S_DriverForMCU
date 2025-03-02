/* 重要 拡張子はc++にしておかないと名前解決できずエラーになる */
#include "rcs660s_uart_hw_arduino.h"

void setupSerial(void){
    UART_PC.begin(9600);
    UART_RCS660S.begin(115200);
    return;
}


//UART送信 Arduino用
void sendUart(const uint8_t CommandFrame[], const uint16_t LEN){

    //ToDo 0から始まるはずなのにLEN要素まで入れないとダメ → 上位レイヤにバグがあるはず

    debugPrintMsg("sendUart_LEN = ");
    debugPrintDec(LEN);
    debugPrintMsg("sendUart_DATA = ");
    for (size_t i = 0; i <= LEN; i++){
        debugPrintHex(CommandFrame[i]);
    }

    UART_RCS660S.write(CommandFrame, LEN+1);

    return;
}

int uart_hw_available(){
    return UART_RCS660S.available();
}

uint8_t uart_hw_read(){
    return (uint8_t)UART_RCS660S.read();
}


void debugPrintMsg(const char* str){
    UART_PC.print("\n");
    UART_PC.println(str);
    return;
}

void debugPrintHex(const uint8_t hex){
    UART_PC.print(hex, HEX);
    UART_PC.print(",");
    return;
}

void debugPrintDec(const int dec){
    UART_PC.print(dec, DEC);
    UART_PC.print("\n");
    return;
}