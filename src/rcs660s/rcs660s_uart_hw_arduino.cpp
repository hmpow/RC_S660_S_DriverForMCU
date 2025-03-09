/* 重要 拡張子はc++にしておかないと名前解決できずエラーになる */
#include "rcs660s_uart_hw_arduino.h"

void setupSerial(void){
    UART_PC.begin(9600);
    UART_RCS660S.begin(115200);
    return;
}


//UART送信 Arduino用
void uart_hw_sendUart(const uint8_t CommandFrame[], const uint16_t LEN){

#ifdef UART_HW_LAYER_DEBUG
    debugPrintMsg("uart_hw_sendUart_LEN = ");
    debugPrintDec(LEN);
    debugPrintMsg("uart_hw_sendUart_DATA = ");
    for (size_t i = 0; i < LEN; i++){
        debugPrintHex(CommandFrame[i]);
    }
#endif

    UART_RCS660S.write(CommandFrame, LEN);

    return;
}

int uart_hw_available(){
    return UART_RCS660S.available();
}

uint8_t uart_hw_read(){
    return (uint8_t)UART_RCS660S.read();
}


void uart_wait_ms(uint16_t ms){
    delay(ms);
    return;
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