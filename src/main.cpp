#include <Arduino.h>
#include "./rcs660s/rcs660s_app_if.h"
#include <vector>

#define TEST_INTERVAL_MS 1000
#define TEST_LOOP_INTERVAL_MS 30000
#define TEST_WAIT_HUMAN_READABLE_INTERVAL_MS 3000

//マニュアル指定定数
const uint8_t FULL_COMMAND_GetFirmWareVersion[] = {0x00, 0x00, 0xFF, 0x00, 0x0E, 0xF2, 0x6B, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x56, 0x00, 0x00, 0x3C, 0x00};

Rcs660sAppIf rcs660sAppIf;

void printCardRes(const std::vector<uint8_t> vec){
  if(vec.empty() == false){
    debugPrintMsg("CARD RES START");
    for (size_t i = 0; i < vec.size(); i++)
    {
      debugPrintHex(vec[i]);
    }
    debugPrintMsg("CARD RES END");
  }else{
    debugPrintMsg("ERROR! CARD RES is empty");
  }
  uart_wait_ms(TEST_WAIT_HUMAN_READABLE_INTERVAL_MS);
  return;
}

void setup() {
  // put your setup code here, to run once:
  rcs660sAppIf.begin();
  delay(10000);//PIO Upload → シリアルモニタ切替待機
}

void loop() {
  // put your main code here, to run repeatedly:

  debugPrintMsg("◆◆◆ テストループ開始 ◆◆◆");
  /*********************************************************************/
  debugPrintMsg("【コマンド実行】リセットデバイス");

  rcs660sAppIf.resetDevice();
  delay(TEST_INTERVAL_MS);

  /*********************************************************************/
  //rcs660 に GetFirmwareVersionを送信 マニュアルモード
  debugPrintMsg("【コマンド実行】GetFirmwareVersion");

  uart_receiver_init();
  assemblyAPDUcommand_GetFirmwareVersion();
  (void)rcs660sAppIf.receiveSequence(TEST_RX_MODE_WO_TLV);
  delay(TEST_INTERVAL_MS);
  
  /*********************************************************************/

  rcs660sAppIf.setNfcType(NFC_TYPE_B);
  rcs660sAppIf.updateTxAndRxFlag({false, false, 3, false});
  bool isCatch = rcs660sAppIf.catchNfc(RETRY_CATCH_INFINITE);

  if(isCatch == E_OK){
    debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★");
    debugPrintMsg("★★★★★★★★★ カードと通信開始 ★★★★★★★★★");
    debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★\n");

    debugPrintMsg("★ MFを選択 Case0 タイムアウトなし ★");
    uart_wait_ms(TEST_WAIT_HUMAN_READABLE_INTERVAL_MS);

    const std::vector<uint8_t> selectMfCase0 = { 0x00,0xA4,0x00,0x00 };
    std::vector<uint8_t> nfc_res;

    nfc_res = rcs660sAppIf.communicateNfc(selectMfCase0, 0);
    printCardRes(nfc_res);

    /********************************************************/

    uart_wait_ms(TEST_WAIT_HUMAN_READABLE_INTERVAL_MS);
    if (nfc_res.empty() == false)
    {
      nfc_res.clear();
    }

    /********************************************************************/
    debugPrintMsg("★ MFを選択 Case0 タイムアウト 60ms ★");
    uart_wait_ms(TEST_WAIT_HUMAN_READABLE_INTERVAL_MS);

    nfc_res = rcs660sAppIf.communicateNfc(selectMfCase0, 60);
    printCardRes(nfc_res);

    /********************************************************/
    
    uart_wait_ms(TEST_WAIT_HUMAN_READABLE_INTERVAL_MS);
    if (nfc_res.empty() == false)
    {
      nfc_res.clear();
    }

    /********************************************************************/
    debugPrintMsg("★ MFを選択 Case3 タイムアウト10000 ★");
    uart_wait_ms(TEST_WAIT_HUMAN_READABLE_INTERVAL_MS);

    const std::vector<uint8_t> selectMfCase3 = {0x00,0xA4,0x00,0x00,0x02,0x3F,0x00};
    
    nfc_res = rcs660sAppIf.communicateNfc(selectMfCase3, 10000);
    printCardRes(nfc_res);
    
    /********************************************************/

    uart_wait_ms(TEST_WAIT_HUMAN_READABLE_INTERVAL_MS);
    if (nfc_res.empty() == false)
    {
      nfc_res.clear();
    }
    debugPrintMsg("★ MFを選択 Case3 FCI応答なし 暗号化 タイムアウトなし ★");
    uart_wait_ms(TEST_WAIT_HUMAN_READABLE_INTERVAL_MS);

    const std::vector<uint8_t> selectMfCase3_NoFCI = {0x00,0xA4,0x00,0x0C,0x02,0x3F,0x00};
  
    nfc_res = rcs660sAppIf.communicateNfc(selectMfCase3_NoFCI, 0);
    printCardRes(nfc_res);

    /********************************************************/

    uart_wait_ms(TEST_WAIT_HUMAN_READABLE_INTERVAL_MS);
    if (nfc_res.empty() == false)
    {
      nfc_res.clear();
    }

    /********************************************************************/
    debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★");
    debugPrintMsg("★★★★★★★★★ カードと通信終了 ★★★★★★★★★");
    debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★\n");
    uart_wait_ms(TEST_WAIT_HUMAN_READABLE_INTERVAL_MS);
  }


  delay(TEST_INTERVAL_MS);
  rcs660sAppIf.releaseNfc();

debugPrintMsg("◆◆◆ テストループ終了 ◆◆◆");
delay(TEST_LOOP_INTERVAL_MS);
}

