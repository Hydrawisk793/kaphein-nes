#ifndef KAPHEIN_HGRD_kaphein_nes_RP2A03ApuDmcOutputUnit_h
#define KAPHEIN_HGRD_kaphein_nes_RP2A03ApuDmcOutputUnit_h

#include "def.h"
#include "kaphein/ErrorCode.h"
#include "ShiftRegister8.h"
#include "RP2A03ApuDmcReader.h"

struct kaphein_nes_RP2A03ApuDmcOutputUnit
{
    //파라미터
    kaphein_UInt8 * pFlagParam_;
    kaphein_UInt8 * pFreqParam_;
    //kaphein_UInt8 * pAddrParam_;
    //kaphein_UInt8 * pLengthParam_;

    //샘플 데이터 리더
    struct kaphein_nes_RP2A03ApuDmcReader * reader_;

    //버퍼
    kaphein_Int16 * buffer_;
    
    //출력기
    kaphein_UInt16 timer_;
    kaphein_UInt8 sr_;                      //(8) 시프트 레지스터
    kaphein_UInt8 remainingBitsCounter_;    //(4) 잔여 비트 카운터
    kaphein_UInt8 deltaCounter_;            //(7) 델타 카운터
    bool silenceFlag_;                      //(1) 침묵 플래그
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuDmcOutputUnit_construct(
    struct kaphein_nes_RP2A03ApuDmcOutputUnit * thisObj
    , kaphein_UInt8 * flagSource
    , kaphein_UInt8 * freqSource
    , kaphein_Int16 * buffer
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuDmcOutputUnit_setDmcReader(
    struct kaphein_nes_RP2A03ApuDmcOutputUnit * thisObj
    , struct kaphein_nes_RP2A03ApuDmcReader * reader
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuDmcOutputUnit_getOutput(
    const struct kaphein_nes_RP2A03ApuDmcOutputUnit * thisObj
    , kaphein_UInt8 * resultOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuDmcOutputUnit_setDeltaCounter(
    struct kaphein_nes_RP2A03ApuDmcOutputUnit * thisObj
    , kaphein_UInt8 v
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuDmcOutputUnit_run(
    struct kaphein_nes_RP2A03ApuDmcOutputUnit * thisObj
    , int debugCycleCounter
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuDmcOutputUnit_reset(
    struct kaphein_nes_RP2A03ApuDmcOutputUnit * thisObj
);

#endif
