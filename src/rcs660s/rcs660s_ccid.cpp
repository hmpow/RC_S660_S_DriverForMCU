#include "rcs660s_ccid.h"


/************************************************************************************/
/*************************************** 送信 ***************************************/
/************************************************************************************/

//APDUコマンドから PC_to_RDR_Escape CCID コマンドを組み立て
//コマンド = 0x68 <LEN lttleEndian 4byte> 0x00 <SeqNo> <RFU 00 00 00> [DATA PACKET]
void assemblyCCIDcommand_PC_to_RDR_Escape(const uint8_t APDU_data[], const uint32_t APDU_lenBigendian, const uint8_t SeqNo){

    //定数(マニュアル指定値)
    const uint8_t bMessageType = 0x6B;
    const uint8_t bSlot        = 0x00;
    const uint8_t RFUbyte      = 0x00;
    const uint8_t START_OFFSET = 10;

    //変数たち
    uint32_t dwLength = 0x00000000; //データの長さ
    uint8_t*  arrCCIDcommand = NULL;       //出力CCIDコマンドフレーム
    
    //長すぎたらエラー
    if(APDU_lenBigendian > 0xFFFF - START_OFFSET){
        debugPrintMsg("ERROR! assemblyCCIDcommand_PC_to_RDR_Escape データが長すぎます\n");
        exit(1);
    }


    //card_command＋リーダライタコマンド分のメモリを確保
    size_t arrSize = (size_t)(START_OFFSET + APDU_lenBigendian);
    arrCCIDcommand = (uint8_t*)malloc(sizeof(uint8_t) * arrSize);

	if (arrCCIDcommand == NULL){
		debugPrintMsg("malloc ERROR!\n");
		exit(1);
	}

    //APDUコマンドの前にCCID_PC_to_RDR_Escapeコマンド結合
    arrCCIDcommand[0] = bMessageType;
    
    //dwLength リトルエンディアンにして詰め込み
    dwLength = bigEndianToLittleEndian(APDU_lenBigendian);
    
    arrCCIDcommand[1] = (uint8_t)((0xFF000000 & dwLength) >> 24);
    arrCCIDcommand[2] = (uint8_t)((0x00FF0000 & dwLength) >> 16);
    arrCCIDcommand[3] = (uint8_t)((0x0000FF00 & dwLength) >> 8);
    arrCCIDcommand[4] = (uint8_t)(0x000000FF & dwLength);
    
    arrCCIDcommand[5] = bSlot;
    arrCCIDcommand[6] = SeqNo;
    arrCCIDcommand[7] = RFUbyte;
    arrCCIDcommand[8] = RFUbyte;
    arrCCIDcommand[9] = RFUbyte;
    
    //パケットデータ配列に流し込み
    uint32_t i = 0;
    for(i = 0; i < APDU_lenBigendian; i++){
        arrCCIDcommand[i + START_OFFSET] = APDU_data[i]; //DATA
    }

    //組み立てたCCIDコマンドをコマンドフレーム組み立てに引き継ぎ

    assemblyRcs660sUartCommandFrame(arrCCIDcommand, (uint16_t)((0xFFFF & APDU_lenBigendian) + START_OFFSET));

    free(arrCCIDcommand);
    
    return;
}


//指定シーケンスNoの PC_to_RDR_Abort コマンドを組み立て
//コマンド = 0x72 <LEN 00 00 00 00> 0x00 <SeqNo> <RFU 00 00 00>
void assemblyCCIDcommand_PC_to_RDR_Abort(uint8_t SeqNo){
    //定数(マニュアル指定値)
    const uint8_t  bMessageType = 0x72;
    const uint32_t dwLength     = 0x00000000;
    const uint8_t  bSlot        = 0x00;
    const uint8_t  abRFU_byte   = 0x00;

    //変数
    uint8_t bSeq = SeqNo;
    uint8_t arrCCIDcommand[1 + 4 + 1 + 1 + 3];

    //CCID_PC_to_RDR_Abortコマンド結合
    arrCCIDcommand[0] = bMessageType;
    arrCCIDcommand[1] = (uint8_t)((0xFF000000 & dwLength) >> 24);
    arrCCIDcommand[2] = (uint8_t)((0x00FF0000 & dwLength) >> 16);
    arrCCIDcommand[3] = (uint8_t)((0x0000FF00 & dwLength) >> 8);
    arrCCIDcommand[4] = (uint8_t)(0x000000FF & dwLength);
    arrCCIDcommand[5] = bSlot;
    arrCCIDcommand[6] = bSeq;
    arrCCIDcommand[7] = abRFU_byte;
    arrCCIDcommand[8] = abRFU_byte;
    arrCCIDcommand[9] = abRFU_byte;

     //組み立てたCCIDコマンドをコマンドフレーム組み立てに引き継ぎ
    assemblyRcs660sUartCommandFrame(arrCCIDcommand, (uint16_t)(1 + 4 + 1 + 1 + 3));

    return;
}

/************************************************************************************/
/*************************************** 受信 ***************************************/
/************************************************************************************/

