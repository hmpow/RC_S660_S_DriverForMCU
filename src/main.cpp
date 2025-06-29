#include <Arduino.h>
#include "./rcs660s/rcs660s_app_if.h"
#include <vector>

#define TEST_INTERVAL_MS 1000
#define TEST_LOOP_INTERVAL_MS 30000 //シリアル出力を手動でコピペするための時間
#define TEST_WAIT_HUMAN_READABLE_INTERVAL_MS 3000

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

  std::vector<uint8_t> nfc_res;

  debugPrintMsg("◆◆◆ テストループ開始 ◆◆◆");
  
  /* TEST 1 リセットデバイス *********************************************/

  debugPrintMsg("リセットデバイス");

  rcs660sAppIf.resetDevice();
  delay(TEST_INTERVAL_MS);

  /* TEST 2 APDU層をAppからコール ***************************************/

  //rcs660 に GetFirmwareVersionを送信 マニュアルモード
  debugPrintMsg("【マニュアルモード】GetFirmwareVersion");

  uart_receiver_init();
  assemblyAPDUcommand_GetFirmwareVersion();
  (void)rcs660sAppIf.receiveSequence(TEST_RX_MODE_WO_TLV);
  delay(TEST_INTERVAL_MS);

  /* TEST 3 NFC Type-A 捕捉(有限リトライ) ********************************/

  debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★");
  debugPrintMsg("★★★★★★★★★ NFC-TypeA カードをタッチ ★★★★★★★★★");
  debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★\n");
  
  uart_wait_ms(TEST_WAIT_HUMAN_READABLE_INTERVAL_MS);

  //NFC Type-A通信用に設定
  rcs660sAppIf.setNfcType(NFC_TYPE_A);
  rcs660sAppIf.updateTxAndRxFlag({false, false, 0, false});
  
  //NFC-TypeAを捕捉 リトライ回数10
  bool isCatch = rcs660sAppIf.catchNfc(10);

  if(isCatch == E_OK){
    debugPrintMsg("NFC-TypeA 捕捉 OK");

    //UID 読み出し ： RC-S660 のように返してくれないので GET DATA 無線コマンド叩く
    debugPrintMsg("★ GET DATE で UID 取得 タイムアウト 100ms ★");
    
    //const std::vector<uint8_t> readUidTypeA = {0xFF, 0xCA, 0x00, 0x00, 0x00}; //JIS X 6320-4：2017 表89 偶数INS : 68 81 (CLAで示される機能は提供しない/指定された論理チャネルを提供していない) になる
    const std::vector<uint8_t> readUidTypeA = {0xFF, 0xCB, 0x00, 0x00, 0x00}; //JIS X 6320-4：2017 表90 奇数INS : 68 81 (CLAで示される機能は提供しない/指定された論理チャネルを提供していない) になる


    nfc_res = rcs660sAppIf.communicateNfc(readUidTypeA, 100);
    
    printCardRes(nfc_res);

  }else{
    debugPrintMsg("NFC-TypeA 捕捉 NG");
  }

  //リリース NFC
  delay(TEST_INTERVAL_MS);

  debugPrintMsg("リリースNFC");
  rcs660sAppIf.releaseNfc();

  debugPrintMsg("Type A テスト終了 続いて Type B テスト");
  uart_wait_ms(TEST_LOOP_INTERVAL_MS);

  /* TEST 4 NFC Type-B 捕捉(無限リトライ)～通信 *******************************/

  debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★");
  debugPrintMsg("★★★★★★★★★★ 従来型免許証 をタッチ ★★★★★★★★★★");
  debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★\n");
  
  uart_wait_ms(TEST_WAIT_HUMAN_READABLE_INTERVAL_MS);

  //従来型免許証通信用に設定
  rcs660sAppIf.setNfcType(NFC_TYPE_B);
  rcs660sAppIf.updateTxAndRxFlag({false, false, 3, false});
  
  //NFC-TypeBを捕捉　リトライ回数無限
  isCatch = E_NG;
  isCatch = rcs660sAppIf.catchNfc(RETRY_CATCH_INFINITE);

  if(isCatch == E_OK){
    debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★");
    debugPrintMsg("★★★★★★★★★ 従来型免許証と通信開始 ★★★★★★★★★");
    debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★\n");
    
    debugPrintMsg("★タイムアウトをテストする場合は免許証を離す★");

    /********************************************************/
    
    uart_wait_ms(TEST_WAIT_HUMAN_READABLE_INTERVAL_MS);
    if (nfc_res.empty() == false)
    {
      nfc_res.clear();
    }

    /********************************************************************/

    debugPrintMsg("★ MFを選択 Case0 タイムアウトなし ★");
    uart_wait_ms(TEST_WAIT_HUMAN_READABLE_INTERVAL_MS);

    const std::vector<uint8_t> selectMfCase0 = { 0x00,0xA4,0x00,0x00 };

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
    debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★");
    debugPrintMsg("★★★★★★★★★ 従来型免許証と通信終了 ★★★★★★★★★");
    debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★\n");
    uart_wait_ms(TEST_WAIT_HUMAN_READABLE_INTERVAL_MS);
  }

  delay(TEST_INTERVAL_MS);
  
  debugPrintMsg("\n★電流計に注目★\n");
  
  delay(TEST_INTERVAL_MS);

  debugPrintMsg("リリースNFC");
  rcs660sAppIf.releaseNfc();

  delay(TEST_WAIT_HUMAN_READABLE_INTERVAL_MS);

  /* TEST 5 パワーダウン・ウェイクアップ ***********************************/

  debugPrintMsg("パワーダウン");
  rcs660sAppIf.powerDown();

  delay(TEST_WAIT_HUMAN_READABLE_INTERVAL_MS);
  
  debugPrintMsg("ウェイクアップ");
  rcs660sAppIf.wakeup();

  

debugPrintMsg("◆◆◆ テストループ終了 ◆◆◆");
delay(TEST_LOOP_INTERVAL_MS);
}

