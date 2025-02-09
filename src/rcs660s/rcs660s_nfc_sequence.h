#ifndef RCS660S_NFC_SEQUENCE_H
#define RCS660S_NFC_SEQUENCE_H

#include <stdint.h>
#include <stdbool.h>

// Your code goes here

#define CARD_TYPE_A 0x00
#define CARD_TYPE_B 0x01
#define CARD_TYPE_V 0x02
#define CARD_TYPE_FELICA 0x03


class RCS660S_NFC_SEQUENCE {
    public:
        RCS660S_NFC_SEQUENCE(const uint8_t cart_type);
        ~RCS660S_NFC_SEQUENCE();
        bool catchCard(void);
        void communicateNFC(const uint8_t*, const uint16_t, uint8_t*, uint16_t*);
    private:
        uint8_t card_type;
        //bool catchCard_TypeA(void);
        bool catchCard_TypeB(void);
        //bool catchCard_TypeV(void);
        //bool catchCard_FeliCa(void);
        void releaseCard(void);
};

#endif // RCS660S_NFC_SEQUENCE_H