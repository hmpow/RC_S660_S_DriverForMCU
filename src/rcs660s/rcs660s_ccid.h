#ifndef RCS660S_CCID_H
#define RCS660S_CCID_H

//#define CCID_LAYER_DEBUG

#include <stdlib.h>
//RC-S660/Sでサポート・カスタムされたCCIDコマンドを組み立てる
#include <stdint.h>

#include <vector> //.cリンクできない問題で.cppに統一したら使えるようになったので受信側はmallocやめる！

#include "rcs660s_util.h"
#include "rcs660s_uart.h"

void assemblyCCIDcommand_PC_to_RDR_Escape(const uint8_t*, const uint32_t, const uint8_t);
void assemblyCCIDcommand_PC_to_RDR_Abort(uint8_t SeqNo);

std::vector<uint8_t> parseCCIDresponse_RDR_to_PC_Escape(const uint8_t*, const uint32_t);
void parseCCIDresponse_RDR_to_PC_DataBlock(const uint8_t*, const uint32_t);

bool isOK_CCIDresponse_bStatus(const uint8_t);
void debugPrintCCIDresponse_bError(const uint8_t);

#endif // RCS660S_CCID_H