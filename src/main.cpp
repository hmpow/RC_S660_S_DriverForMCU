#include <Arduino.h>
#include "./rcs660s/rcs660s_apdu.h"
#include "./rcs660s/rcs660s_uart.h"
#include <vector>

#define TEST_INTERVAL_MS 1000
#define TEST_LOOP_INTERVAL_MS 30000
#define TEST_TOUCH_WAIT_INTERVAL_MS TEST_INTERVAL_MS
#define ERROR_OCCURED true
#define ERROR_NOT_OCCURED false

typedef enum _test_rx_mode{
  TEST_RX_MODE_WO_TLV = 0,
  TEST_RX_MODE_W_TLV,
  TEST_RX_MODE_W_TLV_AND_ATR_TYPE_A,
  TEST_RX_MODE_W_TLV_AND_ATR_TYPE_B,
  TEST_RX_MODE_W_TLV_AND_CARD_RES,
}TEST_RX_MODE;

//マニュアル指定定数
const uint8_t FULL_COMMAND_GetFirmWareVersion[] = {0x00, 0x00, 0xFF, 0x00, 0x0E, 0xF2, 0x6B, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x56, 0x00, 0x00, 0x3C, 0x00};

bool receiveData(TEST_RX_MODE);

void setup() {
  // put your setup code here, to run once:
  setupSerial();
  delay(10000);//PIO Upload → シリアルモニタ切替待機
}

