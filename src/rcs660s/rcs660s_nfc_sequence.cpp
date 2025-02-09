#include "rcs660s_nfc_sequence.h";

/*****************/
/* Public Method */
/*****************/

//コンストラクタ
RCS660S_NFC_SEQUENCE::RCS660S_NFC_SEQUENCE(const uint8_t cType){
    card_type = cType;
    reader_state = READER_RFOFF;
}

//デストラクタ
RCS660S_NFC_SEQUENCE::~RCS660S_NFC_SEQUENCE(){
    return;
}


bool RCS660S_NFC_SEQUENCE::catchCard(void){

    if(reader_state != READER_RFOFF){
        return false;
    }

    if(reader_state == READER_RFOFF){
        //Reset
        assemblyAPDUcommand_ResetDevice();

        uart_receiver_checkACK();
        

        //Start Transparent Session

        //Switch Protocol TypeB AutoActivate

        //Transparent Exchange TransmissionAndReceptionFlag

        //RF On
        assemblyAPDUcommand_ManageSession_TrunOnRfField();
    }

    bool ret = false;
    switch (card_type)
    {
        case CARD_TYPE_B:
            ret = catchCard_TypeB();
            break;
        default:
            ret = false;
            break;
    }
    return ret;
}

void RCS660S_NFC_SEQUENCE::communicateNFC(const uint8_t *tx_data, const uint16_t tx_len, uint8_t *rx_data, uint16_t *rx_len){

    
    return;
}


/******************/
/* Private Method */
/******************/

bool RCS660S_NFC_SEQUENCE::catchCard_TypeB(void){
    //Start Transparent Session
    //Switch Protocol TypeB AutoActivate
    //※RC-S660/Sではソフト側でキャッチするまで無限リトライが必要

    //Transparent Exchange TransmissionAndReceptionFlag

    return false;
}


void RCS660S_NFC_SEQUENCE::releaseCard(void){
    //RF Off
    assemblyAPDUcommand_ManageSession_TrunOffRfField();

    //End Transparent Session
    assemblyAPDUcommand_ManageSession_EndTransparentSession();
    return;
}