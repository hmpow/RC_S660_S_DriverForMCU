//とりあえずNFC_TypeBしか使わないので他は無視

#ifndef RCS660S_APDU_H
#define RCS660S_APDU_H

//#define APDU_LAYER_DEBUG

#include <stdint.h>
#include <stdbool.h>

#include "rcs660s_util.h"
#include "rcs660s_ccid.h"
#include "rcs660s_uart.h"

/*****************/
/* 型や定数の宣言 */
/*****************/

//APDUコマンドに関するもの
#define LC_NO_DATA   0x00 //データなしコマンドの場合のLc
#define LC_MAX       0xFF //データありコマンドの場合のLc最大値　規格上の最大値 0xFF 配列長を兼ねるのでメモリが足らない場合チューニング対象
#define LE_NO_USE    0x00 //Le機能を使用しない場合は省略ではなく明示的に 0x00 とする
#define TIMER_NO_USE 0 //Le最大値

#define DATA_OBJECT_TAG_SIZE       2 //DataObjectのTagのバイト数
#define DATA_OBJECT_LENGTH_SIZE    1 //DataObjectのLengthのバイト数
#define DATA_OBJECT_VALUE_MAX_SIZE (LC_MAX - DATA_OBJECT_TAG_SIZE - DATA_OBJECT_LENGTH_SIZE)//DataObjectのTagのバイト数

#define COMMAND_HEADER_SIZE     4 //Leのバイト数"
#define COMMAND_LC_SIZE         1 //Lcのバイト数"
#define COMMAND_LE_SIZE         1 //Leのバイト数"

#define COMMAND_DATA_IN_NO_OFFSET   0 //DataInのオフセットがない


#define TAG_OF_ERROR_STATUS_TLV 0xC0 //エラーステータス用TLVのタグ 4.4章
#define TWO_BYTE_TAG_FLAG_1 0xFF //2バイトタグの識別 4.8章、4.9章、4.10章から
#define TWO_BYTE_TAG_FLAG_2 0x5F //2バイトタグの識別 4.8章、4.9章、4.10章から

#define GEENERIC_ERROR_STATUS_TAG 0xC0; //汎用エラーステータスのタグ

#define APDU_ERROR_STATUS_LENGTH 2


#define MUST_WAIT_AFTER_POWER_DOWN 20 //Power Downの後に待つ時間 > 10ms (マニュアル 4.13章)
#define MUST_WAIT_AFTER_WAUEKUP    30 //WakeUpの後に待つ時間 > 20ms (マニュアル 4.13章)
#define WAIT_AFTER_RESET           MUST_WAIT_AFTER_WAUEKUP //Resetの後に待つ時間 マニュアルしていないがウェイクアップと同じにしておく



//APDUコマンドの構造体
typedef struct _apdu_command{
    uint8_t CLA;
    uint8_t INS;
    uint8_t P1;
    uint8_t P2;
    uint8_t Lc;
    uint8_t DataIn[LC_MAX]; //APDUのLcはFFまでなので256バイトまでで十分
    uint8_t Le;
}APDU_COMMAND;

//APUDコマンドのエラーステータス　戻り値で使いたいので配列ではなく構造体
typedef struct _apdu_error_status{
    uint8_t sw1;
    uint8_t sw2;
}APDU_ERROR_STATUS;

//APDUコマンドのDataInに並べる小部屋
typedef struct _data_object{
    uint16_t Tag;   //2バイトタグに対応させておき8バイトタグはビットマスクで処理前提
    uint8_t  Length;
    uint8_t  Value[DATA_OBJECT_VALUE_MAX_SIZE]; //LC_MAX からTag,Lengthを引いた分だけの長さ
}APDU_DATA_OBJECT;

//APDUレスポンスのエラーステータス
typedef enum _dpdu_response_error_status{
    STATUS_OK,
    STATUS_ERROR,
    STATUS_WARNING
}APDU_RESPONSE_ERROR_STATUS;

//NFC_TypeAのATR
typedef struct _nfc_type_a_atr{
    uint8_t initialHeader;
    uint8_t t0;
    uint8_t td1;
    uint8_t td2;
    std::vector<uint8_t> ATS_HistoricalBytes;
    uint8_t tck;
}NFC_TYPE_A_ATR;

//NFC_TypeBのATR
typedef struct _nfc_type_b_atr{
    uint8_t initialHeader;
    uint8_t t0;
    uint8_t td1;
    uint8_t td2;
    uint8_t atpbAppData[4];
    uint8_t atpbProtocolInfo[3];
    uint8_t atqbAttrib;
    uint8_t tck;
}NFC_TYPE_B_ATR;

//ToDo: APDU_DATA_OBJECTが複数あるとDataIn最大長超えるのでAPDU_COMMAND組み立て側でチェックする

/******************/
/* プロトタイプ宣言 */
/******************/

void initNfcType(void);
void setNfcTypeA(void);
void setNfcTypeB(void);
void setNfcTypeV(void);
void setNfcTypeFeliCa(void);

/************************************************************************************/
/*************************************** 送信 ***************************************/
/************************************************************************************/

//public

//apduコマンドを配列にしてCCID層にパスする ret:SeqNo
uint8_t passToCcidLayer(APDU_COMMAND);

//4.6章 LoadKeys
//Type-A/Bでは使わないため実装しない

//4.7章 GeneralAuthenticate
//Type-A/Bでは使わないため実装しない

