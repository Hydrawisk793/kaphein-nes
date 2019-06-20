#ifndef KAPHEIN_HGRD_kaphein_nes_RP2A03ApuFrameCounter_h
#define KAPHEIN_HGRD_kaphein_nes_RP2A03ApuFrameCounter_h

#include "def.h"
#include "kaphein/ErrorCode.h"
#include "ClockDivider.h"

struct kaphein_nes_RP2A03ApuFrameCounter
{
    struct kaphein_nes_ClockDivider clockDivider_;

    unsigned int sequencer_;

    kaphein_UInt8 config_;

    bool interruptFlag_;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_construct(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
    , unsigned int clockDividerPeriod
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_getConfig(
    const struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
    , kaphein_UInt8 * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_getMode(
    const struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
    , kaphein_UInt8 * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_getDisableIRQFlag(
    const struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
    , bool * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_isInterruptFlagSet(
    const struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
    , bool * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_getSequencer(
    const struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
    , unsigned int * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_setConfig(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
    , kaphein_UInt8 v
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_setDisableIRQFlag(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_clearDisableIRQFlag(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_setInterruptFlag(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_clearInterruptFlag(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_resetSequencer(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_resetClockDivider(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_incrementSequencer(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuFrameCounter_run(
    struct kaphein_nes_RP2A03ApuFrameCounter * thisObj
    , bool * resultOut
);

#endif
