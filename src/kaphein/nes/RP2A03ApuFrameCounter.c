#include "kaphein/nes/RP2A03ApuFrameCounter.h"

enum HEConfig
{
    HEConfig_MODE = 0x80
    , HEConfig_DISABLE_IRQ = 0x40
    , HEConfig_VALUES = 0xC0
    , HEConfig_UNUSED = 0x3F
};

//TODO : 오류코드 처리

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_construct(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
    , unsigned int clockDividerPeriod
)
{
    thisObj->config_ = 0;
    thisObj->interruptFlag_ = false;
    thisObj->sequencer_ = 0;
    thisObj->clockDivider_.counter = clockDividerPeriod;
    thisObj->clockDivider_.period = clockDividerPeriod;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_getConfig(
    const struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
    , kaphein_UInt8 * valueOut
)
{
    *valueOut = thisObj->config_;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_getMode(
    const struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
    , kaphein_UInt8 * valueOut
)
{
    *valueOut = thisObj->config_ & HEConfig_MODE;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_getDisableIRQFlag(
    const struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
    , bool * valueOut
)
{
    *valueOut = (thisObj->config_ & HEConfig_DISABLE_IRQ) != 0;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_isInterruptFlagSet(
    const struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
    , bool * valueOut
)
{
    *valueOut = thisObj->interruptFlag_;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_getSequencer(
    const struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
    , unsigned int * valueOut
)
{
    *valueOut = thisObj->sequencer_;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_setConfig(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
    , kaphein_UInt8 v
)
{
    thisObj->config_ = v;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_setDisableIRQFlag(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
)
{
    thisObj->config_ |= HEConfig_DISABLE_IRQ;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_clearDisableIRQFlag(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
)
{
    thisObj->config_ &= ~HEConfig_DISABLE_IRQ;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_setInterruptFlag(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
)
{
    thisObj->interruptFlag_ = true;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_clearInterruptFlag(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
)
{
    thisObj->interruptFlag_ = false;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_resetSequencer(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
)
{
    thisObj->sequencer_ = 0;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_resetClockDivider(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
)
{
    kaphein_nes_ClockDivider_reset(&thisObj->clockDivider_);

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_incrementSequencer(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
)
{
    ++thisObj->sequencer_;

    return kapheinErrorCodeNoError;
}

enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_run(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
    , bool * resultOut
)
{
    return kaphein_nes_ClockDivider_run(&thisObj->clockDivider_, resultOut);
}
