#include "kaphein/nes/RP2A03ApuPulseSequencer.h"

enum HESweepParam
{
    HESweepParam_ENABLE = 0x80
    , HESweepParam_PERIOD = 0x70
    , HESweepParam_NEGATE = 0x08
    , HESweepParam_SHIFT = 0x07
};

enum
{
    DUTY_CYCLE = 0xC0
    , MAX_SEQUENCER_VALUE = 0x07
};

static const kaphein_UInt8 dutyCycleTable [0x04][MAX_SEQUENCER_VALUE+1] = {
    {0, 1, 0, 0, 0, 0, 0, 0}
    , {0, 1, 1, 0, 0, 0, 0, 0}
    , {0, 1, 1, 1, 1, 0, 0, 0}
    , {1, 0, 0, 1, 1, 1, 1, 1}
};

//TODO : 오류코드 처리

static
kaphein_UInt16
getTimerReloadValue(   
    const struct kaphein_nes_RP2A03ApuPulseSequencer * thisObj
)
{
    return makeWord(*thisObj->pTimerLParam_, *thisObj->pTimerHParam_ & 0x07);
}

static
void
updateTimerReloadValue(
    struct kaphein_nes_RP2A03ApuPulseSequencer * thisObj
)
{
    *thisObj->pTimerLParam_ = lowByte(thisObj->targetTimer_);
    *thisObj->pTimerHParam_ &= 0xF8;
    *thisObj->pTimerHParam_|= highByte(thisObj->targetTimer_) & 0x07;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuPulseSequencer_construct(
    struct kaphein_nes_RP2A03ApuPulseSequencer * thisObj
    , kaphein_UInt8 * dutyCycleSource
    , kaphein_UInt8 * sweepSource
    , kaphein_UInt8 * timerLSource
    , kaphein_UInt8 * timerHSource
    , bool twosComplement
)
{
    thisObj->pDutyCycleParam_ = dutyCycleSource;
    thisObj->pSweepParam_ = sweepSource;
    thisObj->pTimerLParam_ = timerLSource;
    thisObj->pTimerHParam_ = timerHSource;
    thisObj->adderType_ = twosComplement;
    thisObj->divider_.counter = 0;
    thisObj->divider_.period = 0;
    thisObj->reloadFlag_ = false;
    thisObj->timer_ = 0;
    thisObj->targetTimer_ = 0;
    thisObj->sequencer_ = 0;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuPulseSequencer_getOutput(
    const struct kaphein_nes_RP2A03ApuPulseSequencer * thisObj
    , kaphein_UInt8 * valueOut
)
{
    kaphein_UInt8 output = 0x00;

    if(
        getTimerReloadValue(thisObj) < 8
        || (
            (((*thisObj->pSweepParam_) & 0x08) == 0)
            && (thisObj->targetTimer_ > 0x7FF)
        )
    ) {
        output = 0;
    }
    else {
        output = (dutyCycleTable[((*thisObj->pDutyCycleParam_) & 0xC0) >> 6][thisObj->sequencer_]);
    }

    *valueOut = output;
    
    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuPulseSequencer_setSweepReloadFlag(
    struct kaphein_nes_RP2A03ApuPulseSequencer * thisObj
)
{
    thisObj->reloadFlag_ = true;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuPulseSequencer_resetTimer(
    struct kaphein_nes_RP2A03ApuPulseSequencer * thisObj
)
{
    thisObj->timer_ = getTimerReloadValue(thisObj);

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuPulseSequencer_resetSequencer(
    struct kaphein_nes_RP2A03ApuPulseSequencer * thisObj
)
{
    thisObj->sequencer_ = 0;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuPulseSequencer_reset(
    struct kaphein_nes_RP2A03ApuPulseSequencer * thisObj
)
{
    thisObj->timer_ = thisObj->targetTimer_ = 0;
    thisObj->sequencer_ = 0;
    thisObj->reloadFlag_ = false;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuPulseSequencer_runSweep(
    struct kaphein_nes_RP2A03ApuPulseSequencer * thisObj
)
{
    kaphein_UInt16 timerReloadValue = getTimerReloadValue(thisObj);
    bool truth;
    
    if(kaphein_nes_ClockDivider_run(&thisObj->divider_, &truth), truth){
        thisObj->shiftResult_ = timerReloadValue >> ((*thisObj->pSweepParam_) & 0x07);
        
        if(((*thisObj->pSweepParam_) & 0x08) != 0) {
            thisObj->targetTimer_ = timerReloadValue - thisObj->shiftResult_ - ((thisObj->adderType_)?(0):(1));
        }
        else {
            thisObj->targetTimer_ = timerReloadValue + thisObj->shiftResult_;
        }

        if(
            (((*thisObj->pSweepParam_) & 0x80) != 0)
            && ((*thisObj->pSweepParam_) & 0x07) != 0
            && (timerReloadValue >= 8)
            && (thisObj->targetTimer_ <= 0x7FF)
        ) {
            updateTimerReloadValue(thisObj);
        }
    }

    if(thisObj->reloadFlag_){
        thisObj->divider_.period = (((*thisObj->pSweepParam_) & 0x70) >> 4) + 1; //디바이더 주기 셋
        kaphein_nes_ClockDivider_reset(&thisObj->divider_);
        thisObj->reloadFlag_ = false;
    }

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuPulseSequencer_runSequencer(
    struct kaphein_nes_RP2A03ApuPulseSequencer * thisObj
)
{
    if(thisObj->timer_ < 1){
        thisObj->timer_ = getTimerReloadValue(thisObj);
        
        if(++thisObj->sequencer_ >= MAX_SEQUENCER_VALUE + 1) {
            thisObj->sequencer_ = 0;
        }
    }
    else {
        --thisObj->timer_;
    }

    return kapheinErrorCodeNoError;
}
