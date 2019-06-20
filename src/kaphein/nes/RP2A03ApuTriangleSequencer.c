#include "kaphein/nes/RP2A03ApuTriangleSequencer.h"

enum
{
    MAX_SEQUENCER_VALUE = 0x1F
};

static const kaphein_UInt8 waveformTable[MAX_SEQUENCER_VALUE + 1] = {
    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
    , 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

//TODO : 오류코드 처리

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuTriangleSequencer_construct(
    struct kaphein_nes_RP2A03ApuTriangleSequencer * thisObj
    , kaphein_UInt8 * timerLSource
    , kaphein_UInt8 * timerHSource
)
{
    thisObj->pTimerLParam_ = timerLSource;
    thisObj->pTimerHParam_ = timerHSource;
    thisObj->timer_ = 0;
    thisObj->sequencer_ = 0;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuTriangleSequencer_getOutput(
    const struct kaphein_nes_RP2A03ApuTriangleSequencer * thisObj
    , kaphein_UInt8 * valueOut
)
{
    kaphein_UInt8 result = waveformTable[thisObj->sequencer_];

    *valueOut = (
        (makeWord(*thisObj->pTimerLParam_, *thisObj->pTimerHParam_ & 0x07) < 2)
        ? 7
        : result
    );

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuTriangleSequencer_reset(
    struct kaphein_nes_RP2A03ApuTriangleSequencer * thisObj
)
{
    thisObj->timer_ = 0;
    thisObj->sequencer_ = 0;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuTriangleSequencer_run(
    struct kaphein_nes_RP2A03ApuTriangleSequencer * thisObj
)
{
    if(thisObj->timer_ == 0) {
        thisObj->timer_ = makeWord(*thisObj->pTimerLParam_, *thisObj->pTimerHParam_ & 0x07);
        
        if(++thisObj->sequencer_ >= MAX_SEQUENCER_VALUE + 1) {
            thisObj->sequencer_ = 0;
        }
    }
    else {
        --thisObj->timer_;
    }

    return kapheinErrorCodeNoError;
}
