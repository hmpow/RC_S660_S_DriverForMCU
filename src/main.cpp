#include <Arduino.h>
#include "./rcs660s/rcs660s_apdu.h"
#include <vector>

//マニュアル指定定数
const uint8_t FULL_COMMAND_GetFirmWareVersion[] = {0x00, 0x00, 0xFF, 0x00, 0x0E, 0xF2, 0x6B, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x56, 0x00, 0x00, 0x3C, 0x00};

void setup() {
  // put your setup code here, to run once:
  setupSerial();
}

void loop() {
  // put your main code here, to run repeatedly:

  debugPrintMsg("Transceiver Test\n");
  //レシーバ初期化
  uart_receiver_init();

  delay(2000);

  //rcs660 に GetFirmwareVersionを送信
  debugPrintMsg("--TX GetFirmwareVersion--");
  assemblyAPDUcommand_GetFirmwareVersion();

  //ACK確認
  debugPrintMsg("--RX ACK--");
  (void)uart_receiver_checkACK();

  //データ受信
  debugPrintMsg("--RX DATA--");
  uint8_t sprittedDataArr[RECEIVE_DATA_BUFF_SIZE];
  uint16_t sprittedDataLen = 0;
  bool isReceived = false;

  isReceived = uart_receiver_receiveData(sprittedDataArr, &sprittedDataLen); 

  if(isReceived){
    debugPrintMsg("RX SUCCESS\nDATA = ");
    for (size_t i = 0; i < sprittedDataLen; i++)
    {
      debugPrintHex(sprittedDataArr[i]);
    }
    isReceived = false;

    debugPrintMsg("TEST CCID to APDU\n");
    
    std::vector<uint8_t> apduData = parseCCIDresponse_RDR_to_PC_Escape(sprittedDataArr, sprittedDataLen);
    debugPrintMsg("APDU DATA = ");
    for (size_t i = 0; i < apduData.size(); i++)
    {
      debugPrintHex(apduData[i]);
    }

    debugPrintMsg("\ncheckAPDU_response_ErrStatus実行");
    APDU_RESPONSE_ERROR_STATUS errStatus = checkAPDU_response_ErrStatus(apduData);
    debugPrintMsg("checkAPDU_response_ErrStatus結果_enum = ");
    debugPrintDec(errStatus);
 
  }else{
    debugPrintMsg("RX NO DATA");
  }
  debugPrintMsg("-----\n"); 

 #if 0 
  debugPrintMsg("TEST debugPrintCCIDresponse_bError\n"); 
  debugPrintMsg("-----\n"); 
  debugPrintCCIDresponse_bError(0x81);
  debugPrintMsg("-----\n"); 
  debugPrintMsg("TEST parseCCIDresponse_RDR_to_PC_DataBlock\n"); 
  const uint8_t testaaa[] = {0x80,0x01,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00};
  parseCCIDresponse_RDR_to_PC_DataBlock(testaaa, 10);
  const uint8_t testaab[] = {0x81,0x01,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00};
  parseCCIDresponse_RDR_to_PC_DataBlock(testaab, 10);
  const uint8_t testaac[] = {0x80,0x01};
  parseCCIDresponse_RDR_to_PC_DataBlock(testaaa, 2);
#endif
}
