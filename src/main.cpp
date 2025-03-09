#include <Arduino.h>
#include "./rcs660s/rcs660s_app_if.h"
#include <vector>

#define TEST_INTERVAL_MS 1000
#define TEST_LOOP_INTERVAL_MS 30000
#define TEST_TOUCH_WAIT_INTERVAL_MS TEST_INTERVAL_MS
#define ERROR_OCCURED true
#define ERROR_NOT_OCCURED false



//マニュアル指定定数
const uint8_t FULL_COMMAND_GetFirmWareVersion[] = {0x00, 0x00, 0xFF, 0x00, 0x0E, 0xF2, 0x6B, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x56, 0x00, 0x00, 0x3C, 0x00};

Rcs660sAppIf rcs660sAppIf;

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
  debugPrintMsg("◆ レシーバ初期化 ◆");
  uart_receiver_init();
  delay(TEST_INTERVAL_MS);

  //rcs660 に GetFirmwareVersionを送信
  debugPrintMsg("【コマンド実行】GetFirmwareVersion");
  assemblyAPDUcommand_GetFirmwareVersion();
  (void)rcs660sAppIf.receiveSequence(TEST_RX_MODE_WO_TLV);

  /*********************************************************************/

  /*********************************************************************/

  /*********************************************************************/
  rcs660sAppIf.setNfcType(NFC_TYPE_B);
  rcs660sAppIf.updateTxAndRxFlag({false, false, 3, false});
  rcs660sAppIf.catchNfc(RETRY_CATCH_INFINITE);

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
  assemblyAPDUcommand_TransparentExchange_Transceive(data, 4, 0);

  (void)rcs660sAppIf.receiveSequence(TEST_RX_MODE_W_TLV_AND_CARD_RES);
  /********************************************************/
  debugPrintMsg("◆ レシーバ初期化 ◆");
  uart_receiver_init();
  delay(TEST_INTERVAL_MS);

  debugPrintMsg("WirelessCommand = {00 A4 00 00}, WirelessCommand_Len = 4,  timeout_ms = 60\n");
  assemblyAPDUcommand_TransparentExchange_Transceive(data, 4, 60);

  (void)rcs660sAppIf.receiveSequence(TEST_RX_MODE_W_TLV_AND_CARD_RES);
  /********************************************************/
  debugPrintMsg("◆ レシーバ初期化 ◆");
  uart_receiver_init();
  delay(TEST_INTERVAL_MS);

  debugPrintMsg("\n****** SELECT MF CASE 3 , P1 = 00 , P2 = 00  *****\n\n");
  const uint8_t data_2[] = {0x00,0xA4,0x00,0x00,0x02,0x3F,0x00};
  debugPrintMsg("WirelessCommand = {00,A4,00,00,02,3F,00}, WirelessCommand_Len = 7,  timeout_ms = 0\n");
  assemblyAPDUcommand_TransparentExchange_Transceive(data_2, 7, 0);

  (void)rcs660sAppIf.receiveSequence(TEST_RX_MODE_W_TLV_AND_CARD_RES);
  /********************************************************/

  /*********************************************************************/
  debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★");
  debugPrintMsg("★★★★★★★★★ カードと通信終了 ★★★★★★★★★");
  debugPrintMsg("★★★★★★★★★★★★★★★★★★★★★★★★★★★");

  /*********************************************************************/
  /*********************************************************************/
  /*********************************************************************/

  delay(TEST_INTERVAL_MS);
  rcs660sAppIf.releaseNfc();

debugPrintMsg("◆◆◆ テストループ終了 ◆◆◆");
delay(TEST_LOOP_INTERVAL_MS);
}

