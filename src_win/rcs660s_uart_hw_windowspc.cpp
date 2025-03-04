/* 重要 拡張子はc++にしておかないと名前解決できずエラーになる */
#include "rcs660s_uart_hw_windowspc.h"

void setupSerial(void){
    
    return;
}


//UART送信 Arduino用
void uart_hw_sendUart(const uint8_t CommandFrame[], const uint16_t LEN){
    //コマンドをuart送信 の代わりに画面に表示
    
    uint16_t i = 0;
    
    printf("---------------------------------------\n\n");

    printf("FULL UART Command Output for Visual Inspection : \n ");
    for(i = 0; i <= LEN; i++){
        printf("%02X ",CommandFrame[i]);
    }

    printf("\n\n----Output for use with COPY AND PASTE----");
    
    printf("\nFor TeraTerm Macro TTL file : \n send ");
    for(i = 0; i <= LEN; i++){
        printf("$%02X",CommandFrame[i]);
    }
    printf("\n mpause 100\n");
    
    printf("\nFor Arduino Serial.write : \n");
    printf(" const uint8_t buf[] = {0x%02X",CommandFrame[0]);
    for(i = 1; i <= LEN; i++){
        printf(", 0x%02X",CommandFrame[i]);
    }
    printf("};\n");
    printf(" const uint16_t len = %d;\n", LEN);
    printf(" Serial.write(buf, len);\n");
    printf("---------------------------------------\n\n");

    return;
}

int uart_hw_available(){
    return 0;
}

uint8_t uart_hw_read(){
    return 0;
}


void uart_wait_ms(uint16_t ms){
    delay(ms);
    return;
}


void debugPrintMsg(const char* str){
    printf("\n");
    printf("%s\n",str);
    return;
}

void debugPrintHex(const uint8_t hex){
    printf("%0X", hex);
    printf(",");
    return;
}

void debugPrintDec(const int dec){
    printf("%d", dec);
    printf("\n");
    return;
}