void loop() {
  // put your main code here, to run repeatedly:

  debugPrintMsg("◆◆◆ テストループ開始 ◆◆◆");
  /*********************************************************************/
  debugPrintMsg("【コマンド実行】リセットデバイス");
  executeResetDeviceSequence();
  uart_receiver_init();
  (void)receiveData(TEST_RX_MODE_WO_TLV); //ResetDeviceの完了通知は来ないのでACKタイムアウトで待機代用
  /*********************************************************************/
  debugPrintMsg("◆ レシーバ初期化 ◆");
  uart_receiver_init();
  delay(TEST_INTERVAL_MS);

  //rcs660 に GetFirmwareVersionを送信
  debugPrintMsg("【コマンド実行】GetFirmwareVersion");
  assemblyAPDUcommand_GetFirmwareVersion();
  
  
  (void)receiveData(TEST_RX_MODE_WO_TLV);

  /*********************************************************************/
  debugPrintMsg("◆ レシーバ初期化 ◆");
  uart_receiver_init();
  delay(TEST_INTERVAL_MS);

  debugPrintMsg("【コマンド実行】ManageSession_StartTransparentSession OK期待");
  assemblyAPDUcommand_ManageSession_StartTransparentSession();

  
  (void)receiveData(TEST_RX_MODE_W_TLV);
  /*********************************************************************/
  debugPrintMsg("◆ レシーバ初期化 ◆");
  uart_receiver_init();
  delay(TEST_INTERVAL_MS);

  debugPrintMsg("【コマンド実行】ManageSession_StartTransparentSession 2重呼び出しエラー期待");
  assemblyAPDUcommand_ManageSession_StartTransparentSession();

  
  (void)receiveData(TEST_RX_MODE_W_TLV);
  /*********************************************************************/
  debugPrintMsg("【コマンド実行】リセットデバイス ◆");
  executeResetDeviceSequence();
  uart_receiver_init();
  (void)receiveData(TEST_RX_MODE_WO_TLV); //ResetDeviceの完了通知は来ないのでACKタイムアウトで待機代用
  /*********************************************************************/
  debugPrintMsg("◆ レシーバ初期化 ◆");
  uart_receiver_init();
  delay(TEST_INTERVAL_MS);

  debugPrintMsg("【コマンド実行】ManageSession_StartTransparentSession OK期待");
  assemblyAPDUcommand_ManageSession_StartTransparentSession();

  
  (void)receiveData(TEST_RX_MODE_W_TLV);
  /*********************************************************************/
  debugPrintMsg("◆ レシーバ初期化 ◆");
  uart_receiver_init();
  delay(TEST_INTERVAL_MS);

  do{
    debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★");
    debugPrintMsg("★★★★★NFC-TypeBカードをタッチしてください★★★★");
    debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★");
    delay(TEST_TOUCH_WAIT_INTERVAL_MS);

    debugPrintMsg("【コマンド実行】SwitchProtocol_TypeB_AutoActivate(レイヤー4)");
    assemblyAPDUcommand_SwitchProtocol_TypeB_AutoActivate();
    
  }while(receiveData(TEST_RX_MODE_W_TLV_AND_ATR_TYPE_B));

  /*********************************************************************/
  debugPrintMsg("◆ レシーバ初期化 ◆");
  uart_receiver_init();
  delay(TEST_INTERVAL_MS);

  debugPrintMsg("【コマンド実行】TransparentExchange_TransmissionAndReceptionFlag");
  debugPrintMsg("txDoNotAppendCRC = false, rcDoNotDiscardCRC = false, transceiveParity = 3, doNotAppendOrDiscardProcolProloge = false\n");
  assemblyAPDUcommand_TransparentExchange_TransmissionAndReceptionFlag(false, false, 3, false);
  
  (void)receiveData(TEST_RX_MODE_W_TLV);
  
  /*********************************************************************/
  debugPrintMsg("◆ レシーバ初期化 ◆");
  uart_receiver_init();
  delay(TEST_INTERVAL_MS);

  debugPrintMsg("【コマンド実行】ManageSession_TrunOnRfField");
  assemblyAPDUcommand_ManageSession_TrunOnRfField();

  
  (void)receiveData(TEST_RX_MODE_W_TLV);

  /*********************************************************************/
  /*********************************************************************/
  /*********************************************************************/
  debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★");
  debugPrintMsg("★★★★★★★★★ カードと通信開始 ★★★★★★★★★");
  debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★");
  /*********************************************************************/
  debugPrintMsg("◆ レシーバ初期化 ◆");
  uart_receiver_init();
  delay(TEST_INTERVAL_MS);

  debugPrintMsg("【コマンド実行】assemblyAPDUcommand_Transparent_Exchange_Transceive\n");
  uint8_t data[] = { 0x00,0xA4,0x00,0x00 };
  debugPrintMsg("****** SELECT MF CASE 1 *****\n");
  debugPrintMsg("WirelessCommand = {00 A4 00 00}, WirelessCommand_Len = 4,  timeout_ms = 0\n");
  //Transceive_Func(data, (uint8_t)4, (uint16_t)0);

/****************************/
/****************************/
/****************************/
// 中身を出す

APDU_DATA_OBJECT transmit_data_object = {0};

//固定値(マニュアル指定値)
transmit_data_object.Tag = (uint8_t)0x95; //Transceive

//変数
transmit_data_object.Length = 4;

for(uint8_t i = 0; i < transmit_data_object.Length; i++){
    transmit_data_object.Value[i] = data[i];
}

//Base関数へパス
_assemblyAPDUcommand_TransparentExchange_Base(transmit_data_object, 0);

/****************************/
/****************************/
/****************************/



  (void)receiveData(TEST_RX_MODE_W_TLV_AND_CARD_RES);
  /********************************************************/
  debugPrintMsg("◆ レシーバ初期化 ◆");
  uart_receiver_init();
  delay(TEST_INTERVAL_MS);

  debugPrintMsg("WirelessCommand = {00 A4 00 00}, WirelessCommand_Len = 4,  timeout_ms = 60\n");
  //assemblyAPDUcommand_TransparentExchange_Transceive(data, 4, 60);

  (void)receiveData(TEST_RX_MODE_W_TLV_AND_CARD_RES);
  /********************************************************/
  debugPrintMsg("◆ レシーバ初期化 ◆");
  uart_receiver_init();
  delay(TEST_INTERVAL_MS);

  debugPrintMsg("\n****** SELECT MF CASE 3 , P1 = 00 , P2 = 00  *****\n\n");
  const uint8_t data_2[] = {0x00,0xA4,0x00,0x00,0x02,0x3F,0x00};
  debugPrintMsg("WirelessCommand = {00,A4,00,00,02,3F,00}, WirelessCommand_Len = 7,  timeout_ms = 0\n");
  //assemblyAPDUcommand_TransparentExchange_Transceive(data_2, 7, 0);

  (void)receiveData(TEST_RX_MODE_W_TLV_AND_CARD_RES);
  /********************************************************/

  /*********************************************************************/
  debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★");
  debugPrintMsg("★★★★★★★★★ カードと通信終了 ★★★★★★★★★");
  debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★");

  /*********************************************************************/
  /*********************************************************************/
  /*********************************************************************/

  debugPrintMsg("◆ レシーバ初期化 ◆");
  uart_receiver_init();
  delay(TEST_INTERVAL_MS);

  debugPrintMsg("【コマンド実行】ManageSession_TrunOffRfField");
  assemblyAPDUcommand_ManageSession_TrunOffRfField();

  
  (void)receiveData(TEST_RX_MODE_W_TLV);
  /*********************************************************************/
  debugPrintMsg("◆ レシーバ初期化 ◆");
  uart_receiver_init();
  delay(TEST_INTERVAL_MS);

  debugPrintMsg("【コマンド実行】EndTransparentSession");
  assemblyAPDUcommand_ManageSession_EndTransparentSession();

  
  (void)receiveData(TEST_RX_MODE_W_TLV);

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
debugPrintMsg("◆◆◆ テストループ終了 ◆◆◆");
delay(TEST_LOOP_INTERVAL_MS);
}




