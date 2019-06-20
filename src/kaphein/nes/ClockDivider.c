#include "kaphein/nes/ClockDivider.h"

enum kaphein_ErrorCode
kaphein_nes_ClockDivider_reset(
    struct kaphein_nes_ClockDivider * thisObj
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        thisObj->counter = thisObj->period;
    }

    return resultErrorCode;
}

enum kaphein_ErrorCode
kaphein_nes_ClockDivider_run(
    struct kaphein_nes_ClockDivider * thisObj
    , bool * resultOut
)
{
    enum kaphein_ErrorCode resultErrorCode = kapheinErrorCodeNoError;
    bool result = false;

    if(KAPHEIN_NULL == thisObj) {
        resultErrorCode = kapheinErrorCodeArgumentNull;
    }
    else {
        //TODO : 언더플로우 처리...?
        if(--thisObj->counter == 0) {
            thisObj->counter = thisObj->period;

            result = true;
        }

        if(KAPHEIN_NULL != resultOut) {
            *resultOut = result;
        }
    }

    return resultErrorCode;
}
