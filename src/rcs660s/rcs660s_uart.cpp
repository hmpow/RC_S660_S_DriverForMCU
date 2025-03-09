#include "rcs660s_uart.h"

/*****************/
/* グローバル変数 */
/*****************/


//マニュアル Ver1.0 p.10 フレーム内固定値部
const uint8_t PRE_AMBLE = 0x00;
const uint8_t STATUS_CODE[2] = {0x00,0xFF};
const uint8_t POST_AMBLE = 0x00;

//マニュアル Ver1.0 p.14 ACKフレーム
const uint8_t FULL_COMMAND_ACK[] = {0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00};

//重要　const系はヘッダファイルに置いちゃダメ！ include先に実体がいっぱいできてしまう by chatGPT

enum RECEIVE_STATE{
  RECEIVE_READY,
  RECEIVE_ACK,
  RECEIVE_DATA,
  RECEIVE_COMPLETE
};

uint8_t  receiveAllUartFrameArr[RECEIVE_DATA_BUFF_SIZE];
uint8_t  receiveAllUartFrameLen;
uint16_t receiveDataLen;
enum RECEIVE_STATE receiveState;


//チェックサムなどの付加情報を付与しフルコマンド化
//フルコマンド = 0x00 0x00 0xff LEN LEN LCS [DATA PACKET] DCS 0x00
void assemblyRcs660sUartCommandFrame(const uint8_t wired_packet_data[], const uint16_t wired_packet_data_len){

    //長すぎたらエラー
    if(wired_packet_data_len > (uint16_t)RCS660S_UART_MAX_RES_LEN){
        debugPrintMsg("ERROR! assemblyRcs660sUartCommandFrame パケットが長すぎます\n");
        exit(1);
    }
    
    //定数(マニュアル指定値)
    const uint8_t  preamble  = 0x00;
    const uint16_t startCode = 0x00ff;
    const uint8_t  postamble = 0x00;
    
    //定数(内部)
    const uint8_t START_OFFSET = 6;
    
    //変数たち
    uint16_t LEN = 0x0000;    //データの長さ　RCS-660/Sでは2バイトになった
    uint8_t  LCS = 0x00;      //LENのチェックサム
    uint8_t  DCS = 0x00;      //データのチェックサム
    uint8_t  *arrCommadFrame = NULL; //出力コマンドフレーム
    uint32_t data_sum = 0;    //データパケット合計 チェックサム計算用
    
    
    //LCS計算
    LEN = wired_packet_data_len;   // 送信データの長さ
    
    uint8_t LEN_upperByte = (uint8_t)(LEN >> 8); //上位ビット取出し　全部uintなのでビットマスクしなくても算術シフトされて不要な1が立つ恐れない
    uint8_t LEN_lowerByte = (uint8_t)(0x00FF & LEN); //下位ビット取出し

    LCS = (uint8_t)(0x0100 - (0x00FF & (LEN_upperByte + LEN_lowerByte))); // (LEN上位バイト＋LEN下位バイト+LCS) の下位1バイトが0x00になること

    //card_command＋リーダライタコマンド分のメモリを確保
    arrCommadFrame = (uint8_t*)malloc(sizeof(uint8_t) * (LEN + 8));
	if (arrCommadFrame == NULL) {
		debugPrintMsg("ERROR! assemblyRcs660sUartCommandFrame メモリ確保失敗\n");
		exit(1);
	}
    
    
    //カードリーダのコマンドを送信データに結合
    arrCommadFrame[0] = preamble;   //Preamble
    arrCommadFrame[1] = (uint8_t)(startCode >> 8);      //StartCode上位バイト
    arrCommadFrame[2] = (uint8_t)(0x00FF & startCode);  //StartCode下位バイト
    arrCommadFrame[3] = LEN_upperByte;   //LEN上位バイト
    arrCommadFrame[4] = LEN_lowerByte;   //LEN下位バイト
    arrCommadFrame[5] = LCS;   //LCS
    
    //パケットデータ配列に流し込みつつ合計
    uint16_t i = 0;
    for(i = 0; i < LEN; i++){
        arrCommadFrame[i + START_OFFSET] = wired_packet_data[i]; //DATA
        data_sum = data_sum + wired_packet_data[i];   //DCS計算用
    }

    //DCSを計算
    DCS = (uint8_t)(0x100 - (data_sum % 0x100));

    arrCommadFrame[START_OFFSET + LEN]     = DCS;
    arrCommadFrame[START_OFFSET + LEN + 1] = postamble;

    //コマンドをカードリーダに送信
    //LENは1始まりなので1足すこと！
    uart_hw_sendUart(arrCommadFrame, START_OFFSET + LEN + 1 + 1);

    free(arrCommadFrame);
}

