#ifndef KAPHEIN_HGRD_kaphein_nes_RP2A03ApuPulseSequencer_h
#define KAPHEIN_HGRD_kaphein_nes_RP2A03ApuPulseSequencer_h

#include "def.h"
#include "kaphein/ErrorCode.h"
#include "ClockDivider.h"

struct kaphein_nes_RP2A03ApuPulseSequencer
{
    //Parameters
    kaphein_UInt8 * pDutyCycleParam_;
    kaphein_UInt8 * pSweepParam_;
    kaphein_UInt8 * pTimerLParam_;
    kaphein_UInt8 * pTimerHParam_;
    bool adderType_;

    //Units
    struct kaphein_nes_ClockDivider divider_;   //Sweep
    bool reloadFlag_;
    kaphein_UInt16 timer_;                      //Timer
    kaphein_UInt16 targetTimer_;
    kaphein_UInt16 shiftResult_;
    kaphein_UInt8 sequencer_;                   //Sequencer
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuPulseSequencer_construct(
    struct kaphein_nes_RP2A03ApuPulseSequencer * thisObj
    , kaphein_UInt8 * dutyCycleSource
    , kaphein_UInt8 * sweepSource
    , kaphein_UInt8 * timerLSource
    , kaphein_UInt8 * timerHSource
    , bool twosComplement
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuPulseSequencer_getOutput(
    const struct kaphein_nes_RP2A03ApuPulseSequencer * thisObj
    , kaphein_UInt8 * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuPulseSequencer_setSweepReloadFlag(
    struct kaphein_nes_RP2A03ApuPulseSequencer * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuPulseSequencer_resetTimer(
    struct kaphein_nes_RP2A03ApuPulseSequencer * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuPulseSequencer_resetSequencer(
    struct kaphein_nes_RP2A03ApuPulseSequencer * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuPulseSequencer_reset(
    struct kaphein_nes_RP2A03ApuPulseSequencer * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuPulseSequencer_runSweep(
    struct kaphein_nes_RP2A03ApuPulseSequencer * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuPulseSequencer_runSequencer(
    struct kaphein_nes_RP2A03ApuPulseSequencer * thisObj
);

#endif