//RDR_to_PC_Escape CCID コマンドを解析
std::vector<uint8_t> parseCCIDresponse_RDR_to_PC_Escape(const uint8_t* inputCCIDarr, const uint32_t inputCCIDlen){
    
    //取り出した受信データはうっかり編集しないためconstつけておくこと

    std::vector<uint8_t> abData;

    //定数(マニュアル指定値)
    const uint8_t bMessageTypeEscape    = 0x83;
    const uint8_t bMessageTypeDataBlock = 0x80;
    const uint8_t ABDATA_START_OFFSET   = 10;
    
    if(inputCCIDlen < ABDATA_START_OFFSET){
        debugPrintMsg("ERROR! parseCCIDresponse_RDR_to_PC_Escape データが短すぎます\n");
        return abData; //空のvectorを返す
    }

    const uint8_t bMessageTypeInput = inputCCIDarr[0];

    //その他のエラー(bMassageType=80)ならば引継ぎ
    if (bMessageTypeInput == bMessageTypeDataBlock){
        parseCCIDresponse_RDR_to_PC_DataBlock(inputCCIDarr, inputCCIDlen);
        return abData; //空のvectorを返す
    }

    //dwLength (abDataの長さ) はリトルエンディアンで格納されている
    const uint32_t dwLength = (uint32_t)(inputCCIDarr[1] | (inputCCIDarr[2] << 8) | (inputCCIDarr[3] << 16) | inputCCIDarr[4] << 24);
    const uint8_t  bSlot    = inputCCIDarr[5];
    const uint8_t  SeqNo    = inputCCIDarr[6];
    const uint8_t  bStatus  = inputCCIDarr[7];
    const uint8_t  bError   = inputCCIDarr[8];
    //inputCCIDarr[9]はRFUのためスキップ

    //bStatusが0x00以外ならエラー
    if(isOK_CCIDresponse_bStatus(bStatus) == false){
        debugPrintMsg("ERROR! parseCCIDresponse_RDR_to_PC_Escape :: bStatusにエラーあり\n");
        debugPrintCCIDresponse_bError(bError);
        return abData; //空のvectorを返す
    }

#ifdef CCID_LAYER_DEBUG
    debugPrintMsg("parseCCIDresponse_RDR_to_PC_Escape :: dwLength (HEX) = ");
    debugPrintHex(dwLength);

    debugPrintMsg("parseCCIDresponse_RDR_to_PC_Escape :: abData.max_size() (HEX) = ");
    debugPrintHex(abData.max_size());
#endif

    if(dwLength > abData.max_size()){
        debugPrintMsg("ERROR! parseCCIDresponse_RDR_to_PC_Escape :: dwLengthが長すぎます\n");
        return abData; //空のvectorを返す
    }

    for(uint32_t i = 0; i < dwLength; i++){
        abData.push_back(inputCCIDarr[i + ABDATA_START_OFFSET]);
    }

    return abData; //abDataを詰めたvectorを返す
}

//RDR_to_PC_DataBlock CCID コマンドを解析(エラー発生時のみ)
void parseCCIDresponse_RDR_to_PC_DataBlock(const uint8_t* inputCCIDarr, const uint32_t inputCCIDlen){
    const uint8_t bMessageTypeDataBlock = 0x80;
    const uint8_t fixLengthTypeDataBlock = 10;

    if(inputCCIDlen < fixLengthTypeDataBlock){
        debugPrintMsg("ERROR! parseCCIDresponse_RDR_to_PC_DataBlock データが短すぎます\n");
        return;
    }

    const uint8_t bMessageTypeInput = inputCCIDarr[0];
    if(bMessageTypeInput != bMessageTypeDataBlock){
        debugPrintMsg("ERROR! parseCCIDresponse_RDR_to_PC_DataBlock bMessageTypeが違います\n");
        return;
    }

    const uint8_t bSeqTypeInput = inputCCIDarr[6];
    
    debugPrintMsg("ERROR Wrong bMassageTaype was received!\nSeqNo = ");
    debugPrintDec(bSeqTypeInput);
    return;

}

bool isOK_CCIDresponse_bStatus(const uint8_t bStatus){
    
    const uint8_t bmICCStatus    = bStatus & 0x03; //0b00000011;
    const uint8_t bmRFU          = bStatus & 0x3C; //0b00111100;
    const uint8_t bmCommandStatu = bStatus & 0xC0; //0b11000000;
    
    if(bmICCStatus != 0x02){
        debugPrintMsg("isOK_CCIDresponse_bStatus :: bmICCStatus != 0x02\n");
        return false;
    }
    #ifdef CCID_LAYER_DEBUG
        debugPrintMsg("isOK_CCIDresponse_bStatus :: bmICCStatus OK!\n");
    #endif

    if(bmRFU != 0x00){
        debugPrintMsg("isOK_CCIDresponse_bStatus :: bmRFU != 0x02\n");
        return false;
    }
    #ifdef CCID_LAYER_DEBUG
        debugPrintMsg("isOK_CCIDresponse_bStatus :: bmRFU OK!\n");
    #endif

    if(bmCommandStatu == 0x00){
        #ifdef CCID_LAYER_DEBUG
            debugPrintMsg("isOK_CCIDresponse_bStatus :: bmCommandStatu OK!\n");
        #endif
        return true;
    }else{
        debugPrintMsg("isOK_CCIDresponse_bStatus :: bmCommandStatu != 0x00\n");
        return false;
    }
    return false;
}

//エラーメッセージ表示
void debugPrintCCIDresponse_bError(const uint8_t bError){
    switch(bError){
        case 0x81:
            debugPrintMsg("bError: CMD_NOT_ABORTED\n");
            break;
        case 0xE0:
            debugPrintMsg("bError: CMD_SLOT_BUSY\n");
            break;
        case 0x00:
            debugPrintMsg("bError: Command not supported\n");
            break;
        case 0x01:
            debugPrintMsg("bError: Bad dwLength\n");
            break;
        case 0x05:
            debugPrintMsg("bError: bSlot daes not exist\n");
            break;
        default:
            debugPrintMsg("bError: Unknown Error\n");
            break;
    }
    return;
}