/**
 * @brief ACKを送信
 */

void uart_sendAck(void) {
  uart_hw_sendUart(FULL_COMMAND_ACK, (uint16_t)(sizeof(FULL_COMMAND_ACK)/sizeof(FULL_COMMAND_ACK[0])));
  return;
}

/**
 * @brief 受信処理初期化
 * 
 */
void uart_receiver_init(void){
  //受信処理初期化

  //Serialの受信バッファ空読み
  while(uart_hw_available()){
    (void)UART_RCS660S.read();
  }

  //内部変数初期化
  for(int i = 0; i < RECEIVE_DATA_BUFF_SIZE; i++){
    receiveAllUartFrameArr[i] = 0;
  }
  receiveAllUartFrameLen = 0;
  receiveDataLen = 0;

  //状態遷移初期化
  receiveState = RECEIVE_READY;
}

/**
 * @brief ACKが正常・既定時間内かチェック ACK分バッファが進むので注意
 * 
 * @return true ACK受信OK
 * @return false ACK受信NG
 */
bool uart_receiver_checkACK(void){
  receiveState = RECEIVE_ACK;
  const unsigned long START_TIME = millis();

#ifdef UART_LAYER_DEBUG
  debugPrintMsg("ACK START_TIME = ");
  debugPrintDec(START_TIME);
#endif

  const uint8_t ACK_DATA_LEN = sizeof(FULL_COMMAND_ACK)/sizeof(FULL_COMMAND_ACK[0]); //ACKデータ長

  uint8_t rx_counter = 0;
  uint8_t rx_data = 0;

  while(true)
  {
    if (uart_hw_available() > 0) {
      //データがある場合：受信してチェック
      rx_data = UART_RCS660S.read();
#if 0
#ifdef UART_LAYER_DEBUG
      UART_PC.print(rx_data, HEX);
      UART_PC.print(" ");
#endif
#endif
      if(rx_data == FULL_COMMAND_ACK[rx_counter]){
        //OKの場合
        if(rx_counter == ACK_DATA_LEN - 1){
          //ACK受信完了
          #ifdef UART_LAYER_DEBUG
            debugPrintMsg("ACK CHECK OK");
          #endif

          return true;
        }else{
          //次のデータへ
          rx_counter++;
        }
      }else{
        //ACKデータが正しくない場合
        debugPrintMsg("ERROR! ACK CHECK NG : WRONG PACKET");
        return false;
      }
    }else{
      //データがない場合：タイムアウト時間内ならループ継続
      if(millis() - START_TIME > (unsigned long)RECEIVE_ACK_TIMEOUT){
        break;
      }
    }
  }

  //while抜けてきたらACK受信タイムアウト
  debugPrintMsg("ERROR! ACK CHECK NG : TIMEOUT");
  return false;
}

/**
 * @brief ACKが正常・既定時間内かチェック ACK分バッファが進むので注意
 * 
 * @return true 受信成功
 * @return false 受信失敗
 */
