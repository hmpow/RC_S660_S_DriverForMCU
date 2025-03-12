#ifndef RCS660S_APP_IF_H
#define RCS660S_APP_IF_H

//#define APP_IF_LAYER_DEBUG

/********/
/* 設定 */
/********/

#define CATCH_RETRY_INTERVAL_MS 500
#define BETWEEN_COMMANDS_INTERVAL_MS 50

//目標：カードとアプリが直接通信しているように見せられる
//AT車のようなクラス

#include <stdint.h>
#include <stdbool.h>
#include <vector>
#include "rcs660s_apdu.h"

#define E_OK (bool)true
#define E_NG (bool)false
#define RETRY_CATCH_INFINITE (uint8_t)0xFF

typedef enum _nfc_type{
    NFC_TYPE_UNSET = 0,
    NFC_TYPE_A,
    NFC_TYPE_B,
    NFC_TYPE_V,
    NFC_TYPE_FELICA
}NFC_TYPE;

typedef struct _tx_and_rx_flag{
    bool txDoNotAppendCRC;
    bool rxDoNotDiscardCRC;
    uint8_t transceiveParity;
    bool doNotAppendOrDiscardProcolProloge;
}TX_AND_RX_FLAG;

//このクラスはapdu以下のシーケンスを隠ぺいして使いやすいI/F提供するAT車的な関数
typedef enum _reader_state{
    READER_UNINITIALIZED,
    READER_READY,        //電源ON RF-OFF  "N"で停車中相当
    READER_WAITING_CARD, //カード捕捉待ち  "D"で停車中相当
    READER_COMMUNICATE,  //カード捕捉中    "D"で走行中相当
    READER_SLEEP         //電源OFF RF-OFF "P"で駐車中相当
}READER_STATE;


typedef enum _test_rx_mode{
    TEST_RX_MODE_WO_TLV = 0,
    TEST_RX_MODE_W_TLV,
    TEST_RX_MODE_W_TLV_AND_ATR_TYPE_A,
    TEST_RX_MODE_W_TLV_AND_ATR_TYPE_B,
    TEST_RX_MODE_W_TLV_AND_CARD_RES,
  }TEST_RX_MODE;

class Rcs660sAppIf {
    public:
        Rcs660sAppIf();
        ~Rcs660sAppIf();

        void begin(void);

        void setNfcType(const NFC_TYPE);
        NFC_TYPE getNfcType(void);
        void updateTxAndRxFlag(TX_AND_RX_FLAG);

        bool catchNfc(uint8_t);
        std::vector<uint8_t> communicateNfc(const std::vector<uint8_t>, const uint16_t);
        std::vector<uint8_t> getLatestNfcRes(void);
        void releaseNfc(void);

        
        void sleep(void);  //実装後回し
        void wakeup(void); //実装後回し
  
        void resetDevice(void);

        bool receiveSequence(TEST_RX_MODE mode);

    private:
        NFC_TYPE nfc_type;
        READER_STATE reader_state;
        TX_AND_RX_FLAG tx_and_rx_flag; //カード依存なのでアプリから設定することにする

        std::vector<uint8_t> latest_nfc_res;

        bool is_tx_and_rx_flag_updated;

        void setReaderState(const READER_STATE);
        READER_STATE getReaderState(void);

};

#endif // RCS660S_APP_IF_H