#include "rcs660s_app_if.h";

//基本　レシーバ初期化→コマンド送信→ACK受信→コマンド受信

/*****************/
/* Public Method */
/*****************/

//コンストラクタ
Rcs660sAppIf::Rcs660sAppIf(){
    nfc_type = NFC_TYPE_UNSET;
    reader_state = READER_READY;
    is_tx_and_rx_flag_updated = false;
}

//デストラクタ
Rcs660sAppIf::~Rcs660sAppIf(){
    releaseNfc();

    if(latest_nfc_res.empty() == false){
        latest_nfc_res.clear();
    }

    return;
}

//setup関数で呼ぶ　コンストラクタと一体化するとタイミング制御できないため独立関数で定義
void Rcs660sAppIf::begin(void){
    setupSerial();
    return;
}


void Rcs660sAppIf::setNfcType(const NFC_TYPE nfcType){
    if(getReaderState() != READER_READY || getReaderState() != READER_SLEEP){
        resetDevice();
    }
    nfc_type = nfcType;
    return;
}

NFC_TYPE Rcs660sAppIf::getNfcType(void){
    return nfc_type;
}

void Rcs660sAppIf::updateTxAndRxFlag(TX_AND_RX_FLAG txAndRxFlag){
    tx_and_rx_flag = txAndRxFlag;
    is_tx_and_rx_flag_updated = true;
    return;
}


//ここ無限ループさせてはいけない　start多重呼び出しになる！！！
bool Rcs660sAppIf::catchNfc(uint8_t retryCountSetting){

    //アプリがカードと通信できる状態にする
    //StartTransParetnSession → SwitchProtocolループ → キャッチで来たらTracnsmissionAndReceptionFlag → TurnOnOf

    //事前チェック

    if(getNfcType() == NFC_TYPE_UNSET){
        debugPrintMsg("Rcs660sAppIf::catchNfc::ERROR! NFCタイプ未指定");
        return E_NG;
    }

    if(getReaderState() == READER_SLEEP){
        debugPrintMsg("Rcs660sAppIf::catchNfc::WARNING! スリープ中 起こす");
        wakeup();
        uart_wait_ms(BETWEEN_COMMANDS_INTERVAL_MS);
    }else if(getReaderState() != READER_READY){
        debugPrintMsg("Rcs660sAppIf::catchNfc::WARNING! READY状態でない リセット実行後リトライ");
        resetDevice();
        uart_wait_ms(BETWEEN_COMMANDS_INTERVAL_MS);
        catchNfc(retryCountSetting); //注意：再帰呼び出し使用
    }

    //本体処理

    bool rxStatus = E_NG;

    //StartTransParetnSession 実行

    uart_receiver_init();
    assemblyAPDUcommand_ManageSession_StartTransparentSession();
    rxStatus = receiveSequence(TEST_RX_MODE_W_TLV);
    
    if(rxStatus == E_NG){
        debugPrintMsg("Rcs660sAppIf::releaseNfc::ERROR! StartTransParetnSession失敗 リセット実行後リトライ");
        resetDevice();
        uart_wait_ms(BETWEEN_COMMANDS_INTERVAL_MS);
        catchNfc(retryCountSetting); //注意：再帰呼び出し使用
    }


    //本体

    uint8_t tryCounter = 0;

    do{
        if(tryCounter > 0){
            debugPrintMsg("Rcs660sAppIf::catchNfc::リトライ中");
            uart_wait_ms(CATCH_RETRY_INTERVAL_MS);
        }
       
        tryCounter++;
        rxStatus = E_NG;
        uart_receiver_init();

        switch (nfc_type){
            case NFC_TYPE_A:
                assemblyAPDUcommand_SwitchProtocol_TypeA_AutoActivate();
                rxStatus = receiveSequence(TEST_RX_MODE_W_TLV_AND_ATR_TYPE_A);
                break;
            case NFC_TYPE_B:
                assemblyAPDUcommand_SwitchProtocol_TypeB_AutoActivate();
                rxStatus = receiveSequence(TEST_RX_MODE_W_TLV_AND_ATR_TYPE_B);
                break; 
            case NFC_TYPE_V:
                //未実装
                break;
            case NFC_TYPE_FELICA:
                //未実装
                break;
            default:
                break;
        }

        if(retryCountSetting == RETRY_CATCH_INFINITE){
            tryCounter = 0;
        }
        
    }while(rxStatus == E_NG && tryCounter < retryCountSetting);

    if(rxStatus == E_NG){
        debugPrintMsg("Rcs660sAppIf::catchNfc::ERROR! カードキャッチ失敗");

        //RFを切ってEndTransparentSession
        releaseNfc();
        return E_NG;
    }


    if(is_tx_and_rx_flag_updated){
        //RC-S660/Sのデフォルトから変えたい場合のみ実行で良い

        debugPrintMsg("Rcs660sAppIf::catchNfc::debug! TransmissionAndReceptionFlagを更新↓");
        debugPrintHex(tx_and_rx_flag.txDoNotAppendCRC);
        debugPrintHex(tx_and_rx_flag.rxDoNotDiscardCRC);
        debugPrintHex(tx_and_rx_flag.transceiveParity);
        debugPrintHex(tx_and_rx_flag.doNotAppendOrDiscardProcolProloge);
        debugPrintMsg("Rcs660sAppIf::catchNfc::debug! TransmissionAndReceptionFlagを更新↑");
    
        uart_wait_ms(BETWEEN_COMMANDS_INTERVAL_MS);
        rxStatus = E_NG;
        uart_receiver_init();
        assemblyAPDUcommand_TransparentExchange_TransmissionAndReceptionFlag(
            tx_and_rx_flag.txDoNotAppendCRC,
            tx_and_rx_flag.rxDoNotDiscardCRC,
            tx_and_rx_flag.transceiveParity,
            tx_and_rx_flag.doNotAppendOrDiscardProcolProloge);
    
        rxStatus = receiveSequence(TEST_RX_MODE_W_TLV);
        if(rxStatus == E_NG){
            debugPrintMsg("Rcs660sAppIf::catchNfc::WARNING! TransmissionAndReceptionFlag失敗 デフォルト動作します");
        }
    }else{
        debugPrintMsg("Rcs660sAppIf::catchNfc::debug! TransmissionAndReceptionFlagは デフォルト動作します");
    }

    //TrunOnRfField実行
    uart_wait_ms(BETWEEN_COMMANDS_INTERVAL_MS);
    rxStatus = E_NG;
    uart_receiver_init();
    assemblyAPDUcommand_ManageSession_TrunOnRfField();
    rxStatus = receiveSequence(TEST_RX_MODE_W_TLV);
    if (rxStatus == E_NG)
    {
        debugPrintMsg("Rcs660sAppIf::catchNfc::WARNING! TrunOnRfField失敗 SwProtocol時～RF継続のはずなので継続");
    }
    
    setReaderState(READER_COMMUNICATE);

    return E_OK;
}