bool receiveData(TEST_RX_MODE mode){

  debugPrintMsg("★★ 受信開始 ★★");

  uint8_t sprittedDataArr[RECEIVE_DATA_BUFF_SIZE];
  uint16_t sprittedDataLen = 0;
  bool isReceived = false;
  bool isACKok = false;

  //ACK確認
  debugPrintMsg("★ ACK ★");
  isACKok = uart_receiver_checkACK();
  if(isACKok){
    debugPrintMsg("ACK OK");
  }else{
    debugPrintMsg("ACK NG");
    return ERROR_OCCURED;
  }
  
  //データ受信
  debugPrintMsg("★ データ ★");

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
    const APDU_RESPONSE_ERROR_STATUS errStatus = checkAPDU_response_ErrStatus(apduData);
    debugPrintMsg("checkAPDU_response_ErrStatus結果_enum = ");
    debugPrintDec(errStatus);

    if(errStatus != STATUS_OK){
      return ERROR_OCCURED;
    }

    if(mode != TEST_RX_MODE_WO_TLV){
      debugPrintMsg("★★★★★★★★★ TLV解析 ★★★★★★★★★");
      // 2. 1 が OK なら TLVのバイト列取出し
      debugPrintMsg("parseAPDU_response_DataObjects実行");
      const std::vector <APDU_DATA_OBJECT> apduDataObj = parseAPDU_response_DataObjects(apduData);

      // 3. 2 から TLV セットにエラーがないか確認
      debugPrintMsg("checkAPDU_dataObject_ErrStatus実行");
      const APDU_RESPONSE_ERROR_STATUS ares = checkAPDU_dataObject_ErrStatus(apduDataObj);
      debugPrintMsg("checkAPDU_dataObject_ErrStatus結果_enum = ");
      debugPrintDec(ares);
      if(ares != STATUS_OK){
        return ERROR_OCCURED;
      }

      // 4. 3 がOKなら TLV セットを解析
      if(mode == TEST_RX_MODE_W_TLV_AND_CARD_RES){
        debugPrintMsg("getCardResponse_from_TransparentExchangeResponse実行");
        std::vector<uint8_t> cardRes = getCardResponse_from_TransparentExchangeResponse(apduDataObj);
        debugPrintMsg("getCardResponse_from_TransparentExchangeResponse結果 = ");
        if(cardRes.size() == 0){
          debugPrintMsg("NO DATA");
        }else{
          debugPrintMsg("CARD RES START");
          for (size_t i = 0; i < cardRes.size(); i++)
          {
            debugPrintHex(cardRes[i]);
          }
          debugPrintMsg("CARD RES END");
        }
      }else if(mode == TEST_RX_MODE_W_TLV_AND_ATR_TYPE_B){
        debugPrintMsg("getTypeB_ATR_from_SwitchProtocolResponse実行");
        NFC_TYPE_B_ATR atr = getTypeB_ATR_from_SwitchProtocolResponse(apduDataObj);
        debugPrintMsg("getTypeB_ATR_from_SwitchProtocolResponse結果 = ");
        debugPrintMsg("ATQB APP DATA");
        for (size_t i = 0; i < 4; i++)
        {
          debugPrintHex(atr.atpbAppData[i]);
        }
        debugPrintMsg("ATQB PROTOCOL INFO");
        for (size_t i = 0; i < 3; i++)
        {
          debugPrintHex(atr.atpbProtocolInfo[i]);
        }
        debugPrintMsg("ATQB ATTRIB");
        debugPrintHex(atr.atqbAttrib);

        debugPrintMsg("ATQB END");

      }
    }


  }else{
    debugPrintMsg("RX NO DATA");
  }

  debugPrintMsg("★★ 受信完了 ★★");
  return ERROR_NOT_OCCURED;
}