bool uart_receiver_receiveData(uint8_t receivePacketDataArr[], uint16_t *receivePacketDataLen){
  
  receiveState = RECEIVE_DATA;
  unsigned long START_TIME = millis();

  //defineで足し算が展開されるのでwhileの中でそのまま使いたくない
  const unsigned RxArrMaxSize = RECEIVE_DATA_BUFF_SIZE;

  uint16_t len_lcs_sum = 0;
  uint16_t data_sum_lower = 0;
  uint16_t data_dcs_sum = 0;

#ifdef UART_LAYER_DEBUG
  debugPrintMsg("DATA START_TIME = ");
  debugPrintDec(START_TIME);
#endif

  //データ受信
  uint8_t rx_counter = 0;

  while(true){
    if (uart_hw_available() > 0){
      if(RxArrMaxSize < rx_counter){
        debugPrintMsg("ERROR : BUFFER OVER FLOW");
        return false;
      }
      receiveAllUartFrameArr[rx_counter] = UART_RCS660S.read();

      #ifdef UART_LAYER_DEBUG
        debugPrintHex(receiveAllUartFrameArr[rx_counter]);
      #endif

      if(rx_counter == 0){
        //プリアンブルチェック
        if(receiveAllUartFrameArr[rx_counter] != PRE_AMBLE){
          //プリアンブルが正しくない場合
          debugPrintMsg("DATA CHECK NG : WRONG PRE_AMBLE");
          return false;
        }
        #ifdef UART_LAYER_DEBUG
          debugPrintMsg("PRE_AMBLE OK");
        #endif
      }
      else if(rx_counter == 1 || rx_counter == 2){
        //ステータスコードチェック
        if(receiveAllUartFrameArr[rx_counter] != STATUS_CODE[rx_counter - 1]){
          //ステータスコードが正しくない場合
          debugPrintMsg("DATA CHECK NG : WRONG STATUS_CODE");
          return false;
        }
        #ifdef UART_LAYER_DEBUG
          debugPrintMsg("STATUS_CODE OK");
        #endif
      }
      else if(rx_counter == 3 || rx_counter == 4){
        //特殊処理なし(LEN受信)
      }
      else if(rx_counter == 5){
        //LCSチェック
        //LEN上位バイト + LEN下位バイト + LCS の下位バイトが0x00であること
        
        len_lcs_sum = receiveAllUartFrameArr[3] + receiveAllUartFrameArr[4] + receiveAllUartFrameArr[5];

        if((uint8_t)(len_lcs_sum & 0x00FF) != (uint8_t)0x00){
          //LEN,LCSが正しくない場合
          debugPrintMsg("DATA CHECK NG : WRONG LEN,LCS");
          return false;
        }
        #ifdef UART_LAYER_DEBUG
          debugPrintMsg("LCS OK  Received_LEN(decimal) =");
        #endif

        //OKなら16bitにまとめてLEN格納
        receiveDataLen = (receiveAllUartFrameArr[3] << 8) + receiveAllUartFrameArr[4];
        #ifdef UART_LAYER_DEBUG
          debugPrintDec(receiveDataLen);
        #endif
      }
      else if(6 <= rx_counter && rx_counter < (6 + receiveDataLen)){
        //データ受信
        //チェックサム計算
        data_sum_lower = (data_sum_lower + receiveAllUartFrameArr[rx_counter]) & 0xFF;
      }
      else if(rx_counter == (6 + receiveDataLen)){
        //DCSチェック
        //データの合計 + DCS の下位バイトが0x00であること

        data_dcs_sum = data_sum_lower + receiveAllUartFrameArr[rx_counter];

        if((uint8_t)(data_dcs_sum & 0x00FF) != (uint8_t)0x00){
          //DATA,DCSが正しくない場合
          debugPrintMsg("DATA CHECK NG : WRONG DATA,DCS");
          return false;
        }
        #ifdef UART_LAYER_DEBUG
          debugPrintMsg("DCS OK");
        #endif
      }
      else if(rx_counter == (6 + receiveDataLen + 1)){
        //ポストアンブルチェック
        if(receiveAllUartFrameArr[rx_counter] != POST_AMBLE){
          //プリアンブルが正しくない場合
          debugPrintMsg("DATA CHECK NG : WRONG POST_AMBLE");
          return false;
        }
        #ifdef UART_LAYER_DEBUG
          debugPrintMsg("POST_AMBLE OK");
        #endif

        break;
        /************* 受信完了 *************/
      }
      else{
        //R.F.U.
      }

      rx_counter++;

    }
    else{
      //データがない場合：タイムアウト時間内ならループ継続
      if(millis() - START_TIME > (unsigned long)RECEIVE_DATA_TIMEOUT){
          debugPrintMsg("ERROR! DATA RECEIVE TIME OUT");
          return false;
      }
    }
  }

  //受信結果からデータ部のみ格納
  
  *receivePacketDataLen = receiveDataLen;

  for(uint16_t i = 0; i < receiveDataLen; i++){
    receivePacketDataArr[i] = receiveAllUartFrameArr[i + 6];
  }
  
  receiveState = RECEIVE_COMPLETE;
  return true;
}