//4.8章 Manage Session
void assemblyAPDUcommand_ManageSession_StartTransparentSession(void);
void assemblyAPDUcommand_ManageSession_EndTransparentSession(void);
void assemblyAPDUcommand_ManageSession_TrunOffRfField(void);
void assemblyAPDUcommand_ManageSession_TrunOnRfField(void);

//使わないため実装しない
//void assemblyAPDUcommand_ManageSession_VersionDataObject(void);
//void assemblyAPDUcommand_ManageSession_Timer(void);
//void assemblyAPDUcommand_ManageSession_GetParameters(void);

//使わないため実装しない(Type-Bはデフォルトで動作した)
//void assemblyAPDUcommand_ManageSession_SetParameters(APDU_DATA_OBJECT[]);


//4.9章 Transparent Exchange

//Transmission and Reception Flagコマンド組み立て(TxCRC付加,RxCRC除去,送受信パリティ,ProtocolProloge自動処理)
void assemblyAPDUcommand_TransparentExchange_TransmissionAndReceptionFlag(const bool, const bool, const uint8_t, const bool);

//Transceveコマンド組み立て(無線コマンドarray,無線コマンドlen,受信タイムアウトms)
void assemblyAPDUcommand_TransparentExchange_Transceive(const uint8_t*, const uint8_t, const uint16_t);

//使わないため実装しない
//void assemblyAPDUcommand_TransparentExchange_TransmissionBitFraming(uint8_t);
//void assemblyAPDUcommand_TransparentExchange_TReceptionBitFraming(uint8_t);
//void assemblyAPDUcommand_TransparentExchange_Transmit(const uint8_t*, const uint16_t, const uint8_t);
//void assemblyAPDUcommand_TransparentExchange_Receive(void);
//void assemblyAPDUcommand_TransparentExchange_Timer(const uint8_t); //単体では使用しない
//void assemblyAPDUcommand_TransparentExchange_GetParameters(void);
//void assemblyAPDUcommand_TransparentExchange_SetParameters(APDU_DATA_OBJECT[]);

//4.10章 Switch Protocol
void assemblyAPDUcommand_SwitchProtocol_TypeA_AutoActivate(void);
void assemblyAPDUcommand_SwitchProtocol_TypeB_AutoActivate(void);

//使わないため実装しない
//void assemblyAPDUcommand_SwitchProtocol_TypeV_AutoInventory(void);
//void assemblyAPDUcommand_SwitchProtocol_FeliCa_AutoPolling(void);


//4.11章 Reset Device
//このファイルはAPDU階層の話なのでResetDeviceのACK送信はアプリ層側で処理する
//と思っていたがAPDUコマンドの仕様なのでAPDU層にexecuteを実装する
void assemblyAPDUcommand_ResetDevice(void);
void executeResetDeviceSequence(void);

//4.12章 Get Firmware Version
void assemblyAPDUcommand_GetFirmwareVersion(void);

//private
//LoadKeys はType-A/Bでは使わないため実装しない
//GeneralAuthenticate はType-A/Bでは使わないため実装しない

/********/
/* BASE */
/********/

void _assemblyAPDUcommand_ManageSession_Base(const APDU_DATA_OBJECT);
void _assemblyAPDUcommand_TransparentExchange_Base(const APDU_DATA_OBJECT, const uint16_t);
void _assemblyAPDUcommand_SwitchProtocol_Base(const APDU_DATA_OBJECT);

/************************************************************************************/
/*************************************** 受信 ***************************************/
/************************************************************************************/

// 手順
// 1. APDUレイヤにエラーがないか確認
APDU_RESPONSE_ERROR_STATUS checkAPDU_response_ErrStatus(const std::vector<uint8_t>);

//4.4章 APDUエラーステータス
APDU_RESPONSE_ERROR_STATUS checkAPDU_sw1sw2_ErrStatus(const APDU_ERROR_STATUS);

// ManageSession・TransparentExchange・SwitchProtocol の場合のみ受信データがある

// 2. 1 が OK なら TLVのバイト列取出し
std::vector <APDU_DATA_OBJECT> parseAPDU_response_DataObjects(const std::vector<uint8_t>);

void debugPrintAPDU_response_DataObjects(const std::vector <APDU_DATA_OBJECT>);
void debugPrintAPDU_singleDataOobjectTLV(const APDU_DATA_OBJECT);

// 3. 2 から TLV セットにエラーがないか確認
APDU_RESPONSE_ERROR_STATUS checkAPDU_dataObject_ErrStatus(const std::vector <APDU_DATA_OBJECT>);

// 4. 3 がOKなら TLV セットを解析
std::vector<uint8_t> getCardResponse_from_TransparentExchangeResponse(const std::vector <APDU_DATA_OBJECT>);

NFC_TYPE_A_ATR getTypeA_ATR_from_SwitchProtocolResponse(const std::vector <APDU_DATA_OBJECT>);
NFC_TYPE_B_ATR getTypeB_ATR_from_SwitchProtocolResponse(const std::vector <APDU_DATA_OBJECT>);

/************************************************************************************/
/*************************************** 道具 ***************************************/
/************************************************************************************/


//Don't Repeat Yourself対応：BASE共通化でヘッダーをSwitchCaseではなく、Baseは3つ作り道具を関数にまとめることにする
uint8_t _checkTagSize(const uint16_t);
void _assemblyTag(APDU_COMMAND*, const APDU_DATA_OBJECT, const uint8_t);
void _assemblyValue(APDU_COMMAND*, const APDU_DATA_OBJECT, const uint8_t);

APDU_DATA_OBJECT _getTimerDataObject(const uint16_t);

#endif // RCS660S_APDU_H