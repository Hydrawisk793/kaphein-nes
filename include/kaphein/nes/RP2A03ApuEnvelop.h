#ifndef KAPHEIN_HGRD_kaphein_nes_RP2A03ApuEnvelop_h
#define KAPHEIN_HGRD_kaphein_nes_RP2A03ApuEnvelop_h

#include "def.h"
#include "kaphein/ErrorCode.h"
#include "ClockDivider.h"

struct kaphein_nes_RP2A03ApuEnvelop
{
    kaphein_UInt8 * pParam_;

    struct kaphein_nes_ClockDivider divider_;

    kaphein_UInt8 counter_;

    bool startFlag_;
};

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuEnvelop_construct(
    struct kaphein_nes_RP2A03ApuEnvelop * thisObj
    , kaphein_UInt8 * parameterSource
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuEnvelop_getOutput(
    struct kaphein_nes_RP2A03ApuEnvelop * thisObj
    , kaphein_UInt8 * valueOut
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuEnvelop_setStartFlag(
    struct kaphein_nes_RP2A03ApuEnvelop * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuEnvelop_reset(
    struct kaphein_nes_RP2A03ApuEnvelop * thisObj
);

KAPHEIN_ATTRIBUTE_C_LINKAGE
KAPHEIN_ATTRIBUTE_DLL_API
enum kaphein_ErrorCode
kaphein_nes_RP2A03ApuEnvelop_run(
    struct kaphein_nes_RP2A03ApuEnvelop * thisObj
);

#endif
