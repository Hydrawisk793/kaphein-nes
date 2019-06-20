#ifndef KAPHEIN_HGRD_kaphein_nes_RP2A03ApuTriangleSequencer_h
#define KAPHEIN_HGRD_kaphein_nes_RP2A03ApuTriangleSequencer_h

#include "def.h"
#include "kaphein/ErrorCode.h"

struct kaphein_nes_RP2A03ApuTriangleSequencer
{
    kaphein_UInt8 * pTimerLParam_;

    kaphein_UInt8 * pTimerHParam_;

    kaphein_UInt16 timer_;

    kaphein_UInt8 sequencer_;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuTriangleSequencer_construct(
    struct kaphein_nes_RP2A03ApuTriangleSequencer * thisObj
    , kaphein_UInt8 * timerLSource
    , kaphein_UInt8 * timerHSource
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuTriangleSequencer_getOutput(
    const struct kaphein_nes_RP2A03ApuTriangleSequencer * thisObj
    , kaphein_UInt8 * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuTriangleSequencer_reset(
    struct kaphein_nes_RP2A03ApuTriangleSequencer * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuTriangleSequencer_run(
    struct kaphein_nes_RP2A03ApuTriangleSequencer * thisObj
);

#endif