std::vector<uint8_t> Rcs660sAppIf::communicateNfc(const std::vector<uint8_t> txData, const uint16_t timeout_ms){
    
    const std::vector<uint8_t> emptyRxData;

    unsigned int txDataLen = txData.size();

    //事前チェック

    if(txData.empty() || txDataLen == 0){
        debugPrintMsg("Rcs660sAppIf::communicateNfc::ERROR! 送信データなし");
        return emptyRxData;
    }


    if(getReaderState() != READER_COMMUNICATE){
        debugPrintMsg("Rcs660sAppIf::communicateNfc::ERROR! 通信状態でない");
        return emptyRxData;
    }

    //Vector→配列

    uint8_t txDataArr[txDataLen];
    for (size_t i = 0; i < txDataLen; i++)
    {
        txDataArr[i] = txData[i];
    }
    
    //受信履歴クリア
    if(latest_nfc_res.empty() == false){
        latest_nfc_res.clear();
    }

    //TransparentExchange 実行

    uart_receiver_init();
    assemblyAPDUcommand_TransparentExchange_Transceive(txDataArr, txDataLen, timeout_ms);

    bool rxStatus = E_NG;
    rxStatus = receiveSequence(TEST_RX_MODE_W_TLV_AND_CARD_RES);

    
    if(rxStatus == E_NG){
        debugPrintMsg("Rcs660sAppIf::communicateNfc::ERROR! TransparentExchange失敗");
        return emptyRxData; //空のVector
    }

    return getLatestNfcRes();
}

std::vector<uint8_t> Rcs660sAppIf::getLatestNfcRes(void){
    return latest_nfc_res;
}

void Rcs660sAppIf::releaseNfc(void){

    if(getReaderState() != READER_COMMUNICATE){
        debugPrintMsg("Rcs660sAppIf::releaseNfc::WARNING! 通信状態でないため処置不要");
        return;
    }

    bool rxStatus = E_NG;

    //TrunOffRfField 実行

    uart_receiver_init();
    assemblyAPDUcommand_ManageSession_TrunOffRfField();
    rxStatus = receiveSequence(TEST_RX_MODE_W_TLV);
    
    if(rxStatus == E_NG){
        debugPrintMsg("Rcs660sAppIf::releaseNfc::ERROR! TrunOffRfField失敗 リセット実行");
        resetDevice();
        return;
    }

    uart_wait_ms(BETWEEN_COMMANDS_INTERVAL_MS);

    //End Transparent Session 実行

    uart_receiver_init();
    assemblyAPDUcommand_ManageSession_EndTransparentSession();
    rxStatus = receiveSequence(TEST_RX_MODE_W_TLV);
    
    if(rxStatus == E_NG){
        debugPrintMsg("Rcs660sAppIf::releaseNfc::ERROR! EndTransparentSession失敗 リセット実行");
        resetDevice();
        return;
    }

    setReaderState(READER_READY);

    return;
}


void Rcs660sAppIf::sleep(void){
    //実装後回し
    setReaderState(READER_SLEEP);
    return;
}

void Rcs660sAppIf::wakeup(void){
    //実装後回し
    setReaderState(READER_READY);
    return;
}

void Rcs660sAppIf::resetDevice(void){
    executeResetDeviceSequence();
#ifdef TEST_MODE   
    //resetDeviceのACKに対するACNKは来ないのでACKタイムアウトで待機
    (void)receiveSequence(TEST_RX_MODE_WO_TLV);
    uart_receiver_init();
#endif
    setReaderState(READER_READY);
    return;
}

/******************/
/* Private Method */
/******************/


void Rcs660sAppIf::setReaderState(const READER_STATE rState){
    reader_state = rState;
    return;
}

READER_STATE Rcs660sAppIf::getReaderState(void){
    return reader_state;
}


//これが受信のひな型
bool Rcs660sAppIf::receiveSequence(TEST_RX_MODE mode){

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
    return E_NG;
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
      return E_NG;
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
        return E_NG;
      }

      // 4. 3 がOKなら TLV セットを解析
      if(mode == TEST_RX_MODE_W_TLV_AND_CARD_RES){

        if(latest_nfc_res.empty() == false){
            latest_nfc_res.clear();
        }

        debugPrintMsg("getCardResponse_from_TransparentExchangeResponse実行");
        latest_nfc_res = getCardResponse_from_TransparentExchangeResponse(apduDataObj);


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
  return E_OK;
}