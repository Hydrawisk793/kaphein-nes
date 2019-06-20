#include "kaphein/nes/RP2A03ApuLengthCounter.h"

static const kaphein_UInt8 table[0x20] = {
    0x0A, 0xFE,
    0x14, 0x02,
    0x28, 0x04,
    0x50, 0x06,
    0xA0, 0x08,
    0x3C, 0x0A,
    0x0E, 0x0C,
    0x1A, 0x0E,
    0x0C, 0x10,
    0x18, 0x12,
    0x30, 0x14,
    0x60, 0x16,
    0xC0, 0x18,
    0X48, 0x1A,
    0x10, 0x1C,
    0x20, 0x1E
};

//TODO : 오류코드 처리

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLengthCounter_construct(
    struct kaphein_nes_RP2A03ApuLengthCounter * thisObj
    , kaphein_UInt8 stopCountingBit
    , kaphein_UInt8 * parameterSource
)
{
    thisObj->fixLengthBit_ = stopCountingBit;
    thisObj->pParam_ = parameterSource;
    thisObj->counter_ = 0;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLengthCounter_get(
    const struct kaphein_nes_RP2A03ApuLengthCounter * thisObj
    , kaphein_UInt8 * valueOut
)
{
    *valueOut = thisObj->counter_;
    
    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLengthCounter_set(
    struct kaphein_nes_RP2A03ApuLengthCounter * thisObj
    , kaphein_UInt8 index
)
{
    thisObj->counter_ = table[index];

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLengthCounter_reset(
    struct kaphein_nes_RP2A03ApuLengthCounter * thisObj
)
{
    thisObj->counter_ = 0;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLengthCounter_isNonZero(
    const struct kaphein_nes_RP2A03ApuLengthCounter * thisObj
    , bool * truthOut
)
{
    *truthOut = thisObj->counter_ != 0;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLengthCounter_run(
    struct kaphein_nes_RP2A03ApuLengthCounter * thisObj
)
{
    if(
        ((*thisObj->pParam_ & thisObj->fixLengthBit_) == 0)
        && thisObj->counter_ > 0
    ) {
        --thisObj->counter_;
    }

    return kapheinErrorCodeNoError;
}
