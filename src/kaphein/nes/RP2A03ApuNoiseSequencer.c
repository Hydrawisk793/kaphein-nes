#include "kaphein/nes/RP2A03ApuNoiseSequencer.h"
#include "kaphein/nes/ShiftRegister15.h"

enum HEParam
{
    HEParam_MODE = 0x80
    , HEParam_PERIOD_INDEX = 0x0F
};

static const kaphein_UInt16 periodTable[0x10] = {
    //NTSC (CPU cycle)
    //4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068

    //NTSC (APU cycle)
    2, 4, 8, 16, 32, 48, 64, 80, 101, 127, 190, 254, 381, 508, 1017, 2034

    //PAL
    //...
};

//TODO : 오류코드 처리

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuNoiseSequencer_construct(
    struct kaphein_nes_RP2A03ApuNoiseSequencer * thisObj
    , kaphein_UInt8 * parameterSource
)
{
    thisObj->pParam_ = parameterSource;
    kaphein_nes_RP2A03ApuNoiseSequencer_reset(thisObj);

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuNoiseSequencer_getOutput(
    const struct kaphein_nes_RP2A03ApuNoiseSequencer * thisObj
    , kaphein_UInt8 * valueOut
)
{
    //LSB가 1이면 0, 아니면 1.
    *valueOut = (~thisObj->prn_) & 0x01;
    
    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuNoiseSequencer_reset(
    struct kaphein_nes_RP2A03ApuNoiseSequencer * thisObj
)
{
    thisObj->timer_ = periodTable[0];
    thisObj->prn_ = 1;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuNoiseSequencer_run(
    struct kaphein_nes_RP2A03ApuNoiseSequencer * thisObj
)
{
    kaphein_UInt8 param;
    kaphein_UInt8 feedValue;

    if(thisObj->timer_ < 1) {
        param = *thisObj->pParam_;

        thisObj->timer_ = periodTable[param & HEParam_PERIOD_INDEX];
        
        feedValue = (thisObj->prn_ & 0x01) ^ (!!(thisObj->prn_ & ((param & HEParam_MODE) ? 0x40 : 0x02)));
        kaphein_nes_ShiftRegister15_shiftRight(&thisObj->prn_, !!feedValue);
    }
    else {
        --thisObj->timer_;
    }

    return kapheinErrorCodeNoError;
}
