#include "kaphein/nes/RP2A03ApuLinearCounter.h"

enum HEParam
{
    HEParam_CONTROL = 0x80,
    HEParam_RELOAD_VALUE = 0x7F
};

//TODO : 오류코드 처리

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLinearCounter_construct(
    struct kaphein_nes_RP2A03ApuLinearCounter * thisObj
    , kaphein_UInt8 * parameterSource
)
{
    thisObj->pParam_ = parameterSource;
    thisObj->counter_ = 0;
    thisObj->reloadFlag_ = false;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLinearCounter_setReloadFlag(
    struct kaphein_nes_RP2A03ApuLinearCounter * thisObj
)
{
    thisObj->reloadFlag_ = true;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLinearCounter_reset(
    struct kaphein_nes_RP2A03ApuLinearCounter * thisObj
)
{
    thisObj->counter_ = 0;
    thisObj->reloadFlag_ = false;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLinearCounter_isNonZero(
    const struct kaphein_nes_RP2A03ApuLinearCounter * thisObj
    , bool * truthOut
)
{
    *truthOut = thisObj->counter_ != 0;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuLinearCounter_run(
    struct kaphein_nes_RP2A03ApuLinearCounter * thisObj
)
{
    if(thisObj->reloadFlag_) {
        thisObj->counter_ = (*thisObj->pParam_) & HEParam_RELOAD_VALUE;
    }
    else if(thisObj->counter_ > 0) {
        --thisObj->counter_;
    }

    if(((*thisObj->pParam_) & HEParam_CONTROL) == 0) {
        thisObj->reloadFlag_ = false;
    }

    return kapheinErrorCodeNoError;
}
